/**
 * @file conventions.h
 * @author Oliver Kennedy
 */
#ifndef CONVENTIONS_H_SHIELD
#define CONVENTIONS_H_SHIELD

#include <stdint.h>
#include <errno.h>

/// The result of a function call.  Note that since SUCCESS = 0 and ERROR = 1 you can use the following code construct:
///
/// ```
/// if(my_call(...)) { 
///   // Handle the error here
/// }
/// ```
typedef enum Result {
  SUCCESS = 0,
  ERROR = 1,
} Result;

/// The identifier for a specific page
typedef uint64_t PageID;

#endif
