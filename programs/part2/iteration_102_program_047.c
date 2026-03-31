/* gty-callback-test.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_CALLBACK_TEST_H
#define GTY_CALLBACK_TEST_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* 1. Define a callback function pointer type with GTY((callback)) */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* 2. Another callback type with parameters */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* 3. Plain struct (TYPE_STRUCT) */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* 4. Union type (TYPE_UNION) */
union GTY(()) my_union {
  int int_val;
  void* ptr_val;
  simple_callback_fn callback_val;
};

/* 5. Pointer type (TYPE_POINTER) */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* 6. Struct containing callback (nested callback structure) */
struct GTY(()) callback_container {
  /* Direct callback field */
  simple_callback_fn handler GTY((tag("HANDLER")));
  
  /* Array of callbacks */
  complex_callback_fn handlers[3];
  
  /* Union containing callback */
  union GTY(()) {
    simple_callback_fn fn;
    int id;
  } callback_union;
};

/* 7. Array type within struct (TYPE_ARRAY) */
struct GTY(()) with_arrays {
  int int_array[10];
  simple_callback_fn callback_array[5];
};

/* 8. Scalar typedef (TYPE_SCALAR) */
typedef unsigned long my_scalar GTY((length));

/* 9. String type (TYPE_STRING) - char* with special handling */
typedef const char* my_string GTY((length));

/* 10. Struct with multiple callback types */
struct GTY(()) multi_callback_struct {
  simple_callback_fn simple_cb;
  complex_callback_fn complex_cb;
  
  /* Pointer to struct containing callback */
  callback_container* container_ptr;
  
  /* Union with callback alternative */
  union GTY(()) {
    simple_callback_fn cb_fn;
    struct_ptr s_ptr;
  } choice;
};

/* 11. Callback in a chain of structures */
struct GTY(()) outer_struct {
  int id;
  struct GTY(()) inner_struct {
    simple_callback_fn cb;
    int data;
  } inner;
  
  /* Array of structs containing callbacks */
  struct GTY(()) {
    complex_callback_fn handler;
    char name[20];
  } named_handlers[4];
};

/* 12. Typedef for callback function pointer */
typedef simple_callback_fn callback_alias GTY((callback));

/* 13. Struct using the callback alias */
struct GTY(()) uses_alias {
  callback_alias alias_cb;
  int counter;
};

/* 14. Complex nested structure with multiple callback types */
struct GTY(()) complex_nested {
  struct GTY(()) level1 {
    struct GTY(()) level2 {
      simple_callback_fn level2_cb;
      struct GTY(()) level3 {
        complex_callback_fn level3_cb;
        union GTY(()) {
          simple_callback_fn simple;
          complex_callback_fn complex;
          void* generic;
        } callback_choice;
      } deepest;
    } middle;
  } top;
};

/* 15. Self-referential structure with callback */
struct GTY(()) recursive_struct {
  int value;
  simple_callback_fn processor;
  struct recursive_struct* next;
};

/* 16. Union primarily for callbacks */
union GTY(()) callback_dispatcher {
  simple_callback_fn simple;
  complex_callback_fn complex;
  void (*void_callback)(void) GTY((callback));
};

/* 17. Struct with conditional callback inclusion */
struct GTY(()) conditional_callbacks {
#ifdef FEATURE_A
  simple_callback_fn feature_a_cb;
#endif
#ifdef FEATURE_B  
  complex_callback_fn feature_b_cb;
#endif
  simple_callback_fn always_present_cb;
};

/* 18. Array of callback pointers */
typedef simple_callback_fn callback_ptr_array[10];

/* 19. Struct using the array typedef */
struct GTY(()) uses_callback_array {
  callback_ptr_array callbacks;
  int count;
};

/* 20. Final comprehensive structure exercising all patterns */
struct GTY(()) master_test_struct {
  /* Basic types */
  int scalar_field;
  my_scalar typed_scalar;
  my_string string_field;
  
  /* Callbacks */
  simple_callback_fn basic_callback;
  complex_callback_fn data_callback;
  
  /* Nested structures */
  callback_container container;
  outer_struct outer;
  
  /* Pointers */
  struct_ptr s_ptr;
  recursive_struct* recursive_chain;
  
  /* Unions */
  my_union u;
  callback_dispatcher dispatcher;
  
  /* Arrays */
  simple_callback_fn callback_list[8];
  with_arrays array_struct;
  
  /* Typedef usage */
  uses_alias alias_user;
  
  /* Complex nesting */
  complex_nested nested;
  
  /* Array typedef */
  uses_callback_array callback_array_struct;
};

#endif /* GTY_CALLBACK_TEST_H */
