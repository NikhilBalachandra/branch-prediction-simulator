#include "sim_smith_n_bit.h"
#include "sim_math.h"
#include <math.h>
#include <stdio.h>
#include <inttypes.h>

void sim_smith_n_bit_init(struct SimSmithNBit *s, uint64_t n) {
  uint64_t two_pow_n = sim_math_2pow(n);
  s->_counter_max = two_pow_n - 1;
  s->_counter_taken_threshold = two_pow_n / 2;
  s->counter = s->_counter_taken_threshold;
}

bool sim_smith_n_bit_predict_and_update(struct SimSmithNBit *s, struct SimInstruction *i) {
  bool prediction = false;

  if (s->counter >= s->_counter_taken_threshold) {
    prediction = true;
  }

  if (i->taken) {
    if (s->counter < s->_counter_max) {
      s->counter++;
    }
  } else {
    if (s->counter > 0) {
      s->counter--;
    }
  }
  return prediction;
}

void sim_smith_n_bit_print(struct SimSmithNBit *s, FILE *f) {
  fprintf(f, "FINAL COUNTER CONTENT: %" PRIu64 "\n", s->counter);
}
