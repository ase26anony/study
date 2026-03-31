/* gty_test_callback.h - Header file with GTY-annotated types for gengtype coverage */
#ifndef GTY_TEST_CALLBACK_H
#define GTY_TEST_CALLBACK_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* 1. Define a callback function pointer type with GTY((callback)) */
typedef void (*my_callback_fn)(int) GTY((callback));

/* 2. Plain struct type */
struct GTY(()) simple_struct {
  int field;
  double value;
};

/* 3. Union type */
union GTY(()) my_union {
  int a;
  void* GTY((skip)) b;  /* skip annotation for void* */
  long c;
};

/* 4. Pointer type to simple_struct */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* 5. Array type within a struct */
struct GTY(()) with_array {
  int arr[10];
  char name[32];
};

/* 6. Scalar typedef with length attribute */
typedef unsigned my_scalar GTY((length));

/* 7. Struct containing callback function pointer */
struct GTY(()) callback_container {
  my_callback_fn handler;
  int id;
};

/* 8. Array of callbacks in a struct */
struct GTY(()) multi_callback {
  my_callback_fn handlers[2];
  simple_ptr ptr;
};

/* 9. Union containing a callback */
union GTY(()) callback_union {
  my_callback_fn fn;
  int callback_id;
  simple_struct data;
};

/* 10. Nested struct with callback */
struct GTY(()) outer_struct {
  struct GTY(()) inner {
    my_callback_fn inner_cb;
    int inner_val;
  } nested;
  
  callback_container container;
  my_union u;
};

/* 11. Typedef for callback function pointer with parameters */
typedef int (*process_callback)(const char*, int) GTY((callback));

/* 12. Struct using the parameterized callback */
struct GTY(()) processor {
  process_callback proc;
  const char* GTY((skip)) name;  /* skip non-GC pointer */
  int priority;
};

/* 13. Complex nested structure with multiple callback types */
struct GTY(()) complex_structure {
  /* Array of callback containers */
  callback_container containers[4];
  
  /* Union that might hold a callback */
  callback_union cu;
  
  /* Direct callback pointer */
  my_callback_fn direct_callback;
  
  /* Pointer to another GTY type */
  simple_ptr next;
  
  /* Embedded array of callbacks */
  process_callback processors[3];
};

#endif /* GTY_TEST_CALLBACK_H */
