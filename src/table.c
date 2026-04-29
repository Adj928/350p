/**
 * @file table.c
 * @author Adonis Jackson
 */

#include "buffer.h"
#include "conventions.h"
#include "table.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct _Table {
  // put anything you need here
  BufferManager *manager;
  PageID root;
  PageID firstdata;
  PageID lastdata;
  size_t numofp;
  size_t numofrow;
  size_t reserve;
  TableConfig *config;
  size_t field_count;
} _Table;

typedef struct _TableIterator {
  // put anything you need here
  _Table *table;
  PageID curpage;
  size_t curreci;//current index of record
   Frame *frame;
} _TableIterator;

/// @brief Initialize a database in the file managed by the provided buffer
/// manager
/// @param[in] manager         The buffer manager for the file to initialize the
///                            database in
/// @param[in] reserved_pages  The number of leading pages in the database to
///                            reserve
/// @return                    SUCCESS if the database was successfully
///                            initialized
///
/// This function should set up everything that `database_page_allocate` and
/// `database_page_release` need to work.  You should use the page
/// `DATABASE_ROOT_PAGE` to store any information you need.  If the database
/// is closed and subsequently re-opened, `database_init` will not be called
/// again.
///
/// The first `reserved_pages` should be reserved for internal use by the
/// caller.  This value is guaranteed to always be at least
/// `DATABASE_ROOT_PAGE+1`.
///
/// This function should have a constant IO and runtime complexity
#define NEXT_PAGE_OFFSET (PAGE_SIZE - sizeof(PageID))

Result database_init(BufferManager *manager, PageID reserved_pages) {
   Frame *frame;

   if (buffer_pin(manager, DATABASE_ROOT_PAGE, &frame) != SUCCESS){
        return ERROR;
    }
   memset((char*)frame, 0, PAGE_SIZE);
   
   PageID *reserve = (PageID *)(char *)frame;
   PageID *next = (PageID *)((char *)frame + sizeof(PageID));
   PageID *freep = (PageID *)((char *)frame + (2*sizeof(PageID)));

   *reserve = reserved_pages;
   *next = reserved_pages; 
   *freep = 0;            

   buffer_mark(manager, frame);
   buffer_unpin(manager, frame);

   return SUCCESS;
}

/// @brief Allocate a new page for use by the database
/// @param[in] manager       The manager for the database file
/// @param[out] page         The address to write the allocated page id to
/// @return                  SUCCESS if a page was successfully allocated
///
/// Allocate must re-use previously released pages if any have not already
/// been re-allocated.
///
/// This function should have a constant IO and runtime complexity
Result database_page_allocate(BufferManager *manager, PageID *page) {
   Frame *rootframe;

   if (buffer_pin(manager, DATABASE_ROOT_PAGE, &rootframe) != SUCCESS){
        return ERROR;
    }

   PageID *reserve = (PageID *)(char *)rootframe;
   PageID *next = (PageID *)((char *)rootframe + sizeof(PageID));
   PageID *freep = (PageID *)((char *)rootframe + (2*sizeof(PageID)));

  if (*freep != 0) {
     Frame *freeframe;
     *page = *freep;
    
     if (buffer_pin(manager, *page, &freeframe) != SUCCESS) {
         buffer_unpin(manager, rootframe);
         return ERROR;
     }
     *freep = *((PageID*)freeframe);

     buffer_mark(manager, freeframe);
     buffer_unpin(manager, freeframe);
  }
  else{
    *page = *next;
    *next = *next + 1;
  }

  buffer_mark(manager, rootframe);
  buffer_unpin(manager,rootframe);
  return SUCCESS;
}

