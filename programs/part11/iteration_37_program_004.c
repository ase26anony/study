/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_plain_struct GTY(()) {
  int field1;
  my_scalar_type field2;
  struct my_plain_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* Defined in separate user context - will be treated as user struct */
struct GTY((user)) my_user_struct {
  int user_data;
  my_string_type name;
};

/* TYPE_UNION: Union type marked with GTY */
union my_union_type GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val;
  my_scalar_type scalar_val;
};

/* TYPE_POINTER: Pointer to opaque/incomplete type */
struct forward_declared;
typedef struct forward_declared *opaque_ptr_type GTY((ptr));

/* Another pointer type with different options */
typedef struct my_plain_struct *struct_ptr_type GTY((skip));

/* TYPE_ARRAY: Array types */
typedef int fixed_array_type[10] GTY(());

/* Variable length array type */
struct array_container GTY(()) {
  int length;
  int elements GTY((length("%0.length")));
};

/* Flexible array member */
struct flex_array_struct GTY(()) {
  int count;
  int data[] GTY((length("%0.count")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func_type)(int, const char *) GTY((callback));

/* Callback with specific signature for gengtype */
typedef void (*gc_callback_type)(void) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"), desc("%1"))) lang_specific_struct {
  int lang_tag;
  union my_union_type data;
  callback_func_type handler;
};

/* Nested language structure */
struct GTY((tag("CPLUSPLUS"))) cplusplus_struct {
  struct lang_specific_struct *base GTY((skip));
  int cpp_specific_field;
};

/* TYPE_UNDEFINED: Forward declaration without complete definition */
/* This should be categorized as undefined */
struct undefined_struct GTY(());

/* Another undefined case: type with problematic GTY annotation */
typedef void (*problematic_callback)() GTY((invalid_option));

/* Complex nested structure to test traversal */
struct complex_container GTY(()) {
  /* Scalar */
  my_scalar_type id;
  
  /* String */
  my_string_type description;
  
  /* Struct pointer */
  struct my_plain_struct *struct_data GTY((skip));
  
  /* Union */
  union my_union_type variant;
  
  /* Array */
  fixed_array_type numbers;
  
  /* Callback */
  callback_func_type notify;
  
  /* Language struct */
  struct lang_specific_struct *lang_data GTY((skip));
  
  /* Pointer to undefined type */
  struct undefined_struct *undefined_ptr GTY((ptr));
  
  /* User struct */
  struct my_user_struct *user_data GTY((skip));
};

/* Template-like structure for comprehensive testing */
#define DECLARE_GTY_STRUCT(name, field_type) \
  struct name##_container GTY(()) { \
    field_type value; \
    struct name##_container *next GTY((skip)); \
  }

/* Instantiate templates for different types */
DECLARE_GTY_STRUCT(int_container, int);
DECLARE_GTY_STRUCT(string_container, const char * GTY((string)));
DECLARE_GTY_STRUCT(struct_container, struct my_plain_struct *);

/* Enumeration type (should be treated as scalar) */
typedef enum {
  STATE_INIT,
  STATE_PROCESSING,
  STATE_DONE
} process_state GTY(());

/* Structure with conditional fields */
struct conditional_struct GTY(()) {
  int type;
  union {
    int int_value;
    double double_value;
    const char *string_value GTY((string));
  } data GTY((desc("%0.type")));
};

/* Opaque pointer type definition */
struct forward_declared GTY(()) {
  int secret;
  struct forward_declared *next GTY((skip));
};

/* Self-referential structure */
struct recursive_struct GTY(()) {
  int value;
  struct recursive_struct *left GTY((skip));
  struct recursive_struct *right GTY((skip));
};

/* Structure with array of pointers */
struct pointer_array_struct GTY(()) {
  int count;
  struct my_plain_struct *items[10] GTY((skip));
};

/* Union with struct members */
union struct_union GTY(()) {
  struct my_plain_struct as_struct;
  struct complex_container as_complex;
};

#endif /* TEST_GTYPE_H */
