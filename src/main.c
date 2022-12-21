#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "sim_gshare.h"
#include "sim_hybrid.h"
#include "sim_io.h"
#include "sim_math.h"
#include "sim_parser.h"
#include "sim_simulator.h"
#include "sim_smith_n_bit.h"

// log is a macro that writes to the f if f is not NULL
#define log(file, f, ...)                                                      \
  if (file != NULL)                                                            \
    fprintf(file, f, ##__VA_ARGS__);

#define fatal(file, f, ...) log(file, "Fatal: " f "\n", ##__VA_ARGS__);
#define usage(file, f, ...) log(file, "Usage: " f "\n", ##__VA_ARGS__);

void help(FILE *f) {
  log(f, "Usage: sim SIMULATION_TYPE [SIMULATION_ARGS]\n");
  log(f, "       SIMULATION_TYPE should be one of smith, bimodal, gshare, "
         "hybrid\n");
}

enum SimSimulationType {
  SMITH_N_BIT = 1,
  GSHARE = 2,
  BIMODAL = 3,
  HYBRID = 4,
};

struct SimArgs {
  enum SimSimulationType type;
  uint64_t counter_bits;
  uint64_t chooser_bits;
  uint64_t pc_bits_1;
  uint64_t pc_bits_2;
  uint64_t gbhr_bits;
  char *tracefile;
};

// Parse command line arguments.
// A valid command line usage will identify simulator type as the first
// argument. Additional arguments depend on the simulator type.
int parse_args(int argc, char *restrict argv[], FILE *f, struct SimArgs *args) {
  // Requires at least one argument to identify simulator type.
  if (argc < 2) {
    fatal(f, "Expect one of the simulator types as the first argument");
    help(f);
    return -1;
  }

  if (strcmp("smith", argv[1]) == 0) {
    // sim smith COUNTER_BITS TRACE_FILE
    if (argc != 4) {
      fatal(f, "Missing required arguments for the smith branch predictor");
      usage(f, "smith COUNTER_BITS TRACE_FILE");
      return -1;
    }
    int64_t counter_bits;
    if (sim_math_strtoint64(argv[2], &counter_bits) != 0) {
      fatal(f, "Error parsing \"%s\" as number", argv[2]);
      return -1;
    }
    args->type = SMITH_N_BIT;
    args->counter_bits = counter_bits;
    args->tracefile = argv[3];
  } else if (strcmp("gshare", argv[1]) == 0) {
    // sim gshare <M1> <N> <tracefile>
    if (argc != 5) {
      fatal(f, "Missing required arguments for the gshare branch predictor");
      usage(f, "gshare PC_BITS GLOBAL_BRANCH_HISTORY_REGISTER_BITS TRACE_FILE");
      return -1;
    }
    int64_t pc_bits;
    if (sim_math_strtoint64(argv[2], &pc_bits) != 0) {
      fatal(f, "Error parsing \"%s\" as number", argv[2]);
      return -1;
    }
    int64_t global_branch_history_register_bits;
    if (sim_math_strtoint64(argv[3], &global_branch_history_register_bits) !=
        0) {
      fatal(f, "Error parsing \"%s\" as number", argv[3]);
      return -1;
    }

    // TOOD: Handle negative case?
    args->type = GSHARE;
    args->pc_bits_1 = pc_bits;
    args->gbhr_bits = global_branch_history_register_bits;
    args->tracefile = argv[4];
  } else if (strcmp("bimodal", argv[1]) == 0) {
    // sim bimodal <M2> <tracefile>
    if (argc != 4) {
      return -1;
    }
    int64_t pc_bits;
    if (sim_math_strtoint64(argv[2], &pc_bits) != 0) {
      fatal(f, "Error parsing \"%s\" as number", argv[2]);
      return -1;
    }
    args->type = BIMODAL;
    args->pc_bits_1 = pc_bits;
    args->tracefile = argv[3];
  } else if (strcmp("hybrid", argv[1]) == 0) {
    // sim hybrid <K> <M1> <N> <M2> <tracefile>
    if (argc != 7) {
      return -1;
    }
    int64_t chooser_bits;
    if (sim_math_strtoint64(argv[2], &chooser_bits) != 0) {
      fatal(f, "Error parsing \"%s\" as number", argv[2]);
      return -1;
    }
    int64_t pc_bits_1;
    if (sim_math_strtoint64(argv[3], &pc_bits_1) != 0) {
      fatal(f, "Error parsing \"%s\" as number", argv[3]);
      return -1;
    }
    int64_t global_branch_history_register_bits;
    if (sim_math_strtoint64(argv[4], &global_branch_history_register_bits) !=
        0) {
      fatal(f, "Error parsing \"%s\" as number", argv[4]);
      return -1;
    }
    int64_t pc_bits_2;
    if (sim_math_strtoint64(argv[5], &pc_bits_2) != 0) {
      fatal(f, "Error parsing \"%s\" as number", argv[5]);
      return -1;
    }
    args->type = HYBRID;
    args->chooser_bits = chooser_bits;
    args->pc_bits_1 = pc_bits_1;
    args->gbhr_bits = global_branch_history_register_bits;
    args->pc_bits_2 = pc_bits_2;
    args->tracefile = argv[6];
  } else {
    log(f, "%s is not a valid simulation type\n", argv[1]);
  }

  return 0;
}

// Print command line arguments.
void print_args(int argc, char *restrict argv[], FILE *f) {
  // Always print the program name.
  // Then print rest of the arguments separated by a space.
  // End with a new line.
  fprintf(f, "%s", argv[0]);
  for (size_t i = 1; i < argc; i++) {
    fprintf(f, " %s", argv[i]);
  }
  fprintf(f, "\n");
}

int main(int argc, char *argv[]) {
  struct SimArgs args;
  int status = parse_args(argc, argv, stderr, &args);
  if (status != 0) {
    return status;
  }

  errno = 0;
  int fd = open(args.tracefile, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Error reading trace file: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  // Print command line arguments used to run the simulator.
  printf("COMMAND\n");
  print_args(argc, argv, stdout);

  struct SimSmithNBit smith_n_bit;
  struct SimGshare gshare;
  struct SimHybrid hybrid;
  if (args.type == SMITH_N_BIT) {
    sim_smith_n_bit_init(&smith_n_bit, args.counter_bits);
  } else if (args.type == GSHARE) {
    sim_gshare_init(&gshare, args.gbhr_bits, args.pc_bits_1, 3);
  } else if (args.type == BIMODAL) {
    sim_gshare_init(&gshare, 0, args.pc_bits_1, 3);
  } else if (args.type == HYBRID) {
    sim_hybrid_init(&hybrid, args.chooser_bits, args.gbhr_bits, args.pc_bits_1,
                    args.pc_bits_2, 2, 1);
  } else {
    fprintf(stderr, "Invalid simulation type\n");
    return EXIT_FAILURE;
  }

  struct SimReadBuf buffer = {.buf = NULL, .fd = fd};
  struct SimParser parser = {.buf = buffer};
  struct SimInstruction inst;
  if (sim_read_buf_init(&parser.buf, 1024) < 0) {
    fprintf(stderr, "Error allocating buffer to read the file\n");
    return EXIT_FAILURE;
  }

  uint64_t total_predictions = 0;
  uint64_t mis_predictions = 0;
  uint64_t parse_status = 0;
  while ((parse_status = sim_parser_next_token(&parser, &inst)) > 0) {
    total_predictions++;
    bool prediction = false;
    bool outcome = inst.taken;
    if (args.type == SMITH_N_BIT) {
      prediction = sim_smith_n_bit_predict_and_update(&smith_n_bit, &inst);
    } else if (args.type == GSHARE) {
      prediction = sim_gshare_predict_and_update(&gshare, &inst);
    } else if (args.type == BIMODAL) {
      prediction = sim_gshare_predict_and_update(&gshare, &inst);
    } else if (args.type == HYBRID) {
      prediction = sim_hybrid_predict_and_update(&hybrid, &inst);
    } else {
      fprintf(stderr, "Assertion failure: Invalid branch predictor type: %d\n", args.type);
      return EXIT_FAILURE;
    }
    if (prediction != outcome) {
      mis_predictions++;
    }
  }
  close(fd);
  sim_read_buf_free(&parser.buf);

  fprintf(stdout, "OUTPUT\n");
  fprintf(stdout, "number of predictions: \t%" PRIu64 "\n", total_predictions);
  fprintf(stdout, "number of mispredictions:\t%" PRIu64 "\n", mis_predictions);
  fprintf(stdout, "misprediction rate:\t\t%.2f%%\n",
          (mis_predictions * 100.0) / total_predictions);
  if (args.type == SMITH_N_BIT) {
    sim_smith_n_bit_print(&smith_n_bit, stdout);
  } else if (args.type == GSHARE) {
    sim_gshare_print(&gshare, stdout);
    sim_gshare_free(&gshare);
  } else if (args.type == BIMODAL) {
    sim_gshare_print(&gshare, stdout);
    sim_gshare_free(&gshare);
  } else if (args.type == HYBRID) {
    sim_hybrid_print(&hybrid, stdout);
    sim_hybrid_free(&hybrid);
  } else {
    fprintf(stderr, "Assertion failure: Invalid branch predictor type: %d\n", args.type);
    return EXIT_FAILURE;
  }

  if (parse_status != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
