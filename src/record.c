#include "conventions.h"
#include "record.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef DEBUG_RECORD
#define DEBUG_RECORD 0
#endif
#define DEBUG                                                                  \
  if (DEBUG_RECORD)                                                            \
  printf

#define RECORD_HEADER_SIZE(fields) ((fields + 2) * sizeof(short int))

/// @brief Obtain the length (in bytes) of the provided record.
/// @param[in] record  The record to obtain the length of.
/// @return            The number of bytes required to store record.
size_t record_length(Record *record) {
  unsigned short *header = (unsigned short *)record;
  unsigned short field_count = header[0];
  return header[field_count + 1];
}

size_t estimate_length(size_t field_count, size_t field_length[]) {
  size_t needed_bytes = RECORD_HEADER_SIZE(field_count);
  for (int i = 0; i < field_count; i++) {
    needed_bytes += field_length[i];
  }
  return needed_bytes;
}

/// @brief Encode an array of boxed fields into a record.
/// @param[in] record        A pre-allocated region of memory to write the
///                          record to.
/// @param[in] field_count   The number of fields in the array.
/// @param[in] field_data    The array of field data
/// @param[in] field_length  The array of field lengths
/// @param[in] max_bytes     The maximum number of bytes to write.  It is an
///                          error if there are not enough bytes to write
///                          the record.
/// @return                  SUCCESS if the record was successfully written
///                          and ERROR otherwise
///
/// See the notes on `Record` for some ideas on how to organize the written
/// records.  In general, you shoult treat the data values in each field as
/// completely opaque (let the caller worry about what's stored there). Just
/// make sure to track the length of each field.
Result record_create(Record *record, size_t field_count, Field field_data[],
                     size_t field_length[], size_t max_bytes) {
  if (estimate_length(field_count, field_length) > max_bytes) {
    return ERROR;
  }
  unsigned short *header = (unsigned short *)record;
  header[0] = field_count;
  unsigned short offset = RECORD_HEADER_SIZE(field_count);
  header[1] = offset;
  for (int i = 0; i < field_count; i++) {
    char *target = ((char *)record) + offset;
    DEBUG("Offset: %d -> %p (header @ %p)\n", offset, target, header);
    memcpy(target, field_data[i], field_length[i]);
    offset += field_length[i];
    header[i + 2] = offset;
  }
  return SUCCESS;
}

/// @brief Decode a field of the provided record.
/// @param[in] record      The address of the record to read from
/// @param[in] field_index The index of the field to read
/// @param[out] value      Output parameter that will be assigned to the memory
///                        address where the field starts.
/// @param[out] length     Output parameter that will be assigned to the length
///                        of the field in bytes.
/// @return                SUCCESS if value was successfully assigned to the
///                        address
Result record_field_get(Record *record, size_t field_index, void **value,
                        size_t *length) {
  unsigned short *header = (unsigned short *)record;
  unsigned short start = header[1 + field_index];
  unsigned short end = header[2 + field_index];
  *value = (void *)(((char *)record) + start);
  *length = end - start;
  return SUCCESS;
}

/// @brief Initialize a record page
/// @param[in] page        A page of data to be initialized as a record page
///
/// While you will not be tested on the specific layout you use, you are
/// encouraged to use the footer-style layout we discussed in class.  The
/// following example uses a 40 byte PAGE_SIZE with two records of sizes 8
/// and 10, respectively.
///
/// ```
/// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// | RECORD 1              | RECORD 2                    | FREE|
/// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20
///
/// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// | FREE                              |  8  |  0  | 18  |  2  |
/// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40
/// ```
/// * Bytes 0-7: The 8 bytes of record 0
/// * Bytes 8-17: The 10 bytes of record 1
/// * Bytes 38-39: The number of records 'allocated' (2)
/// * Bytes 36-37: The first 'free' byte (18)
/// * Bytes 34-35: The first byte of record 0 (0)
/// * Bytes 32-33: THe first byte of record 1 (8)
Result record_page_init(Frame *page) {
  bzero(page, PAGE_SIZE);
  return SUCCESS;
}

