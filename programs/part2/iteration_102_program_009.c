#ifndef GTY_TEST_HEADER_H
#define GTY_TEST_HEADER_H

/* Include necessary GCC headers */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Core callback type definition */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Another callback with different signature */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_STRUCT: Plain struct */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_USER_STRUCT: Struct with tag */
struct GTY((tag("USER_STRUCT"))) user_struct {
  long id;
  char* name;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int as_int;
  void* as_ptr;
  double as_double;
};

/* TYPE_POINTER: Pointer type with tag */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Array within struct */
struct GTY(()) array_container {
  int numbers[10];
  char strings[5][20];
};

/* TYPE_LANG_STRUCT: Language-specific struct (simulated) */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
  struct lang_struct* next;
  struct lang_struct* prev;
  int lang_specific_data;
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer */
typedef const char* my_string GTY((length));

/* Nested callback structure - callback inside struct */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  complex_callback_fn validator;
  int state;
};

/* Array of callbacks */
struct GTY(()) multi_handler {
  simple_callback_fn handlers[3];
  int priority;
};

/* Union containing callback */
union GTY(()) callback_union {
  simple_callback_fn fn_ptr;
  int (*regular_fn)(void);  /* Not GTY-marked */
  int id;
};

/* Complex nested structure with multiple callback types */
struct GTY(()) complex_system {
  /* TYPE_STRUCT member */
  simple_struct base;
  
  /* TYPE_CALLBACK members */
  simple_callback_fn on_start;
  complex_callback_fn on_data;
  
  /* TYPE_POINTER to callback */
  simple_callback_fn* callback_array GTY((length("%0 ? 5 : 0")));
  
  /* TYPE_ARRAY of callbacks */
  complex_callback_fn callbacks[2];
  
  /* TYPE_UNION containing callback */
  callback_union u;
  
  /* TYPE_STRING */
  const char* name;
};

/* Callback type used in typedef */
typedef simple_callback_fn alias_callback GTY((callback));

/* Struct using the callback alias */
struct GTY(()) uses_alias {
  alias_callback cb;
  int count;
};

/* Self-referential structure with callback */
struct GTY(()) recursive_with_callback {
  simple_callback_fn process;
  struct recursive_with_callback* next;
  struct recursive_with_callback* prev;
};

#endif /* GTY_TEST_HEADER_H */
