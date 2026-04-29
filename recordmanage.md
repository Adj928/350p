## Project 2: Record management

In this project, you will implement logic to manage records on pages of data

- **Handout**: https://git.odin.cse.buffalo.edu/CSE-350/2025-fa-project/src/branch/project_2/PROJECT_2.md
- **Submission**: https://autolab.cse.buffalo.edu/courses/cse350-f25/assessments/Record-Layout

This project is scored out of 15 points and is worth 15% of the overall project component of your course grade.

#### Updating Your Repository

Assuming you have configured your ubstream with `project` as your upstream remote, as in PA0, then the following will update your repository:
```bash
git fetch project
git merge project/project_2
```

#### Learning Goals
- Understand how record layouts (fields in records) work
- Understand how page layouts (records in fields) work
- Learn to work with abstract structures

#### Resources
- The `man` pages (e.g., `man fread`) on the command line are very useful.  Useful functions for this assignment include:
    - `memcpy`: Copy data at one address to another

#### Objectives

You must implement the following functions in `src/record.c`.  Details and instructions for each function are provided in the function's documentation comment.

The following functions manage fields in records:
```c
size_t record_length(Record *record);
Result record_create(Record *record, size_t field_count, Field field_data[], size_t field_length[], size_t max_bytes);
Result record_field_get(Record *record, size_t field_index, void **value, size_t *length);
```

The following functions manage records in pages.
```c
Result record_page_init(Frame *page);
size_t record_page_count(Frame *page);
Result record_page_put(Frame *page, size_t field_count, Field field_data[], size_t field_length[], size_t *index);
Result record_page_update(Frame *page, size_t field_count, Field field_data[], size_t field_length[], size_t index);
Result record_page_delete(Frame *page, size_t index);
Result record_page_get(Frame *page, size_t index, Record **record);
Result record_page_defragment(Frame *page);
```

#### Testing

The REPL now provides the following additional commands

* `I [register]`: Initialize the page pinned to the frame identified by the provided register for use with records
* `m [register] [field_count] [field_length] [field data] [field_length] [field_data] ...`: Make a record in the page pinned to the frame identified by the provided register.  The record has `field_length` fields.  Each field is provided as a number (of bytes in the field) followed by a space, followed by the data values.  The record ID of the resulting record will be printed.
* `t [register] [record id] [field_count] [field_length] [field_data] ...`: Update a record in the page pinned to the frame identified by the provided register.  The record ID is the same value returned when the record was first made.  Fields are provided as in `m`.
* `g [register] [record_id]`: Retrieve the record identified by the provided identifier, from the page pinned to the frame identified by the provided register.  The record ID is the same value returned when the record was first made. 

#### Submission and Late Policy

Upload `src/record.c` to Autolab.  You may submit as many times as you like before the deadline.  You may submit for up to 48 hours after the deadline, but each partial day over the deadline consumes a grace day if you have any remaining, or reduces your grade by 25% of the maximum (3.75 pt) if you do not.  

