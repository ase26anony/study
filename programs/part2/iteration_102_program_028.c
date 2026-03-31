/* test-gty-callback.h - Header with GTY annotations for gengtype coverage */

#ifndef TEST_GTY_CALLBACK_H
#define TEST_GTY_CALLBACK_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* 1. Define a callback function pointer type with GTY((callback)) */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* 2. Another callback type with different signature */
typedef const char* (*string_callback_fn)(void*, int) GTY((callback));

/* 3. Plain struct type (TYPE_STRUCT) */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* 4. Union type (TYPE_UNION) */
union GTY(()) my_union {
  int int_val;
  void* GTY((skip)) ptr_val;
  double double_val;
};

/* 5. Pointer type (TYPE_POINTER) */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* 6. Array type within a struct (TYPE_ARRAY) */
struct GTY(()) array_container {
  int numbers[20];
  char* GTY((length("strlen(%h.name)+1"))) name;
};

/* 7. Scalar typedef (TYPE_SCALAR) */
typedef unsigned long my_scalar GTY((length));

/* 8. String type (TYPE_STRING) - using GTY tag for strings */
typedef const char* my_string GTY((tag("STRING_TYPE")));

/* 9. Struct containing callback pointers (nested callback) */
struct GTY(()) callback_container {
  /* Direct callback pointer */
  simple_callback_fn direct_cb;
  
  /* Array of callbacks */
  string_callback_fn callback_array[5];
  
  /* Union containing a callback */
  union {
    simple_callback_fn cb_fn;
    int cb_id;
  } GTY((desc("1"))) callback_union;
};

/* 10. Another struct with multiple callback types */
struct GTY(()) complex_callback_struct {
  /* Pointer to struct with callback */
  callback_container* GTY((skip)) container_ptr;
  
  /* Callback function pointer field */
  void (*action_cb)(struct complex_callback_struct*) GTY((callback));
  
  /* Nested struct with callback */
  struct {
    simple_callback_fn nested_cb;
    int priority;
  } GTY(()) nested;
};

/* 11. Union that can hold either a callback or other data */
union GTY(()) callback_or_data {
  simple_callback_fn callback;
  int data;
  struct complex_callback_struct* GTY((skip)) struct_ptr;
};

/* 12. Typedef for a callback returning a callback */
typedef simple_callback_fn (*meta_callback_fn)(int) GTY((callback));

/* 13. Struct using the meta-callback */
struct GTY(()) meta_container {
  meta_callback_fn get_callback;
  void* GTY((skip)) user_data;
};

/* 14. Undefined forward declaration (TYPE_UNDEFINED during processing) */
struct GTY(()) forward_declared_struct;

/* 15. Complete the forward declared struct with a callback */
struct GTY(()) forward_declared_struct {
  simple_callback_fn completion_cb;
  int state;
};

/* 16. Language-specific structure (TYPE_LANG_STRUCT if properly tagged) */
struct GTY(("language_data")) lang_specific {
  int lang_id;
  void (*lang_callback)(void) GTY((callback));
};

/* 17. User-defined structure type (TYPE_USER_STRUCT) */
/* This would require a user-defined marking routine, but we'll include
   a declaration that might be interpreted as user struct */
typedef struct GTY(()) simple_struct my_user_struct;

#endif /* TEST_GTY_CALLBACK_H */
