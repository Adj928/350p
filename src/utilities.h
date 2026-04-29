/**
 * @file utilities.h
 * @author Oliver Kennedy
 */
#ifndef UTILITIES_H_SHIELD
#define UTILITIES_H_SHIELD

#include "conventions.h"

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

Result compare_values(size_t a_len, void *a_val, size_t b_len, void *b_val,
                      DataType key_type, int *result);

#endif

