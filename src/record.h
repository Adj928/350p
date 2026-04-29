/**
 * @file serialize.h
 * @author Oliver Kennedy
 */
#ifndef SERIALIZE_H_SHIELD
#define SERIALIZE_H_SHIELD

#include "conventions.h"
#include <stddef.h>
#include <stdint.h>

/// An encoded field.  This is just an alias for void* since the actual contents
/// for the field depends on the data itself.  We alias it here mainly to make 
/// functions more readable.
typedef void *Field;

/// An encoded record.  This is just an alias for void* since the structure
/// depends on the schema of the fields.  Still, we alias it here to make the
/// code more readable.
///
/// While your code will not be directly tested on the exact format you choose
/// to use here, it is expected that you will use something like the 'header' 
/// style of record encoding that we discussed in class.  Basically, this
/// means that:
/// 1. Your encoded records should store the length of the record internally
/// 2. Your encoded records should be able to retrieve each field without
///    knowing the types of data stored in other fields.
/// 3. Your encoded record should be no more than C+2*N+L bytes, where
///    N is the number of fields and L is the length of the raw data (4 bytes
///    for int and float each, and strlen(s)+1 for a string s), and C is some
///    small constant (4 for the reference implementation, but generally
///    10 or less)
///
/// An example encoding for (42, "foo", 9.3) might look like
/// ```
/// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// +  20 |   8 |  12 |  16 |   int 42  | f  o  o \0| float 9.3 |
/// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
/// ```
/// * Bytes 0-1 ("20"): the length of the record in bytes
/// * Bytes 2-3 ("8"): the first byte of field 0
/// * Bytes 4-5 ("12"): the first byte of field 1
/// * Bytes 6-7 ("16"): the first byte of field 2 
/// * Bytes 8-11 ("int 42"): the encoding of field 0
/// * Bytes 12-15 ("foo\0"): the encoding of field 1
/// * Bytes 16-19 ("float 9.3"): the encoding of field 2 
typedef void *Record;

size_t record_length(Record *record);
Result record_create(Record *record, size_t field_count, Field field_data[],
                     size_t field_length[], size_t max_bytes);
Result record_field_get(Record *record, size_t field_index, void **value,
                        size_t *length);

Result record_page_init(Frame *page);
size_t record_page_count(Frame *page);
Result record_page_put(Frame *page, size_t field_count, Field field_data[],
                       size_t field_length[], size_t *index);
Result record_page_update(Frame *page, size_t field_count, Field field_data[],
                          size_t field_length[], size_t index);
Result record_page_delete(Frame *page, size_t index);
Result record_page_get(Frame *page, size_t index, Record **record);
Result record_page_defragment(Frame *page);

#endif
