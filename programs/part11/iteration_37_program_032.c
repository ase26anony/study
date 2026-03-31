/* Test header for gengtype coverage testing */
/* This file defines all type categories to trigger the switch cases in gengtype.cc */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Plain C structure */
struct my_struct GTY(()) {
  int a;
  my_scalar b;
};

/* TYPE_USER_STRUCT: User-defined structure */
/* Often structures from plugins or extensions are considered user structs */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_struct *link GTY((skip));
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
  int i;
  void *p;
  my_scalar s;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr;

/* TYPE_POINTER: Pointer type (to incomplete type) */
typedef struct undefined_struct *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array type (flexible array) */
struct array_container GTY(()) {
  int length;
  int data[] GTY((length("%0.length")));
};

typedef int int_array[] GTY((length("0")));

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to identify as language-specific */
struct GTY((tag("LANG"))) lang_struct {
  int lang_data;
  callback_fn handler;
};

/* Another undefined type for counting */
union undefined_union;

/* Complex nested types to ensure traversal */
struct container GTY(()) {
  /* TYPE_STRUCT member */
  struct my_struct nested_struct;
  
  /* TYPE_POINTER member */
  opaque_ptr ptr_field;
  
  /* TYPE_ARRAY member (pointer to array) */
  int_array *array_ptr;
  
  /* TYPE_UNION member */
  union my_union union_field;
  
  /* TYPE_CALLBACK member */
  callback_fn callback_field;
  
  /* TYPE_STRING member */
  my_string string_field;
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_struct *lang_ptr;
  
  /* TYPE_USER_STRUCT */
  struct my_user_struct user_field;
};

/* Template-like structure for additional coverage */
template<typename T>
struct GTY((template)) template_struct {
  T data;
  T* next GTY((skip));
};

/* Explicit instantiation for gengtype */
typedef template_struct<int> int_template GTY(());

/* Enum type (treated as scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Bitmask type */
typedef unsigned int bitmask GTY(());

/* Nested pointer structure */
struct nested_pointers GTY(()) {
  struct nested_pointers *self_ptr GTY((skip));
  struct container *container_ptr;
  void *generic_ptr GTY((ptr));
};

/* Variable length structure */
struct var_struct GTY(()) {
  int count;
  /* Variable length array at end */
  int items[1] GTY((length("%0.count")));
};

/* Union with pointers */
union pointer_union GTY(()) {
  struct my_struct *struct_ptr;
  struct lang_struct *lang_ptr;
  callback_fn func_ptr;
  const char *string_ptr GTY((string));
};

/* Callback with context */
typedef void (*context_callback)(void *context GTY((skip)), int value) GTY((callback));

/* Structure with callback */
struct with_callback GTY(()) {
  context_callback cb;
  void *context GTY((skip));
};

/* Array of pointers */
typedef opaque_ptr *ptr_array GTY((length("10")));

/* Multi-dimensional array */
typedef int matrix[3][4] GTY(());

/* Structure with bitfields */
struct bitfield_struct GTY(()) {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int value : 8;
  unsigned int : 5; /* padding */
  unsigned int last : 16;
};

/* Anonymous union within struct */
struct anon_union_struct GTY(()) {
  int type;
  union {
    int int_value;
    float float_value;
    const char *str_value GTY((string));
  } data;
};

/* Self-referential structure */
struct tree_node GTY(()) {
  int value;
  struct tree_node *left GTY((skip));
  struct tree_node *right GTY((skip));
  struct tree_node *parent GTY((skip));
};

/* Opaque type declaration (will be TYPE_UNDEFINED) */
typedef struct _secret *secret_handle;

/* Another undefined type */
struct forward_declared;

/* Typedef to undefined type */
typedef struct forward_declared mystery_type;

/* Const pointer typedef */
typedef const struct my_struct *const_struct_ptr GTY((ptr));

/* Volatile pointer */
typedef volatile int *volatile_int_ptr GTY((ptr));

/* Function type (not callback-marked) */
typedef int (*plain_func_ptr)(void);

/* Marked function pointer */
typedef int (*marked_func_ptr)(void) GTY((callback));

/* Structure with array of function pointers */
struct dispatch_table GTY(()) {
  int num_funcs;
  marked_func_ptr funcs[] GTY((length("%0.num_funcs")));
};

/* Union with array */
union array_union GTY(()) {
  int ints[4];
  float floats[4];
  char chars[16];
};

/* Packed structure */
struct packed_struct GTY(()) {
  char a;
  int b;
  short c;
} __attribute__((packed));

/* Aligned structure */
struct aligned_struct GTY(()) {
  long long data;
} __attribute__((aligned(64)));

/* Transparent union */
typedef union {
  int *int_ptr;
  void *void_ptr;
} transparent_union GTY(());

#endif /* TEST_GTYPE_H */
