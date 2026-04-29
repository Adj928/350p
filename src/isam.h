/**
 * @file isam.h
 * @author Oliver Kennedy
*/
#ifndef ISAM_H_SHIELD
#define ISAM_H_SHIELD

#include "conventions.h"
#include "table.h"

Result isam_init(Table table, size_t key_index, DataType key_type);
Result isam_find_page(Table table, size_t key_length, void *key, PageID *page_id);
Result isam_find_record(Table table, size_t key_length, void *key, Frame **page,
                        Record **record, PageID *page_id, size_t *record_id);

#endif
