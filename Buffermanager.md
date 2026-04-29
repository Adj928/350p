## Project 1: Buffer Manager

In this project, you will implement a buffer manager.

- **Handout**: https://git.odin.cse.buffalo.edu/CSE-350/2025-fa-project-1
- **Submission**: https://autolab.cse.buffalo.edu/courses/cse350-f25/assessments/Buffer-Manager

This project is scored out of 15 points and is worth 15% of the overall project component of your course grade.

#### Learning Goals
- Remind yourself how C works
- Refamiliarize yourself with the C streams api
- Familiarize yourself with the distinction between frames and pages
- Start to grok pointers

#### Resources
- The `man` pages (e.g., `man fread`) on the command line are very useful.  Useful functions for this assignment include:
    - `malloc`
    - `calloc`
- Principles in Data Management Chapter 1.3

Note that:
- `malloc(size * sizeof(Foo))` will allocate an array of `size` elements, each of size `Foo`. `calloc(size, sizeof(Foo))` does the same, but guarantees that the allocated memory will be zeroed out first.

#### Working with C

This course assumes that you have taken CSE 220 or an equivalent course and have an environment set up for working with C (e.g., the 220 VM).  You are encouraged to use a text editor such as Emacs, Sublime, Vim/Neovim, Helix, etc...  Your instructor is an old fogie and may not be able to help you if you use a "modern" IDE that hides all of its activities behind several layers of abstraction. 

This project comes with a Makefile.  Feel free to edit it as you see fit.

* `make`: Compile your code into a binary at `src/350_no_scope`
* `make run`: Compile your code and run it
* `make test`: Run all test scripts in `tests`

#### Objectives

You must implement the following functions in `src/buffer.c`.  Details and instructions for each function are provided in the function's documentation comment.

```c
Result buffer_init(BufferManager *manager, FILE *file, uint64_t frame_count);
Result buffer_pin(BufferManager *manager, PageID page_id, Frame **frame);
Result buffer_mark(BufferManager *manager, Frame *frame);
Result buffer_unpin(BufferManager *manager, Frame *frame);
Result buffer_flush_unpinned(BufferManager *manager);
Result buffer_flush_frame(BufferManager *manager, Frame *frame);
```

You may define additional functions, structures, types, etc... as needed.  You may also use the `extra_data` field of `manager` to allocate additional state.  However note that you will only be submitting `buffer.c`, so keep changes to other files minimal. 

#### Testing

The main function for this project implements a toy REPL that you may use for testing.  A simple example may be found in `tests/1.test`.  After you run this test, the file `database.350` should contain exactly 1024 underscores.


Commands include

* `i [frames] [registers]`: Allocate a buffer manager with `[frames]` frames, and set up an array of `[registers]` temporary registers for use in testing
* `p [register] [page_id]`: Pin the page at `[page_id]` and store the resulting frame at register `[register]`
* `u [register]`: Unpin the frame at register `[register]`
* `w [register][data]`: Write `[data]` to the frame at register `[register]`.  `[data]` must be *exactly* `PAGE_SIZE` bytes.
* `r [register]`: Read the contents of the frame at register `[register]` to standard out.
* `f`: Flush all pages to disk.

#### Submission and Late Policy

Upload `src/buffer.c` to Autolab.  You may submit as many times as you like before the deadline.  You may submit for up to 48 hours after the deadline, but each partial day over the deadline consumes a grace day if you have any remaining, or reduces your grade by 25% of the maximum (3.75 pt) if you do not.  

