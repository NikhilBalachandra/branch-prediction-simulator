#pragma once

#include <stdint.h>
#include "sim_parser.h"
#include <stdio.h>

struct SimGshare {
  uint64_t n;
  uint64_t m;
  uint64_t global_bhr;
  uint64_t *prediction_table;

  uint64_t _pc_bits_mask;
  uint64_t _counter_max;
  uint64_t _counter_taken_threshold;
  uint64_t _global_bhr_mask;
};

void sim_gshare_init(struct SimGshare *gs, uint64_t n, uint64_t m, uint64_t counter_bits);
void sim_gshare_free(struct SimGshare *gs);
size_t sim_gshare_get_index(struct SimGshare *gs, struct SimInstruction *i);
void sim_gshare_update_prediction_table(struct SimGshare *gs, size_t index, bool taken);
void sim_gshare_update_gbhr(struct SimGshare *gs, bool taken);
bool sim_gshare_predict(struct SimGshare *gs, size_t index);
bool sim_gshare_predict_and_update(struct SimGshare *gs, struct SimInstruction *i);
void sim_gshare_print(struct SimGshare *gs, FILE *f);
