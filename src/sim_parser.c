#include "sim_parser.h"
#include "sim_io.h"
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int sim_parser_next_token(struct SimParser *parser,
                          struct SimInstruction *inst) {
  // 16 hex characters is enough to represent 64 bit address.
  // Math: 64/4 == 16. This is because each hex character represents 16 bits.
  char token[16];
  int token_index = 0;

  int read_status;
  char c;
  while ((read_status = sim_read_buf_next_char(&parser->buf, &c) > 0)) {
    switch (c) {
    case ' ':
      token[token_index++] = '\0'; // Append \0 to indicate string termination.
      // TODO: Handle error condition if strtoll fails.
      inst->address = strtoll(token, NULL, 16);
      token_index = 0;
      break;
    case '\r':
      // TODO: Should \r only line endings be supported?
      break;
    case '\n':
      switch (token[token_index - 1]) {
      case 't':
        inst->taken = true;
        break;
      case 'n':
        inst->taken = false;
        break;
      default:
        // TODO: Handle error.
        break;
      }
      return 1;
    default:
      token[token_index++] = c;
      break;
    }
  }

  return read_status;
}
