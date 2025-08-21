/**
 * @file main.c
 * @author Oliver Kennedy
 */
#include "conventions.h"
#include "page.h"
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

    case 'W': {
      Frame temp_frame;
      PageID target_page;
      ret = scanf("%lu", &target_page);
      CHECK_ERROR(ret < 0, "Error parsing target page");

      for (uint64_t i = 0; i < PAGE_SIZE; i++) {
        ret = scanf("%c", &temp_frame[i]);
        CHECK_ERROR(ret <= 0, "Error reading write data");
      }

      ret = page_write(db_file, target_page, temp_frame);
      CHECK_ERROR(ret, "Error writing page");

      printf("PAGE_WRITE\n");
    } break;

    case 'R': {
      Frame temp_frame;
      PageID target_page;
      ret = scanf("%lu", &target_page);
      CHECK_ERROR(ret < 0, "Error parsing target page");

      ret = page_read(db_file, target_page, temp_frame);
      CHECK_ERROR(ret, "Error writing page");

      printf("PAGE_READ\n");
    } break;

    case 'f': {
      ret = fflush(db_file);
      CHECK_ERROR(ret, "Error flushing");

      printf("FLUSH\n");
    } break;
    }
  }

  return 0;
}
