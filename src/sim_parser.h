#pragma once

#include "sim_io.h"
#include <stdbool.h>
#include <stdint.h>

struct SimInstruction {
  int64_t address;
  bool taken;
};

struct SimParser {
  struct SimReadBuf buf;
};

int sim_parser_next_token(struct SimParser *parser, struct SimInstruction *token);
