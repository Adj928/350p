/**
 * @file lsm.h
 * @author Oliver Kennedy
*/
#ifndef LSM_H_SHIELD
#define LSM_H_SHIELD

#include "buffer.h"
#include "conventions.h"
#include "record.h"

typedef void *LSMTree;

Result lsm_init(BufferManager *manager, PageID header_page, TableConfig *config,
                LSMTree *lsm);
Result lsm_open(BufferManager *manager, PageID header_page, LSMTree *lsm);
Result lsm_insert(LSMTree lsm, Field field_data[], size_t field_length[]);
Result lsm_find_record(LSMTree lsm, size_t key_length, void *key,
                       Frame **page, Record **record);
Result lsm_close(LSMTree lsm);
Result lsm_field_count(LSMTree lsm, size_t *field_count);

#endif
