#include "sim_hybrid.h"
#include "sim_gshare.h"
#include "sim_math.h"
#include <stdlib.h>
#include <inttypes.h>

#define GSHARE_COUNTER_BITS 3

void sim_hybrid_init(struct SimHybrid *h, uint64_t k, uint64_t n, uint64_t m1, uint64_t m2, uint64_t counter_bits, uint64_t counter_init_val) {
  h->n = n;
  h->k = k;
  h->m1 = m1;
  h->m2 = m2;
  h->global_bhr = 0;
  h->chooser_table = malloc(sizeof(uint64_t) * sim_math_2pow(h->k));

  uint64_t two_pow_counter_bits = sim_math_2pow(counter_bits);
  h->_pc_bits_mask = sim_gen_mask(0, h->k);
  h->_counter_max = two_pow_counter_bits - 1;
  h->_counter_gshare_threshold = two_pow_counter_bits / 2;

  sim_gshare_init(&h->gshare, h->n, h->m1, GSHARE_COUNTER_BITS);
  sim_gshare_init(&h->bimodal, 0, h->m2, GSHARE_COUNTER_BITS);

  for (size_t i = 0; i < sim_math_2pow(k); i++) {
    h->chooser_table[i] = counter_init_val;
  }
}

void sim_hybrid_free(struct SimHybrid *h) {
  h->n = 0;
  h->k = 0;
  h->m1 = 0;
  h->m2 = 0;
  h->global_bhr = 0;
  free(h->chooser_table);
  sim_gshare_free(&h->gshare);
  sim_gshare_free(&h->bimodal);
}

bool sim_hybrid_predict_and_update(struct SimHybrid *h, struct SimInstruction *i) {
  uint64_t index = (i->address >> 2) & h->_pc_bits_mask;
  uint64_t counter = h->chooser_table[index];

  size_t gshare_index = sim_gshare_get_index(&h->gshare, i);
  size_t bimodal_index = sim_gshare_get_index(&h->bimodal, i);
  bool gshare_prediction = sim_gshare_predict(&h->gshare, gshare_index);
  bool bimodal_prediction = sim_gshare_predict(&h->bimodal, bimodal_index);
  bool hybrid_prediction = bimodal_prediction;
  if (counter >= h->_counter_gshare_threshold) {
    hybrid_prediction = gshare_prediction;
  }

  sim_gshare_update_gbhr(&h->gshare, i->taken);

  if (counter >= h->_counter_gshare_threshold) {
    sim_gshare_update_prediction_table(&h->gshare, gshare_index, i->taken);
  } else {
    sim_gshare_update_prediction_table(&h->bimodal, bimodal_index, i->taken);
  }

  if(gshare_prediction != bimodal_prediction) {
    if (gshare_prediction == i->taken) {
      if (counter < h->_counter_max) {
        h->chooser_table[index]++;
      }
    } else {
      if (counter > 0) {
        h->chooser_table[index]--;
      }
    }
  }

  return hybrid_prediction;
}

void sim_hybrid_print(struct SimHybrid *h, FILE *f) {
  fprintf(f, "FINAL CHOOSER CONTENTS\n");
  for (size_t i = 0; i < sim_math_2pow(h->k); i++) {
    fprintf(f, "%" PRIuPTR " %" PRIu64 "\n", i, h->chooser_table[i]);
  }
  sim_gshare_print(&h->gshare, f);
  sim_gshare_print(&h->bimodal, f);
}
