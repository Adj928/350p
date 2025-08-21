/**
 * @file file.h
 * @author Oliver Kennedy
 */
#ifndef FILE_H_SHIELD
#define FILE_H_SHIELD

#include "conventions.h"
#include <stdio.h>

Result page_read(FILE *file, PageID page_id, Frame frame);
Result page_write(FILE *file, PageID page_id, Frame frame);

#endif
