#include "sim_gshare.h"

#include <stdlib.h>
#include <stdint.h>
#include "sim_math.h"
#include <stdio.h>
#include <inttypes.h>

void sim_gshare_init(struct SimGshare *gs, uint64_t n, uint64_t m, uint64_t counter_bits) {
  gs->n = n;
  gs->m = m;

  uint64_t two_pow_m = sim_math_2pow(m);
  uint64_t two_pow_counter_bits = sim_math_2pow(counter_bits);

  gs->global_bhr = 0;
  gs->prediction_table = malloc(two_pow_m * sizeof(uint64_t));

  gs->_counter_max = two_pow_counter_bits - 1;
  gs->_counter_taken_threshold = two_pow_counter_bits / 2;

  for (size_t i = 0; i < two_pow_m; i++) {
    gs->prediction_table[i] = gs->_counter_taken_threshold;
  }
  gs->_pc_bits_mask = sim_gen_mask(0, gs->m);

  // Gives mask that has MSB set to 1 and everything else to 0.
  // Something like 10000000
  gs->_global_bhr_mask = sim_math_2pow(gs->n-1);
}

void sim_gshare_free(struct SimGshare *gs) {
  gs->n = 0;
  gs->m = 0;
  gs->global_bhr = 0;
  gs->_counter_max = 0;
  gs->_counter_taken_threshold = 0;
  free(gs->prediction_table);
}

size_t sim_gshare_get_index(struct SimGshare *gs, struct SimInstruction *i) {
  if (gs->n > 0) {
    return gs->global_bhr ^ ((i->address >> 2) & gs->_pc_bits_mask);
  } else {
    return ((i->address >> 2) & gs->_pc_bits_mask);
  }
}

bool sim_gshare_predict(struct SimGshare *gs, size_t index) {
  uint64_t counter = gs->prediction_table[index];
  if (counter >= gs->_counter_taken_threshold) {
    return true;
  }

  return false;
}

void sim_gshare_update_gbhr(struct SimGshare *gs, bool taken) {
  gs->global_bhr = gs->global_bhr >> 1;
  if (taken) {
    gs->global_bhr |= gs->_global_bhr_mask;
  }
}

void sim_gshare_update_prediction_table(struct SimGshare *gs, size_t index, bool taken) {
  uint64_t counter = gs->prediction_table[index];
  if (taken) {
    if (counter < gs->_counter_max) {
      gs->prediction_table[index]++;
    }
  } else {
    if (counter > 0) {
      gs->prediction_table[index]--;
    }
  }
}

bool sim_gshare_predict_and_update(struct SimGshare *gs, struct SimInstruction *i) {
  size_t index = sim_gshare_get_index(gs, i);
  bool prediction = sim_gshare_predict(gs, index);
  bool outcome = i->taken;
  sim_gshare_update_gbhr(gs, outcome);
  sim_gshare_update_prediction_table(gs, index, outcome);
  return prediction;
}

void sim_gshare_print(struct SimGshare *gs, FILE *f) {
  if (gs->n > 0) {
    fprintf(f, "FINAL GSHARE CONTENTS\n");
  } else {
    fprintf(f, "FINAL BIMODAL CONTENTS\n");
  }
  for (size_t i = 0; i < sim_math_2pow(gs->m); i++) {
    fprintf(f, "%" PRIuPTR " %" PRIu64 "\n", i, gs->prediction_table[i]);
  }
}
