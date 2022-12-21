#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "sim_parser.h"
#include <stdio.h>

struct SimSmithNBit {
  uint64_t n;
  uint64_t counter;

  uint64_t _counter_max;
  uint64_t _counter_taken_threshold;
};

void sim_smith_n_bit_init(struct SimSmithNBit *s, uint64_t n);
bool sim_smith_n_bit_predict_and_update(struct SimSmithNBit *s, struct SimInstruction *i);
void sim_smith_n_bit_print(struct SimSmithNBit *s, FILE *f);
