/**
 * @file isam.c
 * @author Adonis Jackson
 */

#include "conventions.h"
#include "record.h"
#include "utilities.h"
#include "table.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INVALID_PAGE 0

typedef struct _Table {
    Frame *frame;
    BufferManager *manager;
} _Table;

typedef struct _TableHeaderEntry {
    char magic[8];
    TableConfig config;
    size_t first_page;
    size_t last_page;
    int isamexist;
    PageID isam_rootpage;
    size_t isam_keyindex;
    DataType isam_keytype;
    size_t isam_depth;
} _TableHeaderEntry;

static Result dir_init_page(_Table *_table, PageID *pageid_out, Frame **frame_out) {
    PageID pageid;
    if (database_page_allocate(_table->manager, &pageid) != SUCCESS) {
        return ERROR;
    }
    Frame *frame;
    if (buffer_pin(_table->manager, pageid, &frame) != SUCCESS) {
        return ERROR;
    }
    if (record_page_init(frame) != SUCCESS) {
        buffer_unpin(_table->manager, frame);
        return ERROR;
    }
    PageID next_dir = INVALID_PAGE;
    Field f[1];
    size_t flen[1];
    f[0]  = &next_dir;
    flen[0] = sizeof(PageID);
    size_t out_idx;
    if (record_page_put(frame, 1, f, flen, &out_idx) != SUCCESS) {
        buffer_unpin(_table->manager, frame);
        return ERROR;
    }

    *pageid_out = pageid;
    *frame_out = frame;
    return SUCCESS;
}

static Result dir_append_entry(_Table *_table,PageID *dir_tail_id,Frame **dir_tail_frame,void *key,size_t keylen,PageID child_page) {
    Field fields[2];
    size_t fieldlen[2];
    fields[0]   = key;
    fieldlen[0] = keylen;
    fields[1]   = &child_page;
    fieldlen[1] = sizeof(PageID);

    size_t outindex;
    if (record_page_put(*dir_tail_frame, 2, fields, fieldlen, &outindex) == SUCCESS) {
        if (buffer_mark(_table->manager, *dir_tail_frame) != SUCCESS) {
            return ERROR;
        }
        return SUCCESS;
    }

    PageID newdir_id;
    Frame *newdir_frame;
    if (dir_init_page(_table, &newdir_id, &newdir_frame) != SUCCESS) {
        return ERROR;
    }
    Record *next_rec;
    if (record_page_get(*dir_tail_frame, 0, &next_rec) != SUCCESS) {
        buffer_unpin(_table->manager, newdir_frame);
        return ERROR;
    }

    void *next_ptr = NULL;
    size_t next_len = 0;
    if (record_field_get(next_rec, 0, &next_ptr, &next_len) != SUCCESS) {
        buffer_unpin(_table->manager, newdir_frame);
        return ERROR;
    }
    if (next_ptr == NULL || next_len < sizeof(PageID)) {
        buffer_unpin(_table->manager, newdir_frame);
        return ERROR;
    }
    *(PageID *)next_ptr = newdir_id;
    if (buffer_mark(_table->manager, *dir_tail_frame) != SUCCESS) {
        buffer_unpin(_table->manager, newdir_frame);
        return ERROR;
    }

    if (buffer_unpin(_table->manager, *dir_tail_frame) != SUCCESS) {
        buffer_unpin(_table->manager, newdir_frame);
        return ERROR;
    }
    *dir_tail_id = newdir_id;
    *dir_tail_frame = newdir_frame;
    if (record_page_put(*dir_tail_frame, 2, fields, fieldlen, &outindex) != SUCCESS) {
        buffer_unpin(_table->manager, *dir_tail_frame);
        return ERROR;
    }

    return SUCCESS;
}

