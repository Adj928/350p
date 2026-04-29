/**
 * @file buffer.c
 * @author Adonis Jackson
 */

#include "buffer.h"
#include <stdlib.h>
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
    if(manager == NULL){
        return ERROR;
    }
    if(file == NULL){
        return ERROR;
    }
    if(frame_count<1){
        return ERROR;
    }

    manager->frame_count = frame_count;
    manager->file = file;
    manager->frames = calloc(frame_count, sizeof(Frame));
    if(manager->frames == NULL){
        return ERROR;
    }
    manager->headers = calloc(frame_count, sizeof(FrameHeader));
    if(manager->headers == NULL){
        free(manager->frames);
        return ERROR;
    }
    for (uint64_t i = 0; i < frame_count; i++) {
            manager->headers[i].state = FRAME_UNALLOCATED;
        }

    return SUCCESS;
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
///     `buffer_flush_frame`), and then use it
///     - If all frames are in a PINNED_* state, return an error
/// - If an existing frame is storing page_id:
///     - If the existing frame is currently Unpinned and Clean, return that
///     - If the existing frame is currently Pinned, return an error
///     - If the existing frame is currently Dirty, force a flush and then
///     return it
Result buffer_pin(BufferManager *manager, PageID page_id, Frame **frame) {
  // TODO
    if (manager == NULL || frame == NULL) {
        return ERROR;
    }

    // Check if page is already loaded in a frame
    for (uint64_t i = 0; i < manager->frame_count; i++) {
        FrameHeader *head = &manager->headers[i];
        //correct id is stored in frame
        if (head->page_id == page_id && head->state != FRAME_UNALLOCATED) {
            if (head->state == FRAME_PINNED_CLEAN || head->state == FRAME_PINNED_DIRTY) {
                return ERROR;
            }

            if (head->state == FRAME_UNPINNED_DIRTY) {
                if (buffer_flush_frame(manager, &manager->frames[i]) != SUCCESS) {
                    return ERROR;
                }
            }

            head->state = FRAME_PINNED_CLEAN;
            *frame = &manager->frames[i];
            return SUCCESS;
        }
    }
    //noframe contains page id
    int64_t freeindex = -1;
    //check for unallocated frame
    for (uint64_t i = 0; i < manager->frame_count; i++) {
        FrameHeader *head = &manager->headers[i];

        if (head->state == FRAME_UNALLOCATED) {
            freeindex = i;
            break;
        }
    }
    //first check didn't pass now check for unpinned clean
    if (freeindex == -1) {
        for (uint64_t i = 0; i < manager->frame_count; i++) {
            if (manager->headers[i].state == FRAME_UNPINNED_CLEAN) {
                freeindex = i;
                break;
            }
        }
    }
    //second check didn't pass now check for unpinned dirty
    if (freeindex == -1) {
        for (uint64_t i = 0; i < manager->frame_count; i++) {
            if (manager->headers[i].state == FRAME_UNPINNED_DIRTY) {
                //flush dirty frame
                if (buffer_flush_frame(manager, &manager->frames[i]) != SUCCESS) {
                    return ERROR;
                }
                freeindex = i;
                break;
            }
        }
    }
    //all frames are pinned return error
    if (freeindex == -1) {
        return ERROR;
    }

    long offset = (page_id * PAGE_SIZE);
    //read file
    if (fseek(manager->file, offset, SEEK_SET) != 0) {
        return ERROR;
    }

    size_t bytesread = fread(manager->frames[freeindex], 1, PAGE_SIZE, manager->file);
    if (bytesread < PAGE_SIZE) {
       //if file is shorter than PAGE_SIZE pad  
       memset(manager->frames[freeindex] + bytesread, 0, PAGE_SIZE - bytesread);
    }

    FrameHeader *head = &manager->headers[freeindex];
    head->page_id = page_id;
    head->state = FRAME_PINNED_CLEAN;

    *frame = &manager->frames[freeindex];
    return SUCCESS;
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
  if(manager == NULL){
        return ERROR;
     }
  if(frame == NULL){
        return ERROR;
   }
   FrameHeader *head = NULL;
   
   for(uint64_t i = 0 ; i < manager->frame_count; i++ ){
    if(frame == &manager->frames[i]){
        head = &manager->headers[i];
        break;
    }
   }
   
   if(head == NULL){
    return ERROR;
   }
   
   if(head->state == FRAME_PINNED_CLEAN ){
    head->state = FRAME_PINNED_DIRTY;
    return SUCCESS;
   }
   
   if(head->state == FRAME_PINNED_DIRTY){
    head->state = FRAME_PINNED_DIRTY;
    return SUCCESS;
   }
  
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
   if(manager == NULL){
        return ERROR;
     }
  if(frame == NULL){
        return ERROR;
   }
  FrameHeader *head = NULL;
  for(uint64_t i = 0 ; i < manager->frame_count; i++ ){
    if(frame == &manager->frames[i]){
        head = &manager->headers[i];
        break;
    }
   }
   
   if(head == NULL){
    return ERROR;
   }
   if(head->state == FRAME_PINNED_CLEAN ){
    head->state = FRAME_UNPINNED_CLEAN;
    return SUCCESS;
   }
   
   if(head->state == FRAME_PINNED_DIRTY){
    head->state = FRAME_UNPINNED_DIRTY ;
    return SUCCESS;
   }
  return ERROR;
}

