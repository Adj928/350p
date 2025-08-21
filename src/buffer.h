/**
 * @file buffer.h
 * @author Oliver Kennedy
 */
#ifndef BUFFER_H_SHIELD
#define BUFFER_H_SHIELD

#include "conventions.h"
#include <stdint.h>
#include <stdio.h>

/**
 * Allow the frame to be overridden by a compile flag (-DFRAME_SIZE=...), but
 * default to 4098
 */
#ifndef PAGE_SIZE
#define PAGE_SIZE 4098
#endif

/**
 * A single frame is an array of bytes
 */
typedef uint8_t Frame[PAGE_SIZE];

/**
 * A page can be dirty or clean (has content that needs to be written back), and pinned or not (available to be
 * overwritten by another page).  A page can also be completely unallocated (at first)
 */
typedef enum FrameState {
  FRAME_UNALLOCATED = 0,
  FRAME_UNPINNED_CLEAN = 1,
  FRAME_UNPINNED_DIRTY = 2,
  FRAME_PINNED_CLEAN   = 3,
  FRAME_PINNED_DIRTY   = 4,
} FrameState;

/**
 * Status information for a frame.
 */
typedef struct FrameHeader {
  /** 
   * The ID of the page in this frame
   */
  PageID page_id;
  
  /**
   * The current frame state
   */
  FrameState state;

  /**
   * A pointer you can use for state tracking
   */
  struct FrameHeader *next;
} FrameHeader;

/**
 * Context for the buffer manager
 */
typedef struct BufferManager {
  /**
   * The file backing the buffer manager
   */
  FILE *file;
  
  /**
   * The total number of frames allocated by this buffer manager.  .status[i] and .headers[i] should
   * be defined for 0 <= i < .frame_count
   */
  uint64_t frame_count;
  
  /**
   * An array of status objects of size .frame_count.  Each entry .status[i] corresponds to .headers[i].
   */
  FrameHeader *headers;
  
  /**
   * An array of frame buffers of size .frame_count.  Eachentry .headers[i] corresponds to .status[i]
   */
  Frame *frames;

  /**
   * A pointer to an additional object that you can use to store anything you want.
   */
  void *extra_data;
} BufferManager;

Result buffer_init(BufferManager *manager, FILE *file, uint64_t frame_count);
Result buffer_pin(BufferManager *manager, PageID page_id, Frame **frame);
Result buffer_mark(BufferManager *manager, Frame *frame);
Result buffer_unpin(BufferManager *manager, Frame *frame);
Result flush_unpinned(BufferManager *manager);
Result flush_frame(BufferManager *manager, Frame *frame);
#endif
