/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_struct_type GTY(()) {
  int field1;
  my_scalar_type field2;
  struct my_struct_type *next GTY((skip));
};

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This typically requires being in a separate module/plugin */
struct GTY((user)) my_user_struct_type {
  int user_data;
  my_string_type user_name;
};

/* TYPE_UNION: Union type marked with GTY */
union my_union_type GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val;
  my_scalar_type scalar_val;
};

/* TYPE_POINTER: Pointer type with ptr option */
typedef struct opaque_type *opaque_ptr_type GTY((ptr));

/* Forward declaration for opaque type */
struct opaque_type;

/* TYPE_ARRAY: Array type with length specification */
typedef int flexible_array_type[] GTY((length("my_struct_type::field1")));

/* Another array type */
struct array_container GTY(()) {
  int count;
  int elements[10] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func_type)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific_struct {
  int lang_data;
  callback_func_type lang_callback;
  struct lang_specific_struct *next_lang;
};

/* TYPE_UNDEFINED: Incomplete/forward declared type without proper definition */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr_type GTY(());

/* Another undefined case - struct declared but not defined with GTY */
struct no_gty_struct;

/* Complex nested example to ensure traversal */
struct container_struct GTY(()) {
  /* Scalar */
  int id;
  
  /* String */
  const char *name GTY((string));
  
  /* Struct */
  struct my_struct_type embedded;
  
  /* Pointer */
  struct opaque_type *opaque GTY((ptr));
  
  /* Array */
  int scores[5];
  
  /* Union */
  union my_union_type value;
  
  /* Callback */
  callback_func_type handler GTY((callback));
  
  /* Nested struct with lang tag */
  struct GTY((tag("NESTED_LANG"))) nested_lang {
    int nested_data;
  } nested;
  
  /* Pointer to undefined type */
  struct undefined_struct *undefined GTY(());
};

/* Template-like structure for additional coverage */
struct template_struct GTY(()) {
  enum {
    KIND_SCALAR,
    KIND_STRING,
    KIND_STRUCT,
    KIND_UNION
  } kind;
  
  union {
    my_scalar_type as_scalar;
    my_string_type as_string;
    struct my_struct_type *as_struct;
    union my_union_type *as_union;
  } u GTY((desc("%0.kind")));
};

/* Extern declaration to force inclusion in gtype-desc */
extern struct container_struct global_container GTY(());

#endif /* TEST_GTYPE_H */