#define FOOTER_FIELD(page, i)                                                  \
  ((unsigned short *)page)[PAGE_SIZE / sizeof(unsigned short) - 1 - (i)]
#define UNALLOCATED ((255 << 8) + 255)

/// @brief The count of the number of allocated records
/// @param page  The page to query
/// @return      The number of records (including unused slots) that have been
///              allocated on the page
///
/// The function record_page_get() should behave correctly
/// if called on any index between 0 and the value returned
/// by this function.
size_t record_page_count(Frame *page) { return FOOTER_FIELD(page, 0); }

/// @brief Create a record on the provided page (see record_field_create).
/// @param[in] page         The page to put the record into
/// @param[in] field_count  The number of fields of the record to insert
/// @param[in] field_data   The data values of the record to insert
/// @param[in] field_length The length of each data value
/// @param[out] index       Output parameter that will be set to the index of
///                         at which the record was written
/// @return                 SUCCESS if the record was successfully put and
///                         ERROR otherwise.
///
/// If insufficient free space is immediately available, you should use
/// `record_page_defrag` to try to free space.  If no space is available
/// even then, then put should fail.
Result record_page_put(Frame *page, size_t field_count, Field field_data[],
                       size_t field_length[], size_t *index) {

  unsigned short footer_size = (FOOTER_FIELD(page, 0) + 2) * 2;
  unsigned short next_free = FOOTER_FIELD(page, 1);
  DEBUG("Writing at %d (%d already used with %d records)\n", next_free,
        footer_size + next_free, FOOTER_FIELD(page, 0));
  unsigned short available = PAGE_SIZE - footer_size - next_free;
  Result ret;

  // If there's not enough space, then try to defragment
  if (estimate_length(field_count, field_length) >= available) {
    ret = record_page_defragment(page);
    if (ret) {
      return ret;
    }

    // On a successful defragment, the free pointer and available space will
    // change
    next_free = FOOTER_FIELD(page, 1);
    available = PAGE_SIZE - footer_size - next_free;
    DEBUG("Defragment successful\n");
    if (estimate_length(field_count, field_length) >= available) {
      DEBUG("... but still not enough space\n");
      return ERROR;
    }
  }
  unsigned short selected_index = UNALLOCATED;
  for (int i = 0; i < FOOTER_FIELD(page, 0); i++) {
    if (FOOTER_FIELD(page, i + 2) == UNALLOCATED) {
      selected_index = i;
      break;
    }
  }
  if (selected_index == UNALLOCATED) {
    selected_index = FOOTER_FIELD(page, 0);
    FOOTER_FIELD(page, 0) += 1;
  }
  DEBUG("Writing index %d\n", selected_index);

  Record *record = (Record *)(((char *)page) + next_free);
  ret = record_create(record, field_count, field_data, field_length, available);
  if (ret) {
    return ret;
  }
  unsigned short record_size = record_length(record);
  FOOTER_FIELD(page, 1) += record_size;
  FOOTER_FIELD(page, selected_index + 2) = next_free;
  *index = selected_index;

  return SUCCESS;
}

