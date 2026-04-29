## Project 4: ISAM Indexing
**Due**: Nov 24

In this project, you will extend your existing 

- **Handout**: https://git.odin.cse.buffalo.edu/CSE-350/2025-fa-project/src/branch/project_4/PROJECT_4.md
- **Submission**: https://autolab.cse.buffalo.edu/courses/cse350-f25/assessments/ISAM-Index

This project is scored out of 10 points and is worth 10% of your overall grade.

#### Updating Your Repository

Assuming you have configured your upstream with `project` as your upstream remote, as in PA0, then the following will update your repository:
```bash
git fetch project
git merge project/project_4
```

#### Learning Goals
- Create and access a non-trivial, multi-page, on-disk data structure
- Implement a streaming algorithm
- Demonstrate familiarity with tree-based indexing
- Demonstrate familiarity with the single-pass tree construction algorithm

#### Objectives

You must implement the following functions in `src/isam.c`.  Details and instructions for each function are provided in the function's documentation comment.
```c
Result isam_init(Table table, size_t key_index, DataType key_type);
Result isam_find_page(Table table, size_t key_length, void *key, PageID *page_id);
Result isam_find_record(Table table, size_t key_length, void *key, Frame **page,
                        Record *record, PageID *page_id, size_t *record_id);
```

#### Guidelines
In this project you will implement a clustered ISAM index over an existing table.  You can assume (but should verify) that the provided table is already sorted on the key attribute.  As the data pages for this table already exist, your goal is mainly to create the index itself.  You may also assume that the table will receive no further writes.

The index should consist of multiple layers of directory pages that store page indexes separated by key literals.  For example, a data table with keys consecutively numbered from 1, with 20 records per page, might look something like this:
```
Dir1    [ . 521 . ... ]
          |      \____________________________,
          V                                   V
Dir2    [ . 21 . 41 .      ...   501 . ]  [ . 521 . ... 1021 . ]
          |    \___, \_______,        \____,
          V        V         V             V
Data    [1...20] [21...40] [41...60] ... [501...520]
```

The first level directory page leads to the second level directory page, and so forth.

You should use the single-pass algorithm for ISAM tree construction that we created in class.
1. Start by scanning the data pages, one page at a time.
2. Build the ISAM index by continuously appending to the first level of directory pages.  You are encouraged to use functionality from `record.c` to make this easier.
3. At some point, you will run out of space on the lowest level directory page.  At this point, create a new directory page and add a new entry to the directory page on the next level up.
4. If you are already at the topmost directory page, create a new level.

Normally, you might implement the algorithm above by creating a stack of directory pages.  Stacks are typically implemented with Vector types (what 250 would have called an ArrayBuffer, or a resizable Array), but C does not have a Vector type.  The closest analog is the function `realloc`, which allows you to resize an allocated block of memory.  See the man pages for more details.

Once the ISAM index is created, you will need to store information about the index's details (e.g., the root page of the index, key index, key type, and tree depth).  You should store this in a persistent location (e.g. in the table header), and doing so will likely require modifying your PA3 code.  Note that you may add references to external functions in one file by including their function signature.  For example, let's say I define the following function in `foo.c`:
```c
Result foo(int thing) {
  printf("Thing is :%d\n");
}
```
In order to access this function from `bar.c` without modifying header files, I could include the function signature:
```c
Result foo(int thing);
```
(this is, in effect what including header files does)

#### Submission and Late Policy

Run `make submit` to generate a `submission.tgz` file that you can upload to Autolab; this file will contain `record.c`, `table.c`, and `isam.c`.  You may submit as many times as you like before the deadline.  You may submit for up to 48 hours after the deadline, but each partial day over the deadline consumes a grace day if you have any remaining, or reduces your grade by 25% of the maximum (2.5 pt) if you do not.  

