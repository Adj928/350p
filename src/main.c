/**
 * @file main.c
 * @author Oliver Kennedy
 */
#include "buffer.h"
#include "conventions.h"
#include <stdio.h>
#include <stdlib.h>

/// @brief Utility macro for handling error checking
/// @param check The test to see if there is an error.
/// @param message The message to print if there is an error.
///
/// This macro does not return
#define CHECK_ERROR(check, message)                                            \
  if (check) {                                                                 \
    if (errno) {                                                               \
      perror(message);                                                         \
    } else {                                                                   \
      fprintf(stderr, "%s\n", message);                                        \
    }                                                                          \
    exit(-1);                                                                  \
  }

#define DB_FILE "database.350"

/// @brief Entrypoint
int main(int argc, char **argv) {
  FILE *db_file = fopen(DB_FILE, "a+");
  CHECK_ERROR(!db_file, "Error opening db");

  BufferManager manager;
  uint8_t initialized = 0;
  char cmd;
  unsigned int register_count = 0;
  Frame **registers;

  while (1) {
    // printf(">>");
    fflush(stdout);
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

      ret = scanf("%u %u", &frame_count, &register_count);
      CHECK_ERROR(ret < 0, "Error parsing frame count");

      ret = buffer_init(&manager, db_file, frame_count);
      CHECK_ERROR(ret, "Error initializing buffer manager");

      registers = calloc(register_count, sizeof(Frame *));
      CHECK_ERROR(registers == NULL, "Error initializing test registers");

      initialized = 1;
      printf("INIT\n");
    } break;

    case 'p': {
      unsigned int target_register = 0;
      PageID target_page = 0;

      ret = scanf("%u %lu", &target_register, &target_page);
      CHECK_ERROR(ret < 0, "Error parsing pin arguments");

      ret = buffer_pin(&manager, target_page, &registers[target_register]);
      CHECK_ERROR(ret, "Error pinning page");

      printf("PIN\n");
    } break;

    case 'u': {
      unsigned int target_register = 0;
      ret = scanf("%u", &target_register);
      CHECK_ERROR(ret < 0, "Error parsing unpin arguments");

      ret = buffer_unpin(&manager, registers[target_register]);
      CHECK_ERROR(ret, "Error unpinning page");

      printf("UNPIN\n");
    } break;

    case 'w': {
      unsigned int target_register = 0;

      ret = scanf("%u", &target_register);
      CHECK_ERROR(ret < 0, "Error parsing target_register");
      CHECK_ERROR(registers[target_register] == NULL,
                  "Target register is not in use");

      for (uint64_t i = 0; i < PAGE_SIZE; i++) {
        ret = scanf("%c", &(*registers[target_register])[i]);
        CHECK_ERROR(ret <= 0, "Error reading write data");
      }

      buffer_mark(&manager, registers[target_register]);

      printf("WRITE\n");
    } break;

    case 'r': {
      unsigned int target_register = 0;

      ret = scanf("%u", &target_register);
      CHECK_ERROR(ret < 0, "Error parsing target_register");
      CHECK_ERROR(registers[target_register] == NULL,
                  "Target register is not in use");

      printf("BEGIN READ\n");
      for (uint64_t i = 0; i < PAGE_SIZE; i++) {
        putc((*registers[target_register])[i], stdout);
      }
      printf("\nEND READ\n");
    } break;

    case 'f': {
      ret = buffer_flush_unpinned(&manager);
      CHECK_ERROR(ret, "Error flushing");

      printf("FLUSH\n");
    } break;
    }
  }

  return 0;
}
