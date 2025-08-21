/**
 * @file buffer.c
 * @author YOUR NAME HERE
 */

#include "buffer.h"
#include "conventions.h"
#include <stdio.h>

/// @brief Initialize the buffer manager.
/// @param[in] manager       A pointer to a pre-allocated `BufferManager` struct
/// @param[in] file          The file stream to use as a backing for the buffer
/// manager
/// @param[in] frame_count   The number of frames to allocate in the buffer
/// manager
///
/// This method is called prior to any other method invocations.
Result buffer_init(BufferManager *manager, FILE *file, uint64_t frame_count) {
  // TODO
  return ERROR;
}

/// @brief Acquire a page from the backing, pinning it into memory
/// @param[in]  manager       A pointer to the `BufferManager`
/// @param[in]  page_id       The id of the page to load
/// @param[out] frame         Returns a `Frame*` pointer to the frame
///
/// - If no existing frames are storing page_id, allocate one
///     - If an UNALLOCATED frame is available, use this first
///     - If an UNPINNED_CLEAN frame is available, use this next
///     - If an UNPINNED_DIRTY frame is available, flush the frame (See
///     `flush_frame`), and then use it
///     - If all frames are in a PINNED_* state, return an error
/// - If an existing frame is storing page_id:
///     - If the existing frame is currently Unpinned and Clean, return that
///     - If the existing frame is currently Pinned, return an error
///     - If the existing frame is currently Dirty, force a flush and then
///     return it
Result buffer_pin(BufferManager *manager, PageID page_id, Frame **frame) {
  // TODO
  return ERROR;
}

/// @brief Mark the listed frame as being dirty.
/// @param[in] manager       A pointer to the `BufferManager`
/// @param[in] frame         A pointer to the frame to mark as dirty
///
/// - The listed frame must be in one of the the PINNED_* states.  If not,
/// return an error.
/// - After calling this function, it should be in the PINNED_DIRTY state.
Result buffer_mark(BufferManager *manager, Frame *frame) {
  // TODO
  return ERROR;
}

/// @brief Unpin the specified frame.
/// @param[in] manager       A pointer to the `BufferManager`
/// @param[in] frame         A pointer to the frame to unpin
///
/// - The listed frame must be in one of the PINNED_* states.  If not, return an
/// error.
/// - After calling this function, a PINNED_CLEAN page should be in the
/// UNPINNED_CLEAN state.
/// - After calling this function, a PINNED_DIRTY page should be in the
/// UNPINNED_DIRTY state.
///
/// Note that this should **not** delete the contents of the frame, only mark it
/// as unpinned, so that later accesses can re-use the data.
Result buffer_unpin(BufferManager *manager, Frame *frame) {
  // TODO
  return ERROR;
}

/// @brief Flush all unpinned frames to disk
/// @param[in] manager       A pointer to the `BufferManager`
///
/// All pages in the UNPINNED_DIRTY state should be flushed to disk and moved
/// into the UNPINNED_CLEAN state
Result flush_unpinned(BufferManager *manager) {
  // TODO
  return ERROR;
}

/// @brief Flush the provided frame to disk
/// @param[in] manager       A pointer to the `BufferManager`
/// @param[in] frame         A pointer to the frame to flush
///
/// - If the provided page is in a *_DIRTY state, it should be written to disk
/// at the appropriate page position
/// - If the provided page is in a *_CLEAN state, do nothing and return SUCCESS
/// - If the provided page is UNALLOCATED, return an error
Result flush_frame(BufferManager *manager, Frame *frame) {
  // TODO
  return ERROR;
}
