/**
 * @file page.h
 * @author Oliver Kennedy
 */

#include "conventions.h"
#include <stdio.h>

/// @brief Read the contents of a page into a frame
/// @param[in] file      The file to read from
/// @param[in] page_id   The page id to read at
/// @param[out] frame    A reference to the frame to write into
Result page_read(FILE *file, PageID page_id, Frame frame) {
  // TODO
  return ERROR;
}

/// @brief Write the contents of a frame onto a page
/// @param[in] file      The file to write to
/// @param[in] page_id   The page id to write at
/// @param[in] frame     A reference to the frame to read from
Result page_write(FILE *file, PageID page_id, Frame frame) {
  // TODO
  return ERROR;
}
