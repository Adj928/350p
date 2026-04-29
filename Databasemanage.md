## Project 3: Database management

In this project, you will implement a basic database with support for table creation and access

- **Handout**: https://git.odin.cse.buffalo.edu/CSE-350/2025-fa-project/src/branch/project_3/PROJECT_3.md
- **Submission**: https://autolab.cse.buffalo.edu/courses/cse350-f25/assessments/Simple-Database

This project is scored out of 15 points and is worth 15% of the overall project component of your course grade.

#### Updating Your Repository

Assuming you have configured your upstream with `project` as your upstream remote, as in PA0, then the following will update your repository:
```bash
git fetch project
git merge project/project_3
```

#### Learning Goals
- Lay the groundwork for tree-based indexing
- Implement a page-based allocator
- Implement a linked-list 
- Become familiar with managing objects that span multiple pages

#### Objectives

You must implement the following functions in `src/table.c`.  Details and instructions for each function are provided in the function's documentation comment.

The following functions manage the database:
```c
Result database_init(BufferManager *manager, size_t reserved_pages);
Result database_page_allocate(BufferManager *manager, size_t *page);
Result database_page_release(BufferManager *manager, size_t page);
```

The following functions manage tables:
```c
Result table_init(BufferManager *manager, size_t header_page, TableConfig *config, Table *table);
Result table_open(BufferManager *manager, size_t header_page, Table *table);
Result table_field_count(Table table, size_t *count);
Result table_close(Table table);
Result table_append(Table table, Field field_data[], size_t field_length[]);
```

The following functions manage table iterators:
```c
Result table_iterate(Table table, TableIterator *iterator);
Result table_iterator_next(TableIterator iterator, Record *record);
Result table_iterator_close(TableIterator iterator);
```

#### Guidelines

All functions in this assignment should have a **constant** IO, Memory, and Runtime complexity.

This is the first assignment where we will be using more complex objects that span multiple pages.  There are two such objects in this assignment: The database, and tables.  For every such object we will use a 'header' page to store any important structural information about the page.  The amount of data stored on these pages will typically be *much* smaller than PAGE_SIZE, and that is OK.  Typically, the objects as a whole will require many pages, so the overhead of a bit of wasted space on one page isn't going to be a huge deal.  The database's header page should be stored on the page `DATABASE_ROOT_PAGE`, while the table header pages will be stored on the page passed to `table_init` or `table_open`.

The database functions reduce to managing available pages.  In general `database_page_allocate` should draw from two different sources of free pages: 
1. The end of the file
2. Pages previously freed with `database_page_release`
Priority should go to previously freed pages, as this will avoid growing the file if possible.

As discussed in class, an easy way to manage free pages is to build a *stack*.  You can store the index of the page on top of the stack (or a null reference if there is no such page), and use the free pages themselves to store the next page on the stack.
i
When `database_init` is called, it is passed a number of reserved pages.  The first page at the 'end of the file' to be allocated should be the page following the last reserved page (i.e., the first allocated page should be the one with id `reserved_pages`).  Note that `database_init` will only be called once.  Calls to `database_page_allocate` or `database_page_release` should consult the database state as stored on page `DATABASE_ROOT_PAGE`.

The internals of your table implementation are not constrained; you can implement them as you see fit.  The reference implementation uses a simple doubly-linked list, using the methods defined in `record.c`.  The records at indexes 0 and 1 each consist of a single 8-byte field that stores the indexes of the next and previous pages.  The remaining records on the page store actual data.  Structural information provided by the `TableConfig` (e.g., the number of fields per record) is stored in the table's header.

The reference implementation's approach is by no means the most efficient way to implement an unindexed table, and you are encouraged to explore alternatives.

#### Testing

The REPL (and test cases) has been rewritten to make it easier to work with, and to read/write test cases.  See REPL.md for documentation.

#### Submission and Late Policy

Run `make submit` to generate a `submission.tgz` file that you can upload to Autolab; this file will contain `record.c` and `table.c`.  You may submit as many times as you like before the deadline.  You may submit for up to 48 hours after the deadline, but each partial day over the deadline consumes a grace day if you have any remaining, or reduces your grade by 25% of the maximum (3.75 pt) if you do not.  

