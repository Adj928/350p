/**
 * @file page.c
 * @author Adonis Jackson
 */

#include "conventions.h"
#include <stdio.h>

/// @brief Read the contents of a page into a frame
/// @param[in] file      The file to read from
/// @param[in] page_id   The page id to read at
/// @param[out] frame    A reference to the frame to write into
Result page_read(FILE *file, PageID page_id, Frame frame) {
  // TODO
  if(file ==NULL || frame== NULL){
      return ERROR;
    }
  //calculate offset
  long offset = (page_id * PAGE_SIZE);

  int res = fseek(file,offset, SEEK_SET);
  if(res != 0){
    return ERROR;
  }

  size_t Byteread = fread(frame, 1, PAGE_SIZE, file);
  if(Byteread != PAGE_SIZE){
    return ERROR; 
  }

  return SUCCESS;
}

/// @brief Write the contents of a frame onto a page
/// @param[in] file      The file to write to
/// @param[in] page_id   The page id to write at
/// @param[in] frame     A reference to the frame to read from
Result page_write(FILE *file, PageID page_id, Frame frame) {
  // TODO
  if(file ==NULL || frame== NULL){
      return ERROR;
  }
  long offset = (page_id * PAGE_SIZE);

  int res = fseek(file,offset, SEEK_SET);
  if(res != 0){
    return ERROR;
  }

  size_t Bytewrite = fwrite(frame, 1, PAGE_SIZE,file);
  if(Bytewrite != PAGE_SIZE){
    return ERROR;
  }

  return SUCCESS;
    
}

