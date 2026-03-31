/* Test header for gengtype coverage - contains various GTY-annotated types */

#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Structure with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
};

/* TYPE_USER_STRUCT: User-defined structure type */
typedef struct my_test_struct GTY(()) my_user_struct_t;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int GTY((tag("0"))) int_field;
  char * GTY((tag("1"))) str_field;
  struct my_test_struct * GTY((tag("2"))) struct_field;
};

/* TYPE_POINTER: Various pointer types with GTY annotations */
extern struct my_test_struct * GTY(()) my_struct_pointer;
extern union my_test_union * GTY(()) my_union_pointer;
extern my_scalar_t * GTY(()) my_scalar_pointer;

/* TYPE_ARRAY: Array types with GTY annotations */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) my_struct_array[5];
extern char * GTY(()) my_string_array[3];

/* TYPE_CALLBACK: Function pointer (callback) with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) my_lang_struct {
  int lang_specific_field;
  void * GTY((skip)) opaque_data;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) complex_container {
  /* Nested scalar */
  my_scalar_t scalar_field;
  
  /* Pointer to struct */
  struct my_test_struct * GTY(()) nested_struct_ptr;
  
  /* Array of pointers */
  union my_test_union * GTY(()) union_array[4];
  
  /* Callback field */
  my_callback_fn GTY(()) callback_field;
  
  /* String field */
  const char * GTY(()) description;
  
  /* Self-referential pointer */
  struct complex_container * GTY(()) next;
};

/* Template-like macro usage (common in GCC) */
#define DEFINE_GTY_STRUCT(name) \
  struct GTY(()) name { \
    int id; \
    struct name * GTY(()) next; \
  }

DEFINE_GTY_STRUCT(linked_list_node);

/* Variable-length array using GTY((length)) */
struct GTY(()) var_len_struct {
  int count;
  int GTY((length("%0.count"))) data[1];
};

/* Union with variable tag field */
struct GTY(()) tagged_union_container {
  enum { TAG_INT, TAG_STRING, TAG_STRUCT } tag;
  union GTY((desc("%0.tag"))) {
    int GTY((tag("TAG_INT"))) int_val;
    const char * GTY((tag("TAG_STRING"))) str_val;
    struct my_test_struct * GTY((tag("TAG_STRUCT"))) struct_val;
  } GTY((desc("%0.tag"))) value;
};

#endif /* MYTEST_H */