/// @brief Replace a record on the provided page (see record_field_create).
/// @param[in] page         The page on which to update the record
/// @param[in] field_count  The number of fields of the record to insert
/// @param[in] field_data   The data values of the record to insert
/// @param[in] field_length The length of each data value
/// @param[in] index        The index of the record to replace.
/// @return                 SUCCESS if the record was successfully updated and
///                         ERROR otherwise.
///
/// If insufficient free space is immediately available, you should use
/// `record_page_defrag` to try to free space.  If no space is available
/// even then, then put should fail.
Result record_page_update(Frame *page, size_t field_count, Field field_data[],
                          size_t field_length[], size_t index) {
  Result ret;

  Record *existing_record =
      (Record *)((char *)page + FOOTER_FIELD(page, 2 + index));
  unsigned short existing_size = record_length(existing_record);

  // Try to re-use the existing space if possible, but limit creation by the
  // existing size of the record
  ret = record_create(existing_record, field_count, field_data, field_length,
                      existing_size);
  if (ret == SUCCESS) {
    return SUCCESS;
  }

  unsigned short footer_size = (FOOTER_FIELD(page, 0) + 2) * 2;
  unsigned short next_free = FOOTER_FIELD(page, 1);
  unsigned short available = PAGE_SIZE - footer_size - next_free;

  Record *record = (Record *)(((char *)page) + next_free);
  ret = record_create(record, field_count, field_data, field_length, available);
  if (ret) {
    // not enough space.  Defrag and try again
    record_page_delete(page, index);
    ret = record_page_defragment(page);
    if (ret) {
      return ret;
    }
    next_free = FOOTER_FIELD(page, 1);
    available = PAGE_SIZE - footer_size - next_free;
    record = (Record *)(((char *)page) + next_free);
    ret =
        record_create(record, field_count, field_data, field_length, available);
    if (ret) {
      return ret;
    }
  }
  unsigned short record_size = record_length(record);
  FOOTER_FIELD(page, 1) += record_size;
  FOOTER_FIELD(page, index + 2) = next_free;

  return SUCCESS;
}

/// @brief Delete a record from the provided page
/// @param[in] page        The page to delete the record from
/// @param[in] index       The index of the record to delete.
///
/// Deletion should preserve record indexes.  That is, a record at index N
/// should remain at index N even after deletion.  One way to accomplish this is
/// to use a special index value to signify a deleted record.
Result record_page_delete(Frame *page, size_t index) {
  FOOTER_FIELD(page, index + 2) = UNALLOCATED;
  return SUCCESS;
}

/// @brief Retrieve a record from the provided page
/// @param[in] page        The page to get the record from
/// @param[in] index       The index of the record to retrieve
/// @param[out] record     Output parameter to assign to the address of the
///                        retrieved record
/// @return                SUCCESS if the record was successfully retrieved, and
///                        ERROR otherwise, such as for example if the record
///                        at this index was deleted.
Result record_page_get(Frame *page, size_t index, Record **record) {
  if (FOOTER_FIELD(page, index + 2) == UNALLOCATED) {
    return ERROR;
  }
  *record = (void *)(((char *)page) + FOOTER_FIELD(page, index + 2));
  return SUCCESS;
}

/// @brief Defragment the records on the provided page.
/// @param[in] page        The page to defragment.
///
/// Rearrange the layout of the records on the provided page to free up space
/// from e.g., deleted records.
Result record_page_defragment(Frame *page) {
  DEBUG("Defragmenting page (%u next free)\n", FOOTER_FIELD(page, 1));
  Frame buffer;
  memcpy(&buffer, page, PAGE_SIZE);
  bzero(page, PAGE_SIZE);
  FOOTER_FIELD(page, 0) = FOOTER_FIELD(&buffer, 0);
  unsigned short next_free = 0;
  DEBUG("defragmenting %p -> %p\n", &buffer, page);
  for (int i = 0; i < FOOTER_FIELD(&buffer, 0); i++) {
    if (FOOTER_FIELD(&buffer, 2 + i) != UNALLOCATED) {
      Record *buffer_record =
          (void *)(((char *)&buffer) + FOOTER_FIELD(&buffer, 2 + i));
      unsigned short length = record_length(buffer_record);
      Record *page_record = (void *)(((char *)page) + next_free);
      DEBUG("copying record %u (%u bytes) from %p to %p\n", i, length,
            buffer_record, page_record);
      memcpy(page_record, buffer_record, length);
      FOOTER_FIELD(page, 2 + i) = next_free;
      next_free += length;
    } else {
      FOOTER_FIELD(page, 2 + i) = UNALLOCATED;
    }
  }
  DEBUG("After defrag, next free byte at %u\n", next_free);
  FOOTER_FIELD(page, 1) = next_free;
  return SUCCESS;
}
