/**
 * @file buffer.c
 * @author YOUR NAME HERE
 */

#include "buffer.h"
#include "conventions.h"
#include <stdio.h>

Result buffer_init(BufferManager *manager, FILE *file, uint64_t frame_count) {
  // TODO
  return ERROR;
}

Result buffer_pin(BufferManager *manager, PageID page_id, Frame **frame) {
  // TODO
  return ERROR;
}

Result buffer_mark(BufferManager *manager, Frame *frame) {
  // TODO
  return ERROR;
}

Result buffer_unpin(BufferManager *manager, Frame *frame) {
  // TODO
  return ERROR;
}

Result flush_unpinned(BufferManager *manager) {
  // TODO
  return ERROR;
}

Result flush_frame(BufferManager *manager, Frame *frame) {
  // TODO
  return ERROR;
}