/// @brief Release a page back to the database
/// @param[in] manager       The manager for the database file
/// @param[in] page          The page to release
/// @return                  SUCCESS if the page was successfully released
///
/// This function should have a constant IO and runtime complexity
Result database_page_release(BufferManager *manager, PageID page) {
  Frame *rootframe;
  if (buffer_pin(manager, DATABASE_ROOT_PAGE, &rootframe) != SUCCESS){
    return ERROR;
  }
  PageID *freep = (PageID *)((char *)rootframe+(2*sizeof(PageID)));
  Frame *pageframe;
  if (buffer_pin(manager,page,&pageframe) != SUCCESS) {
    buffer_unpin(manager,rootframe);
    return ERROR;
  }
  *((PageID *)pageframe) = *freep;
  *freep = page;

  buffer_mark(manager, pageframe);
  buffer_unpin(manager,pageframe);
  buffer_mark(manager, rootframe);
  buffer_unpin(manager,rootframe);
  return SUCCESS;

}

/// @brief Initialize a table at the specified page.
/// @param[in] manager        The buffer manager managing the database
/// @param[in] header_page    The page in which to store header data for the
///                           table
/// @param[in] config         Descriptive information for the table
/// @param[out] table         The address to write a table handle to
/// @return                   SUCCESS if the table was successfully initialized
///                           and opened
///
/// You can retain a reasonable amount of persistent resources (e.g., pinned
/// pages) while the table is open.  Store relevant information in the provided
/// header page.
///
/// This function should have a constant IO and runtime complexity
Result table_init(BufferManager *manager, PageID header_page,
                  TableConfig *config, Table *table) {
  *table = malloc(sizeof(_Table));
  _Table *_table = (_Table *)*table;

  // Initialize both _table and header_page
  _table->manager = manager;
  _table->root = header_page;
  _table->config = config;
  if (config != NULL){
    _table->field_count = config->field_count;
  }
  else{
    _table->field_count = 0;
  }
  _table->numofrow = 0;
  _table->numofp  = 1;

  if (database_page_allocate(manager, &_table->firstdata) != SUCCESS){
    return ERROR;
  }
  _table->lastdata = _table->firstdata;

  Frame *dataframe;
  if (buffer_pin(manager, _table->firstdata, &dataframe) == SUCCESS) {
    record_page_init(dataframe);
      
    buffer_mark(manager, dataframe);
    buffer_unpin(manager, dataframe);
  }

  Frame *frame;

  if (buffer_pin(manager,header_page, &frame) != SUCCESS){
    
    return ERROR;
  }
   
  memset(frame, 0, PAGE_SIZE);
  PageID *pag = (PageID *)frame;
  pag[0] = _table->firstdata;
  pag[1] = _table->lastdata;    
  size_t *meta = (size_t *)(pag + 2);
  meta[0] = _table->numofrow;
  meta[1] = _table->numofp;
  meta[2] = _table->field_count;
  

  buffer_mark(manager, frame);
  buffer_unpin(manager, frame);
  
  return SUCCESS;
}

/// @brief Open a table previously initialized at the specified page
/// @param[in] manager        The buffer manager managing the database
/// @param[in] header_page    The page in which the table was previously
///                           initialized
/// @param[out] table         The address to write a table handle to
/// @return                   SUCCESS if the table was successfully opened
///
/// You can retain a reasonable amount of persistent resources (e.g., pinned
/// pages) while the table is open.  Store relevant information in the provided
/// header page.
///
/// This function should have a constant IO and runtime complexity
Result table_open(BufferManager *manager, PageID header_page, Table *table) {
  *table = malloc(sizeof(_Table));
  _Table *_table = (_Table *)*table;

  // Initialize _table based on header_page
  _table->manager = manager;
  _table->root = header_page;

  Frame *frame;

  if (buffer_pin(manager,header_page, &frame) != SUCCESS){
    free(_table);
    return ERROR;
  }

  PageID *pag = (PageID *)frame;
  _table->firstdata = pag[0];
  _table->lastdata  = pag[1];

  size_t *meta = (size_t *)(pag + 2);
  _table->numofrow = meta[0];
  _table->numofp   = meta[1];
  _table->field_count = meta[2];
  _table->manager = manager;
  _table->root = header_page;

  buffer_unpin(manager, frame);
  return SUCCESS;
  
}

