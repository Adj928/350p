/**
 * @file main.c
 * @author Oliver Kennedy
 */
#include "conventions.h"
#include "page.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  FILE *db_file = fopen(DB_FILE, "w+");
  CHECK_ERROR(!db_file, "Error opening db");

  char cmd;

  for (int a = 1; a < argc; a++) {
    FILE *cmd_file;
    if (strlen(argv[a]) == 1 && argv[a][0] == '-') {
      cmd_file = stdin;
    } else {
      printf("Running: '%s'...\n", argv[a]);
      cmd_file = fopen(argv[a], "r");
      CHECK_ERROR(!cmd_file, "Error opening command file")
    }

    while (!feof(cmd_file)) {
      int ret = fscanf(cmd_file, "%c", &cmd);
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
        ret = fscanf(cmd_file, "%lu", &target_page);
        CHECK_ERROR(ret < 0, "Error parsing target page");

        for (uint64_t i = 0; i < PAGE_SIZE; i++) {
          ret = fscanf(cmd_file, "%c", &temp_frame[i]);
          CHECK_ERROR(ret <= 0, "Error reading write data");
        }

        ret = page_write(db_file, target_page, temp_frame);
        CHECK_ERROR(ret, "Error writing page");

        printf("WRITE_FRAME");
      } break;

      case 'R': {
        Frame temp_frame;
        PageID target_page;
        ret = fscanf(cmd_file, "%lu", &target_page);
        CHECK_ERROR(ret < 0, "Error parsing target page");

        printf("BEGIN PAGE_READ\n");
        for (uint64_t i = 0; i < PAGE_SIZE; i++) {
          putc(temp_frame[i], stdout);
        }
        printf("\nEND PAGE_READ\n");

        ret = page_read(db_file, target_page, temp_frame);

      } break;

      case 'f': {
        ret = fflush(db_file);
        CHECK_ERROR(ret, "Error flushing");

        printf("FLUSH\n");
      } break;
      }

      ret = fclose(cmd_file);
      CHECK_ERROR(ret, "Error closing files");
    }
  }

  return 0;
}