/// @brief Flush all unpinned frames to disk
/// @param[in] manager       A pointer to the `BufferManager`
///
/// All pages in the UNPINNED_DIRTY state should be flushed to disk and moved
/// into the UNPINNED_CLEAN state
Result buffer_flush_unpinned(BufferManager *manager) {
  // TODO
  if(manager == NULL){
        return ERROR;
     }
  FrameHeader *head = NULL;
  for (uint64_t i = 0; i < manager->frame_count; i++) {
        head = &manager->headers[i];

        if (head->state == FRAME_UNPINNED_DIRTY) {
            Frame *frame = &manager->frames[i];
            if (buffer_flush_frame(manager, frame) != SUCCESS) {
                return ERROR;
            }
            head->state = FRAME_UNPINNED_CLEAN; // update state after flushing
        }
    }

    return SUCCESS;
}

/// @brief Flush the provided frame to disk
/// @param[in] manager       A pointer to the `BufferManager`
/// @param[in] frame         A pointer to the frame to flush
///
/// - If the provided page is in a *_DIRTY state, it should be written to disk
/// at the appropriate page position
/// - If the provided page is in a *_CLEAN state, do nothing and return SUCCESS
/// - If the provided page is UNALLOCATED, return an error
Result buffer_flush_frame(BufferManager *manager, Frame *frame) {
  // TODO
  if(manager == NULL){
        return ERROR;
     }
  if(frame == NULL){
        return ERROR;
   }
  FrameHeader *head = NULL;
  PageID id = 0;
  for(uint64_t i = 0 ; i < manager->frame_count; i++ ){
    if(frame == &manager->frames[i]){
        head =&manager->headers[i];
        break;
    }
   }
   if(head == NULL){
    return ERROR;
   }
/// - If the provided page is UNALLOCATED, return an error
   if(head->state == FRAME_UNALLOCATED){
    return ERROR;
   }
/// - If the provided page is in a *_CLEAN state, do nothing and return SUCCESS
   if(head->state == FRAME_UNPINNED_CLEAN){
    return SUCCESS;
   }
   if(head->state == FRAME_PINNED_CLEAN ){
    return SUCCESS;
   }
/// - If the provided page is in a *_DIRTY state, it should be written to disk
/// at the appropriate page position
   if(head->state==FRAME_UNPINNED_DIRTY||head->state==FRAME_PINNED_DIRTY){
        id = head->page_id;
        long offset = (id * PAGE_SIZE);
        int res = fseek(manager->file,offset, SEEK_SET);
        if(res != 0){
            return ERROR;
        }
        size_t Bytewrite = fwrite(*frame,1,PAGE_SIZE,manager->file);
        fflush(manager->file);
        if(Bytewrite != PAGE_SIZE){
            return ERROR;
        }
        return SUCCESS;
   }

  return ERROR;
}
