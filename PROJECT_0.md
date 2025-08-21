## Project 1: Buffer Manager

In this project, you will implement a buffer manager.

- **Handout**: https://git.odin.cse.buffalo.edu/CSE-350/2025-fa-project
- **Submission**: https://autolab.cse.buffalo.edu/courses/cse350-f25/assessments/File-API

This project will be scored out of 10 points and is worth 10% of the overall project component of your course grade.

#### Learning Goals
- Remind yourself how C works
- Refamiliarize yourself with the C streams api

#### Resources
- The `man` pages (e.g., `man fread`) on the command line are very useful.  Useful functions for this assignment include:
    - `fread`
    - `fwrite`
    - `fseek`
- Principles in Data Management Chapter 1

Note that `fread` and `fwrite` are NOT guaranteed to read or write the full amount.  

You may use any function provided in a standard LIBC header file, including:
- `<stdio.h>`
- `<stdlib.h>`

As a reminder, you may **not** use coding assistants on this project.

#### Working with C

This course assumes that you have taken CSE 220 or an equivalent course and have an environment set up for working with C (e.g., the 220 VM).  You are encouraged to use a text editor such as Emacs, Sublime, Vim/Neovim, Helix, etc...  Your instructor is an old fogie and may not be able to help you if you use a "modern" IDE that hides all of its activities behind several layers of abstraction. 

This project comes with a Makefile.  Feel free to edit it as you see fit.

* `make`: Compile your code into a binary at `src/350_no_scope`
* `make run`: Compile your code and run it
* `make test`: Run all test scripts in `tests`

#### Objectives

You must implement the following functions in `src/file.c`.  Details and instructions for each function are provided in the function's documentation comment.

```c
Result page_read(FILE *file, PageID page_id, Frame frame);
Result page_write(FILE *file, PageID page_id, Frame frame);
```

You may define additional functions, structures, types, etc... as needed.  However note that you will only be submitting `buffer.c`, so keep changes to other files minimal. 

#### Testing

The main function for this project implements a toy REPL that you may use for testing.  A simple example may be found in `tests/0.test`.  After you run this test, the file `database.350` should contain exactly 32 lines of 31 underscores, followed by a like number of hyphens and asterisks each.


Available commands include
* `W [page_id][data]`: Write `[data]` to page `[page_id]`.  `[data]` must be *exactly* `PAGE_SIZE` bytes.
* `R [page_id][data]`: Read page `[page_id]` to standard out.
* `f`: Ensure that changes are reflected on disk

#### Submission and Late Policy

Upload `src/file.c` to Autolab.  You may submit as many times as you like before the deadline.  You may submit for up to 48 hours after the deadline, but each partial day over the deadline consumes a grace day if you have any remaining, or reduces your grade by 25% of the maximum (2.5 pt) if you do not.  

