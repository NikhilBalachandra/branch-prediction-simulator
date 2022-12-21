#pragma once

#include <stdint.h>
#include <stdio.h>
#include "sim_parser.h"
#include "sim_gshare.h"

struct SimHybrid {
  uint64_t n;
  uint64_t k;
  uint64_t m1;
  uint64_t m2;
  uint64_t global_bhr;
  uint64_t *chooser_table;

  struct SimGshare gshare;
  struct SimGshare bimodal;

  uint64_t _pc_bits_mask;
  uint64_t _counter_max;
  uint64_t _counter_gshare_threshold;
};

void sim_hybrid_init(struct SimHybrid *h, uint64_t k, uint64_t n, uint64_t m1, uint64_t m2, uint64_t counter_bits, uint64_t counter_init_val);
void sim_hybrid_free(struct SimHybrid *h);
bool sim_hybrid_predict_and_update(struct SimHybrid *h, struct SimInstruction *i);
void sim_hybrid_print(struct SimHybrid *h, FILE *f);
