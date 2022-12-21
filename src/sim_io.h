#pragma once

#include <sys/types.h>

/**
 * SimReadBuf provides buffered read from the underlying file descriptor.
 * @var fd Underlying file descriptor.
 * @var buf Buffer
 * @var capacity Capacity of the buffer
 * @var size Current size of the buffer
 * @var index Seek index. Anything between index-size is buffered.
 */
struct SimReadBuf {
  int fd;
  char *buf;
  unsigned int capacity;
  unsigned int size;
  unsigned int index;
};

/**
 * Initilize read buffer
 * @param[in] buf Uninitialized SimReadBuf.
 * @param[in] capacity Size of the buffer memory to allocate.
 * @return 0 if the number is initialization and memory allocation is successful
 *        -1 if initialization or malloc is a failure.
 */
int sim_read_buf_init(struct SimReadBuf *buf, unsigned int capacity);

/**
 * Free read buffer
 * @param[in] buf Initialized SimReadBuf.
 */
void sim_read_buf_free(struct SimReadBuf *buf);

/**
 * Get next character from the read buffer.
 * @param[in]  buf Initialized SimReadBuf.
 * @param[out] c Pointer where next character should be stored.
 * @return 1 if the operation is successful.
 *         0 if the underlying file reaches EOF.
 *        -1 if there is a read failure.
 */
int sim_read_buf_next_char(struct SimReadBuf *buf, char *c);