/// @brief Return the number of fields in an open table
/// @param[in]                The handle of the table
/// @param[out]               The number of fields in the table
/// @return                   SUCCESS if the number of fields was successfully
///                           retrieved
Result table_field_count(Table table, size_t *count) {
  if(table==NULL || count ==NULL){
      return ERROR;
    }

  _Table *_table = (_Table *)table;
  if (_table->field_count == 0){
    return ERROR;
  }
  *count = _table->field_count;
  return SUCCESS;

}

/// @brief Close a previously opened table
/// @param[in]                The handle of the previously opened table
/// @return                   SUCCESS if the table was successfully opened
///
/// This should release *all* resources allocated during table_init and/or
/// table_open
///
/// This function should have a constant IO and runtime complexity
Result table_close(Table table) {
  if (table == NULL) {
        return ERROR;
    }

  _Table *_table = (_Table *)table;

  Frame *rootframe;
  if (buffer_pin(_table->manager, _table->root, &rootframe) == SUCCESS) {
      PageID *pag = (PageID *)rootframe;
      pag[0] = _table->firstdata;
      pag[1] = _table->lastdata;
      size_t *meta = (size_t *)(pag + 2);
      meta[0] = _table->numofrow;
      meta[1] = _table->numofp;
      meta[2] = _table->field_count;
      buffer_mark(_table->manager, rootframe);
      buffer_unpin(_table->manager, rootframe);
  }
  
  // Clean up resources allocated in _table

  free(table);
  return SUCCESS;
}

/// @brief Append a new record to the provided table
/// @param[in] table         The handle of the table to append to
/// @param[in] field_data    Data for the fields of the record to append
/// @param[in] field_length  Lengths of the fields to be appended
/// @return                  SUCCESS if the new record was successfully appended
///
/// The field length should be taken from the TableConfig passed when the table
/// was initialized
///
/// This function should have a constant IO and runtime complexity, although it
/// is permitted to create additional work for later function calls (e.g.,
/// buffer_flush_unpinned).
Result table_append(Table table, Field field_data[], size_t field_length[]) {
  _Table *_table = (_Table *)table;

    Frame *frame;
    if (buffer_pin(_table->manager, _table->lastdata, &frame) != SUCCESS)
        return ERROR;

    size_t index;
    Result result = record_page_put(frame, _table->field_count, field_data, field_length, &index);
    if (result == SUCCESS) {
    
        _table->numofrow++;

    
        buffer_mark(_table->manager, frame);
        buffer_unpin(_table->manager, frame);

    
        Frame *hdr;
        if (buffer_pin(_table->manager, _table->root, &hdr) != SUCCESS)
            return ERROR;
        PageID *pag = (PageID *)hdr;
        pag[0] = _table->firstdata;
        pag[1] = _table->lastdata;
        size_t *meta = (size_t *)(pag + 2);
        meta[0] = _table->numofrow;
        meta[1] = _table->numofp;
        meta[2] = _table->field_count;
        buffer_mark(_table->manager, hdr);
        buffer_unpin(_table->manager, hdr);
        buffer_flush_unpinned(_table->manager);

        return SUCCESS;
    }

    
    buffer_unpin(_table->manager, frame);

    PageID newpage;
    if (database_page_allocate(_table->manager, &newpage) != SUCCESS)
        return ERROR;

    Frame *newframe;
    if (buffer_pin(_table->manager, newpage, &newframe) != SUCCESS)
        return ERROR;

    
    record_page_init(newframe);

    
    {
        Frame *lastframe;
        if (buffer_pin(_table->manager, _table->lastdata, &lastframe) == SUCCESS) {
            char *pdata = (char *)lastframe;
            PageID *next_ptr = (PageID *)(pdata + (PAGE_SIZE - sizeof(PageID)));
            *next_ptr = newpage;
            buffer_mark(_table->manager, lastframe);
            buffer_unpin(_table->manager, lastframe);
        } else {
            buffer_unpin(_table->manager, newframe);
            return ERROR;
        }
    }
    
    if (record_page_put(newframe, _table->field_count, field_data, field_length, &index) != SUCCESS) {
        buffer_unpin(_table->manager, newframe);
        return ERROR;
    }

    
    _table->numofrow++;
    _table->numofp++;
    _table->lastdata = newpage;

    
    buffer_mark(_table->manager, newframe);
    buffer_unpin(_table->manager, newframe);

    
    Frame *hdr;
    if (buffer_pin(_table->manager, _table->root, &hdr) != SUCCESS)
        return ERROR;
    PageID *pag = (PageID *)hdr;
    pag[0] = _table->firstdata;
    pag[1] = _table->lastdata;
    size_t *meta = (size_t *)(pag + 2);
    meta[0] = _table->numofrow;
    meta[1] = _table->numofp;
    meta[2] = _table->field_count;
    buffer_mark(_table->manager, hdr);
    buffer_unpin(_table->manager, hdr);
    buffer_flush_unpinned(_table->manager);

    return SUCCESS;
}

