/* Test header for gengtype coverage - contains diverse GTY-annotated types */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_test_struct GTY(()) my_user_struct_t;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_field;
  char * GTY((skip)) string_field;
  struct my_test_struct *struct_field;
};

/* TYPE_POINTER: Pointer type with GTY annotation */
extern struct my_test_struct * GTY(()) my_test_pointer;

/* TYPE_ARRAY: Array with GTY annotation */
extern int GTY(()) my_test_array[10];

/* TYPE_CALLBACK: Function pointer (callback) with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
/* This mimics language-specific structs in GCC */
struct GTY(()) my_lang_struct {
  int lang_specific_data;
  struct my_test_struct *base_struct;
};
#endif

/* Complex nested example to ensure thorough processing */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;           /* TYPE_SCALAR */
  const char * GTY((skip)) name;      /* TYPE_STRING */
  struct my_test_struct * GTY((tag("0"))) first;  /* TYPE_POINTER */
  union my_test_union data;           /* TYPE_UNION */
  int GTY((length("array_len"))) dynamic_array[1]; /* TYPE_ARRAY */
  int array_len;
  
  /* Callback field */
  my_callback_fn GTY((skip)) callback; /* TYPE_CALLBACK */
};

/* Forward declaration for pointer chain */
struct GTY(()) forward_decl_struct;
struct forward_decl_struct {
  int data;
  struct forward_decl_struct * GTY((skip)) next;
};

/* Another union with nested struct */
union GTY(()) nested_union {
  struct {
    int x;
    int y;
  } GTY((skip)) point;
  struct my_test_struct obj;
};

/* Array of pointers */
typedef struct my_test_struct * GTY(()) ptr_array_t[5];

/* Struct with array of structs */
struct GTY(()) struct_with_array {
  struct my_test_struct GTY((length("count"))) items[10];
  int count;
};

#endif /* MYTEST_GTY_H */
