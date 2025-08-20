/**
 * @file main.c
 * @author Oliver Kennedy
 */
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>

#define DB_FILE "database.350"

/// @brief Entrypoint
int main(int argc, char **argv) {
  FILE *db_file = fopen(DB_FILE, "a+");
  CHECK_ERROR(!db_file, "Error opening db");

  BufferManager manager;
  uint8_t initialized = 0;
  char cmd;

  while (1) {
    int ret = scanf("%c", &cmd);
    if (ret == EOF) {
      return 0;
    }
    CHECK_ERROR(ret < 0, "Error parsing command");
    switch (cmd) {
    case 'h': {
      printf("Hello World!\n");
    } break;

    case 'i': {
      unsigned int frame_count = 0;
      CHECK_ERROR(initialized, "Buffer manager already initialized");
      ret = scanf("%u", &frame_count);
      CHECK_ERROR(ret < 0, "Error parsing frame count");
      ret = buffer_init(&manager, db_file, frame_count);
      CHECK_ERROR(ret < 0, "Error initializing buffer manager");
      initialized = 1;
      printf("Buffer manager initialized\n");
    } break;
    }
  }

  return 0;
}
