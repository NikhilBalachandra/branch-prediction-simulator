#include "sim_math.h"

int sim_math_strtoint64(char *str, int64_t *result) {
  char *end_str = NULL;

  errno = 0;
  *result = strtoll(str, &end_str, 10);
  if (errno != 0) {
    return errno;
  } else if (*end_str != '\0') {
    return EINVAL;
  } else if (end_str == str) {
    // Happens if input is empty string.
    return EINVAL;
  }

  return 0;
}

inline uint64_t sim_math_2pow(uint64_t exp) {
  return 1UL << exp;
}

inline uint64_t sim_gen_mask(uint64_t start, uint64_t run) {
  return ((UINT64_C(1) << run) - UINT64_C(1)) << start;
}