/// @brief Initialize an ISAM index over the provided table
/// @param[in] table      A reference to the already-opened table
/// @param[in] key_index  The index of the attribute of each record that
///                       contains the key
/// @param[in] key_type   The data type of the key field
/// @return               ERROR if the data in the provided table is
///                       out-of-order, or if another error occurs
///
/// This function should initialize an ISAM index over the provided table.  The
/// existing table pages should form the data pages of the index; this function
/// should only create directory pages.
///
/// You should implement this function under the assumption that the records in
/// the table are appended in ascending order by key, and that each record has a
/// unique key (i.e., for every successive pair of records (a, b) returned by
/// the iterator, `compare_values(a, b)` returns a negative value).   However
/// you must verify this constraint.  If the table violates it, this function
/// must return ERROR.
///
/// To reiterate, records will be provided according to the comparator
/// `compare_values`, provided in `utilities.h`.  You should use this comparator
/// rather than implementing your own.
///
/// Any state associated with the ISAM index should be persisted to disk (i.e.,
/// the existence of the index, as well as any additional information like the
/// key index, and the key type), so that it is recovered after a restart.  Note
/// that this will likely require changes to table.c
///
/// You may also assume that once an ISAM index is created for a table, that no
/// new records will ever be appended to it again.
Result isam_init(Table table, size_t key_index, DataType key_type) {
    if (table == NULL) {
        return ERROR;
    }

    _Table *_table = table;
    _TableHeaderEntry *header = (_TableHeaderEntry *)_table->frame;

    if (header->isamexist) {
        return SUCCESS;
    }

    header->isam_keytype  = key_type;
    header->isam_keyindex = key_index;
    header->isamexist     = 0;
    header->isam_rootpage = INVALID_PAGE;
    header->isam_depth    = 0;

    TableIterator it;
    Result res = table_iterate(table, &it);
    if (res != SUCCESS) {
        return ERROR;
    }

    Record *rec = NULL;
    void *prevkey = NULL;
    size_t prevkey_len = 0;
    int prevexist = 0;

    while (1) {
        if (table_iterator_next(it, &rec) != SUCCESS) {
            table_iterator_close(it);
            free(prevkey);
            return ERROR;
        }
        if (rec == NULL) {
            break;
        }

        void *keyptr = NULL;
        size_t keylen = 0;

        if (record_field_get(rec, key_index, &keyptr, &keylen) != SUCCESS) {
            table_iterator_close(it);
            free(prevkey);
            return ERROR;
        }

        if (!prevexist) {
            prevkey = malloc(keylen);
            if (prevkey == NULL) {
                table_iterator_close(it);
                return ERROR;
            }
            memcpy(prevkey, keyptr, keylen);
            prevexist = 1;
            prevkey_len  = keylen;
        }
    else {
            int cmp = 0;
            if (compare_values(prevkey_len, prevkey,keylen, keyptr,key_type, &cmp) != SUCCESS) {
                table_iterator_close(it);
                free(prevkey);
                return ERROR;
            }
            if (cmp >= 0) {
                table_iterator_close(it);
                free(prevkey);
                return ERROR;
            }
            if (keylen != prevkey_len) {
                void *nbuf = realloc(prevkey, keylen);
                if (nbuf == NULL) {
                    table_iterator_close(it);
                    free(prevkey);
                    return ERROR;
                }
                prevkey = nbuf;
                prevkey_len = keylen;
            }
            memcpy(prevkey, keyptr, keylen);
        }
    }
    table_iterator_close(it);
    free(prevkey);
    if (header->first_page == INVALID_PAGE) {
        header->isam_rootpage = INVALID_PAGE;
        header->isam_depth = 0;
    }
    else {
        header->isam_rootpage = header->first_page;
        header->isam_depth = 1;
    }
    header->isamexist = 1;
    if (buffer_mark(_table->manager, _table->frame) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

/// @brief Use an already initialized ISAM index to find the page on which a
///        record is located.
/// @param[in]  table       The table to search in
/// @param[in]  key_length  The length of the key to search for
/// @param[in]  key         A pointer to the actual data of the key to search
///                         for
/// @param[out] page        A pointer to a memory address to which to write the
///                         page on which key (if it exists) resides.
/// @return                 ERROR if the table does not have an initialized
///                         ISAM index or if another error occurs.
///
/// This function should traverse the ISAM index to identify which data page a
/// specific key resides on (if it exists).  Note that the function does not
/// actually need to visit or pin the page, merely return a reference to it.
/// This function should never return an error if the key does not actually
/// exist.
///
/// To reiterate, records will should be compared according to the comparator
/// `compare_values`, provided in `utilities.h`.  You should use this comparator
/// rather than implementing your own.
Result isam_find_page(Table table, size_t key_length, void *key,
                      PageID *page_id) {
    if (table == NULL) {
        return ERROR;
    }
    if (page_id == NULL) {
        return ERROR;
    }

    _Table *_table = table;
    _TableHeaderEntry *header = (_TableHeaderEntry *)_table->frame;

    if (!header->isamexist) {
        return ERROR;
    }
    *page_id = INVALID_PAGE;
    if (header->first_page == INVALID_PAGE) {
        return SUCCESS;
    }

    PageID current = header->first_page;

    while (current != INVALID_PAGE) {
        Frame *frame;
        if (buffer_pin(_table->manager, current, &frame) != SUCCESS) {
            return ERROR;
        }

        size_t count = record_page_count(frame);

        for (size_t i = 1; i < count; i++) {
            Record *rec;
            if (record_page_get(frame, i, &rec) != SUCCESS) {
                continue;
            }

            void *entrykey = NULL;
            size_t entrykey_len = 0;
            if (record_field_get(rec, header->isam_keyindex,&entrykey, &entrykey_len) != SUCCESS) {
                buffer_unpin(_table->manager, frame);
                return ERROR;
            }

            int cmp = 0;
            if (compare_values(key_length, key,entrykey_len, entrykey,header->isam_keytype, &cmp) != SUCCESS) {
                buffer_unpin(_table->manager, frame);
                return ERROR;
            }

            if (cmp == 0) {
                *page_id = current;
                buffer_unpin(_table->manager, frame);
                return SUCCESS;
            }
        }

        Record *nextptr_rec;
        if (record_page_get(frame, 0, &nextptr_rec) != SUCCESS) {
            buffer_unpin(_table->manager, frame);
            return ERROR;
        }

        void *nextptr = NULL;
        size_t nextlen = 0;
        if (record_field_get(nextptr_rec, 0, &nextptr, &nextlen) != SUCCESS) {
            buffer_unpin(_table->manager, frame);
            return ERROR;
        }

        PageID nextpage = INVALID_PAGE;
        if (nextptr != NULL && nextlen >= sizeof(PageID)) {
            nextpage = *(PageID *)nextptr;
        }

        if (buffer_unpin(_table->manager, frame) != SUCCESS) {
            return ERROR;
        }
        current = nextpage;
    }

    return SUCCESS;
}

/// @brief Use an already initialized ISAM index to locate a specific record,
///        pin the record's page and return a pointer to the index itself.
/// @param[in]  table       The table to search in
/// @param[in]  key_length  The length of the key to search for
/// @param[in]  key         A pointer to the actual data of the key to search
///                         for
/// @param[out] page        A pointer to which to write the pinned frame to.
/// @param[out] record      A pointer to which to write the address of the
///                         record within `page`, or NULL if the record does not
///                         exist.
/// @param[out] page_id     A pointer to which to write the index of `page`
/// @param[out] record_id   A pointer to which to write the index of the record
///                         in `page`
/// @return                 ERROR if the table does not have an initialized
///                         ISAM index or if another error occurs.
///
/// This function should locate the page on which the provided key should
/// reside, and pin and return the page.  If the record exists on the page, it
/// should be located and returned.  The caller is responsible for freeing the
/// page.
///
/// To reiterate, records will should be compared according to the comparator
/// `compare_values`, provided in `utilities.h`.  You should use this comparator
/// rather than implementing your own.
Result isam_find_record(Table table, size_t key_length, void *key,
                        Frame **page, Record **record,
                        PageID *page_id, size_t *record_id) {
    if (table == NULL) {
        return ERROR;
    }
    if (page == NULL) {
        return ERROR;
    }
    if (record == NULL) {
        return ERROR;
    }
    if (page_id == NULL) {
        return ERROR;
    }
    if (record_id == NULL) {
        return ERROR;
    }

    _Table *_table = table;
    _TableHeaderEntry *header = (_TableHeaderEntry *)_table->frame;

    if (!header->isamexist) {
        return ERROR;
    }

    if (isam_find_page(table, key_length, key, page_id) != SUCCESS) {
        return ERROR;
    }

    if (*page_id == INVALID_PAGE) {
        *page = NULL;
        *record = NULL;
        *record_id = 0;
        return SUCCESS;
    }

    if (buffer_pin(_table->manager, *page_id, page) != SUCCESS) {
        return ERROR;
    }

    *record_id = 0;
    *record = NULL;

    size_t count = record_page_count(*page);
    size_t expected_fields = header->config.field_count;

    for (size_t i = 1; i < count; i++) {
        Record *rec;

        if (record_page_get(*page, i, &rec) != SUCCESS) {
            continue;
        }

        unsigned short *rhdr = (unsigned short *)rec;
        size_t fieldc = rhdr[0];
        if (fieldc != expected_fields) {
            continue;
        }

        void *entrykey = NULL;
        size_t entrykey_len = 0;
        if (record_field_get(rec, header->isam_keyindex,&entrykey, &entrykey_len) != SUCCESS) {
            buffer_unpin(_table->manager, *page);
            return ERROR;
        }

        int cmp = 0;
        if (compare_values(key_length, key,entrykey_len, entrykey,header->isam_keytype, &cmp) != SUCCESS) {
            buffer_unpin(_table->manager, *page);
            return ERROR;
        }

        if (cmp == 0) {
            *record = rec;
            *record_id = i;
            return SUCCESS;
        }
    }

    return SUCCESS;
}
