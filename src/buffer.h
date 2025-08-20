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
  PAGE_UNALLOCATED = 0,
  PAGE_UNPINNED_CLEAN = 1,
  PAGE_UNPINNED_DIRTY = 2,
  PAGE_PINNED_CLEAN   = 3,
  PAGE_PINNED_DIRTY   = 4,
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


/// @brief Initialize the buffer manager.
/// @param[in] manager       A pointer to a pre-allocated `BufferManager` struct
/// @param[in] file          The file stream to use as a backing for the buffer manager
/// @param[in] frame_count   The number of frames to allocate in the buffer manager
/// 
/// This method is called prior to any other method invocations.
Result buffer_init(BufferManager *manager, FILE *file, uint64_t frame_count);

/// @brief Acquire a page from the backing, pinning it into memory
/// @param[in]  manager       A pointer to the `BufferManager`
/// @param[in]  page_id       The id of the page to load
/// @param[out] frame         Returns a `Frame*` pointer to the frame
///
/// - If no existing frames are storing page_id, allocate one
///     - If an UNALLOCATED frame is available, use this first
///     - If an UNPINNED_CLEAN frame is available, use this next
///     - If an UNPINNED_DIRTY frame is available, flush the frame (See `flush_frame`), and then use it
///     - If all frames are in a PINNED_* state, return an error
/// - If an existing frame is storing page_id:
///     - If the existing frame is currently Unpinned and Clean, return that
///     - If the existing frame is currently Pinned, return an error
///     - If the existing frame is currently Dirty, force a flush and then return it
Result buffer_pin(BufferManager *manager, PageID page_id, Frame **frame);

/// @brief Mark the listed frame as being dirty. 
/// @param[in] manager       A pointer to the `BufferManager`
/// @param[in] frame         A pointer to the frame to mark as dirty
///
/// - The listed frame must be in one of the the PINNED_* states.  If not, return an error.
/// - After calling this function, it should be in the PINNED_DIRTY state.
Result buffer_mark(BufferManager *manager, Frame *frame);

/// @brief Unpin the specified frame.
/// @param[in] manager       A pointer to the `BufferManager`
/// @param[in] frame         A pointer to the frame to unpin
///
/// - The listed frame must be in one of the PINNED_* states.  If not, return an error.
/// - After calling this function, a PINNED_CLEAN page should be in the UNPINNED_CLEAN state.
/// - After calling this function, a PINNED_DIRTY page should be in the UNPINNED_DIRTY state.
/// 
/// Note that this should **not** delete the contents of the frame, only mark it as unpinned, so that later
/// accesses can re-use the data.
Result buffer_unpin(BufferManager *manager, Frame *frame);

/// @brief Flush all unpinned frames to disk
/// @param[in] manager       A pointer to the `BufferManager`
///
/// All pages in the UNPINNED_DIRTY state should be flushed to disk and moved into the UNPINNED_CLEAN state
Result flush_unpinned(BufferManager *manager);

/// @brief Flush the provided frame to disk
/// @param[in] manager       A pointer to the `BufferManager`
/// @param[in] frame         A pointer to the frame to flush
///
/// - If the provided page is in a *_DIRTY state, it should be written to disk at the appropriate page position
/// - If the provided page is in a *_CLEAN state, do nothing and return SUCCESS
/// - If the provided page is UNALLOCATED, return an error
Result flush_frame(BufferManager *manager, Frame *frame);
#endif
