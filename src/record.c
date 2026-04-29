#include "conventions.h"
#include "record.h"
#include "string.h"
/// @brief Obtain the length (in bytes) of the provided record.
/// @param[in] record  The record to obtain the length of.
/// @return            The number of bytes required to store record.
size_t record_length(Record *record) { 
  if(record == NULL){
    return 0;
  }
   uint16_t *meta = (uint16_t *)record;
  size_t fieldcount = meta[0];
  size_t total = sizeof(uint16_t) * (fieldcount + 1);
  
  for (size_t i = 0; i < fieldcount; i++) {
        total += meta[i+1];
    }

  return total;
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
Result record_create(Record *record, size_t field_count, Field field_data[],size_t field_length[], size_t max_bytes) {
  if(record==NULL){
    return ERROR;
  }
  if(field_count>0 && field_data==NULL){
    return ERROR;
  }
   if(field_count>0 && field_length==NULL){
    return ERROR;
  }
  uint16_t *meta = (uint16_t *)record;
  size_t headb = sizeof(uint16_t) * (field_count + 1);
  size_t datab = 0;
  for (size_t i = 0; i < field_count; i++) {
     datab += field_length[i];
  }
  size_t total = datab + headb;
  if(total>max_bytes){
    return ERROR;
  }

  meta[0] = field_count;
  for (size_t i = 0; i < field_count; i++) {
      meta[i+1] = field_length[i];
    }

  
    unsigned char *base = (unsigned char *)record;
    size_t off = headb;
    for (size_t i = 0; i < field_count; i++) {
    if (field_length[i] > 0 && field_data[i] != NULL) {
        memcpy(base + off, field_data[i], field_length[i]);
        off += field_length[i];
    }
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
  if(record==NULL){
    return ERROR;
  }
  if(value == NULL){
    return ERROR;
  }
  if(length == NULL){
    return ERROR;
  }
  uint16_t *meta = (uint16_t *)record;
  size_t fieldcount = meta[0];
  size_t headb = sizeof(uint16_t) * (fieldcount + 1);

  if(field_index>= fieldcount){
    return ERROR;
  }

  unsigned char *base = (unsigned char *)record;
  size_t off = headb;
   
  for (size_t i = 0; i < field_index; i++) {
    off += meta[i + 1];
  }

  *value = base + off;
  *length = meta[field_index + 1];

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
  if(page==NULL){
    return ERROR;
  }
  //* Bytes 38-39: The number of records 'allocated' (2)
  uint8_t *pageby = (uint8_t *)page;
  *( uint16_t *)(pageby + PAGE_SIZE - 2) = 0;
  
  //Bytes 36-37: The first 'free' byte (18)
  
  *( uint16_t *)(pageby + PAGE_SIZE - 4) = 1;

  return SUCCESS; }

/// @brief The count of the number of allocated records
/// @param page  The page to query
/// @return      The number of records (including unused slots) that have been
///              allocated on the page
///
/// The function record_page_get() should behave correctly
/// if called on any index between 0 and the value returned
/// by this function.
size_t record_page_count(Frame *page) { 
  uint8_t *pageby = (uint8_t *)page;
  size_t countofrec = *(uint16_t *)(pageby + PAGE_SIZE - 2);
  return countofrec; }

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
  if(page==NULL){
    return ERROR;
  }
  if(index==NULL){
    return ERROR;
  }
  if(field_count>0){
    if(field_data==NULL){
      return ERROR;
    }
    if(field_length==NULL){
    return ERROR;
    }
  }
  uint8_t *pageby = (uint8_t *)page;

   uint16_t reccount = *( uint16_t *)(pageby + PAGE_SIZE - 2);
  
  
   uint16_t freep = *( uint16_t *)(pageby + PAGE_SIZE - 4);

  size_t total = sizeof(uint16_t) * (field_count + 1);

  for (size_t i = 0; i < field_count; i++) {
        total += field_length[i];
    }

   size_t need = (reccount + 1) * 2 + 4;
    if (PAGE_SIZE < total + freep + need ) {
        if (record_page_defragment(page) != SUCCESS){
           return ERROR;
        }
        
        reccount = *( uint16_t*)(pageby + PAGE_SIZE - 2);
        freep = *( uint16_t*)(pageby + PAGE_SIZE - 4);

        if (PAGE_SIZE< total + freep + need) {
            return ERROR; 
        }
    }

    if (record_create((Record *)((uint8_t *)page + freep), field_count, field_data, field_length,total) != SUCCESS) {
        return ERROR;
    }

    size_t idbyte = reccount * 2;
    size_t firstb = PAGE_SIZE - 6;
    size_t off = firstb - idbyte;
    *( uint16_t *)(pageby + off) = freep;

    *( uint16_t*)(pageby + PAGE_SIZE - 4) = freep + total;
    *( uint16_t *)(pageby + PAGE_SIZE - 2) = reccount + 1;
    

    *index = reccount;  

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
  if(page==NULL){
    return ERROR;
  }
  if(field_count>0){
    if(field_data==NULL){
      return ERROR;
    }
    if(field_length==NULL){
    return ERROR;
    }
  }
  uint8_t *pageby = (uint8_t *)page;
   uint16_t reccount = *( uint16_t *)(pageby + PAGE_SIZE - 2);
    if (index >= reccount) {
        return ERROR;
    }
  size_t idbyte = index * 2;
  //firstbyte
  size_t firstb = PAGE_SIZE - 6;
  size_t off = firstb - idbyte;

  uint16_t lastpage = *(uint16_t*)(pageby + off);
  if(lastpage == 0 ){
    return ERROR; //Page already gone
  }
  *( uint16_t*)(pageby + off) = 0;
  size_t i = 0;
  if (record_page_put(page, field_count, field_data, field_length, &i) != SUCCESS) {
    return ERROR;
}
    size_t updateoff = PAGE_SIZE - 6 - (i * 2);
     uint16_t updatepa = *( uint16_t *)(pageby + updateoff);

    *( uint16_t *)(pageby + off) = updatepa;


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
   if (page == NULL) {
    return ERROR;
  }
  uint8_t *pageby = (uint8_t *)page;
   uint16_t countofrec = 0;
  countofrec = *( uint16_t *)(pageby + PAGE_SIZE - 2);

  if(index>=countofrec){
    return ERROR;
  }

  //index times bytes
  size_t idbyte = index * 2;
  //firstbyte
  size_t firstb = PAGE_SIZE - 6;
  size_t off = firstb - idbyte;
  
  *( uint16_t *)(pageby + off) = 0;

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
  if(page == NULL){
    return ERROR;
  }
  if(record==NULL){
    return ERROR;
  }
  uint8_t *pageby = (uint8_t *)page;
   uint16_t countofrec = 0;
  countofrec = *( uint16_t *)(pageby + PAGE_SIZE - 2);
  
  if(index>=countofrec){
    return ERROR;
  }

  //index times bytes
  size_t idbyte = index * 2;
  //firstbyte
  size_t firstb = PAGE_SIZE - 6;
  size_t off = firstb - idbyte;

   uint16_t Offpage = *( uint16_t *)(pageby + off);
  
  //checkifpageexist
  if(Offpage==0){
    return ERROR;
  }
  *record = (Record *)(pageby + Offpage);

  return SUCCESS;
}

/// @brief Defragment the records on the provided page.
/// @param[in] page        The page to defragment.
///
/// Rearrange the layout of the records on the provided page to free up space
/// from e.g., deleted records.
Result record_page_defragment(Frame *page) { 
  if(page==NULL){
    return ERROR;
  }
  uint8_t *pageby = (uint8_t *)page;
   uint16_t countofrec = 0;
  countofrec = *( uint16_t *)(pageby + PAGE_SIZE - 2);
   uint16_t freepoint = 0;
   for (size_t i = 0; i < countofrec; i++) {
        size_t idbyte = i * 2;
  //firstbyte
        size_t firstb = PAGE_SIZE - 6;
        size_t off = firstb - idbyte;

        uint16_t Offpage = *(uint16_t *)(pageby + off);
        // deleted records
        if (Offpage == 0) {
            continue;
        }

        Record *currec = (Record *)(pageby + Offpage);
        size_t rlen = record_length(currec);  

        if (Offpage != freepoint) {
            memmove(pageby + freepoint, currec, rlen);
        }


        *(uint16_t *)(pageby + off) = freepoint;

        
        freepoint += rlen;
    }
    *(uint16_t *)(pageby + PAGE_SIZE -4) = freepoint;
  return SUCCESS; }
