
#include "conventions.h"
#include <string.h>

/// @brief Compare a pair of data values.
/// @param[in]  a_len    The length of the first parameter
/// @param[in]  a_val    A pointer to the value of the first parameter
/// @param[in]  b_len    The length of the second parameter
/// @param[in]  b_val    A pointer to the value of the second parameter
/// @param[in]  val_type The type of both values
/// @param[out] result   A negative value if a is lesser, positive if b is
///                      lesser, and zero if a=b
/// @return              ERROR if the arguments are nonsensical
Result compare_values(size_t a_len, void *a_val, size_t b_len, void *b_val,
                      DataType val_type, int *result) {
  switch (val_type) {

  case STRING: {
    size_t shorter = a_len < b_len ? a_len : b_len;
    *result = strncmp(a_val, b_val, shorter);
    if (*result == 0) {
      if (a_len < b_len) {
        *result = -1;
      } else if (a_len > b_len) {
        *result = 1;
      }
    }
    return SUCCESS;
  } break;
  case INT_64: {
    if (a_len != 8) {
      return ERROR;
    }
    if (b_len != 8) {
      return ERROR;
    }
    int64_t *a = a_val;
    int64_t *b = b_val;
    if (*a < *b) {
      *result = -1;
    } else if (*a > *b) {
      *result = 1;
    } else {
      *result = 0;
    }
  } break;
  case FLOAT_64: {
    if (a_len != 8) {
      return ERROR;
    }
    if (b_len != 8) {
      return ERROR;
    }
    double *a = a_val;
    double *b = b_val;
    if (*a < *b) {
      *result = -1;
    } else if (*a > *b) {
      *result = 1;
    } else {
      *result = 0;
    }
  } break;
  }
  return ERROR;
}
