/**
 * @file table.h
 * @author Oliver Kennedy
 */
#ifndef TABLE_H_SHIELD
#define TABLE_H_SHIELD

#include "buffer.h"
#include "conventions.h"
#include "record.h"
#include <sys/types.h>

#define DATABASE_ROOT_PAGE 0

typedef void *Table;
typedef void *TableIterator;

Result database_init(BufferManager *manager, PageID reserved_pages);
Result database_page_allocate(BufferManager *manager, PageID *page);
Result database_page_release(BufferManager *manager, PageID page);

Result table_init(BufferManager *manager, PageID header_page, TableConfig *config, Table *table);
Result table_open(BufferManager *manager, PageID header_page, Table *table);
Result table_field_count(Table table, size_t *count);
Result table_close(Table table);
Result table_append(Table table, Field field_data[], size_t field_length[]);

Result table_iterate(Table table, TableIterator *iterator);
Result table_iterator_next(TableIterator iterator, Record **record);
Result table_iterator_close(TableIterator iterator);

#endif
