#include "sim_io.h"
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int sim_read_buf_init(struct SimReadBuf *buf, unsigned int capacity) {
  buf->buf = malloc(capacity * sizeof(char));
  if (buf->buf == NULL) {
    return -1;
  }
  memset(buf->buf, 0, capacity);

  buf->index = 0;
  buf->size = 0;
  buf->capacity = capacity;
  return 0;
}

void sim_read_buf_free(struct SimReadBuf *buf) {
  free(buf->buf);
  buf->capacity = 0;
  buf->size = 0;
  buf->index = 0;
}

// Since this function is called for each character, hint compiler to inline
// the function to reduce function call overhead.
inline int sim_read_buf_next_char(struct SimReadBuf *buf, char *c) {
  if (buf->size == 0 || buf->index >= buf->size) {
    errno = 0;
    ssize_t len = read(buf->fd, buf->buf, buf->capacity);
    if (len == 0) {
      return 0;
    } else if (len < 1) {
      return -1;
    }
    buf->index = 0;
    buf->size = len;
  }

  *c = buf->buf[buf->index++];
  return 1;
}