/// @brief Initialize an iterator over the provided table
/// @param[in] table         The handle of the table to append to
/// @param[out] iterator     The address to write the iterator handle to
/// @return                  SUCCESS if the iterator was successfully created
///
/// You can retain a reasonable amount of persistent resources (e.g., pinned
/// pages) while the iterator is open.
///
/// This function should have a constant IO and runtime complexity.
Result table_iterate(Table table, TableIterator *iterator) {
  *iterator = malloc(sizeof(_TableIterator));
  _TableIterator *_iterator = *iterator;
  _Table *_table = table;
  // Initialize _iterator
  _iterator->table = _table;
  _iterator->curpage = _table->firstdata;
  if (buffer_pin(_table->manager,_iterator->curpage,&_iterator->frame) != SUCCESS){
    free(_iterator);
    return ERROR;
  }
  _iterator->curreci = 0;
  return SUCCESS;
}

/// @brief Retrieve the next record (if one exists) from the provide iterator
/// @param[in] iterator     The iterator to advance
/// @param[out] record      Where to write the address of the next record, or
///                         NULL if the iterator has reached the end
/// @return                 SUCCESS if no errors occurred while advancing the
///                         iterator.  Note that reaching the end should not be
///                         considered an error (set record to NULL in that
///                         case).
///
/// This function should have a constant IO and runtime complexity.
Result table_iterator_next(TableIterator iterator, Record **record) {
   _TableIterator *_iterator = iterator;
   while (1) {
        Frame *frame = _iterator->frame;
        size_t count = record_page_count(frame);

        
        if (_iterator->curreci < count) {
            if (record_page_get(frame, _iterator->curreci, record) != SUCCESS)
                return ERROR;
            _iterator->curreci++;
            return SUCCESS;
        }

        
        if (_iterator->curpage == _iterator->table->lastdata) {
            *record = NULL;
            return SUCCESS;
        }
        
        char *pdata = (char *)frame;
    PageID *next_ptr = (PageID *)(pdata + (PAGE_SIZE - sizeof(PageID)));
    PageID nextpage = *next_ptr;

    if (nextpage == 0) {
            *record = NULL;  
            return SUCCESS;
        }
    
        Frame *nextframe;
        if (buffer_pin(_iterator->table->manager, nextpage, &nextframe) != SUCCESS)
            return ERROR;

     
        buffer_unpin(_iterator->table->manager, frame);
        _iterator->frame = nextframe;
        _iterator->curpage = nextpage;
        _iterator->curreci = 0;

     
    }
   *record = NULL;
   return SUCCESS;
}
/// @brief Close the provided iterator, releasing any relevant resources
/// @param[in] iterator     The iterator to close
/// This function should have a constant IO and runtime complexity.
Result table_iterator_close(TableIterator iterator) {
  _TableIterator *_iterator = iterator;
  if(buffer_unpin(_iterator->table->manager, _iterator->frame)!=SUCCESS){
    return ERROR;
  }
  free(_iterator);
  // Clean up any resources reserved in _iterator
  return SUCCESS;
}
