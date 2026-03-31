/* gty-callback-test.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_CALLBACK_TEST_H
#define GTY_CALLBACK_TEST_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"
#include "coretypes.h"

/* 1. Define a callback function pointer type with GTY((callback)) */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* 2. Another callback type with different signature */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* 3. Plain struct type (TYPE_STRUCT) */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* 4. Union type (TYPE_UNION) */
union GTY(()) my_union {
  int a;
  void* GTY((skip)) b;
  simple_struct* c;
};

/* 5. Pointer type (TYPE_POINTER) */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* 6. Array type within a struct (TYPE_ARRAY) */
struct GTY(()) with_array {
  int arr[10];
  simple_callback_fn callbacks[5];
};

/* 7. Scalar typedef (TYPE_SCALAR) */
typedef unsigned long my_scalar GTY((length));

/* 8. String type (TYPE_STRING) */
typedef const char* my_string GTY((length));

/* 9. Struct containing callback pointers (nested callback) */
struct GTY(()) callback_container {
  /* Direct callback pointer */
  simple_callback_fn handler;
  
  /* Array of callbacks */
  complex_callback_fn handlers[3];
  
  /* Union containing callback */
  union {
    simple_callback_fn fn;
    int id;
  } GTY((desc("1"))) callback_union;
  
  /* Pointer to another struct with callback */
  struct GTY(()) nested {
    simple_callback_fn nested_cb;
    int data;
  } *nested_ptr;
};

/* 10. Another struct mixing different types */
struct GTY(()) mixed_types {
  /* Scalar fields */
  int count;
  my_scalar length;
  
  /* String field */
  my_string name;
  
  /* Callback field */
  complex_callback_fn processor;
  
  /* Array field */
  simple_struct items[20];
  
  /* Pointer field */
  callback_container* GTY((tag("CONTAINER_PTR"))) container;
  
  /* Union field */
  my_union value;
};

/* 11. Typedef for a callback type */
typedef simple_callback_fn callback_alias GTY((callback));

/* 12. Struct using the callback alias */
struct GTY(()) uses_alias {
  callback_alias alias_cb;
  int priority;
};

/* 13. Complex nested structure with multiple callback levels */
struct GTY(()) outer_container {
  /* Direct callback */
  simple_callback_fn notify;
  
  /* Struct containing callback */
  callback_container inner;
  
  /* Array of structs with callbacks */
  uses_alias aliases[4];
  
  /* Union with callback option */
  union GTY(()) data_or_callback {
    int data;
    simple_callback_fn callback;
  } choice;
  
  /* Pointer to mixed types */
  mixed_types* mixed;
};

/* 14. Self-referential structure with callback */
struct GTY(()) tree_node {
  int type;
  simple_callback_fn traverse;
  struct tree_node* GTY((skip)) left;
  struct tree_node* GTY((skip)) right;
};

/* 15. Callback in union with tag */
union GTY((desc ("%0.type"))) tagged_union {
  int type;
  struct {
    simple_callback_fn action;
    int param;
  } GTY((tag ("1"))) callback_case;
  struct {
    int value;
    char* GTY((length)) name;
  } GTY((tag ("2"))) data_case;
};

#endif /* GTY_CALLBACK_TEST_H */
