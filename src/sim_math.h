#pragma once

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * Convert string to integer (int64_t).
 * @param[in]  str String input
 * @param[out] result Pointer where resulting number should be stored.
 * @return 0 if the number is successfully converted.
 *         ERANGE if string input is out of int64_t range
 *         -1 if string is not just numeric value
 */
int sim_math_strtoint64(char *str, int64_t *result);

/**
 * Compute power of 2.
 * @param[in]  exp Power of 2 to compute
 * @return 2^exp
 */
uint64_t sim_math_2pow(uint64_t exp);

/*
 * Generate mask starting at start and running for length run.
 * @param[in]  start Starting bit index (LSB) of the mask.
 * @param[in]  run Length of the mask from starting bit index.
 * @return Mask of length run starting at start.
 *
 * Example:
 * sim_gen_mask(0, 3) = 0b111
 * sim_gen_mask(1, 3) = 0b1110
 * sim_gen_mask(2, 3) = 0b11100
 */
uint64_t sim_gen_mask(uint64_t start, uint64_t run);
