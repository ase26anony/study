/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_struct_type GTY(()) {
  int field1;
  my_scalar_type field2;
  void *field3;
};

/* TYPE_USER_STRUCT: User-defined structure 
   Defined in separate scope with user marker */
#define USER_GTY_MARKER 1
struct my_user_struct_type GTY((user)) {
  int user_data;
  struct my_struct_type *link;
};
#undef USER_GTY_MARKER

/* TYPE_UNION: Union type marked with GTY */
union my_union_type GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val;
  my_string_type str_val;
};

/* TYPE_POINTER: Pointer type with ptr attribute */
typedef struct incomplete_struct *opaque_ptr_type GTY((ptr));

/* Forward declaration for pointer type */
struct incomplete_struct;

/* Another pointer type to scalar */
typedef my_scalar_type *scalar_ptr_type GTY(());

/* TYPE_ARRAY: Array types */
typedef int fixed_array_type[10] GTY(());

/* Variable length array type */
struct array_container GTY(()) {
  int length;
  int elements GTY((length("%0.length")));
};

/* Flexible array member */
struct flexible_struct GTY(()) {
  int count;
  int array GTY((length("0")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func_type)(int, const char *) GTY((callback));

/* Callback structure */
struct callback_container GTY(()) {
  callback_func_type handler;
  void *user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct c_lang_struct GTY((tag("C"))) {
  int lang_specific_data;
  struct my_struct_type *associated;
};

struct cplusplus_lang_struct GTY((tag("CPLUSPLUS"))) {
  void *vtable;
  int cpp_data;
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr_type;

/* Malformed GTY annotation to trigger undefined */
struct problematic_type {
  int x;
} /* Missing GTY annotation */;

/* Another undefined case: type with GTY but no definition */
typedef struct never_defined GTY(()) *never_defined_ptr;

/* Nested structures for complex testing */
struct outer_container GTY(()) {
  struct my_struct_type inner;
  union my_union_type choice;
  struct c_lang_struct *lang_ptr;
  callback_func_type callback;
};

/* Template-like structure for comprehensive coverage */
struct comprehensive_type GTY(()) {
  /* Scalar */
  my_scalar_type scalar;
  
  /* String */
  my_string_type str;
  
  /* Struct */
  struct my_struct_type nested_struct;
  
  /* Pointer */
  struct outer_container *container_ptr;
  
  /* Array */
  int number_array[5];
  
  /* Union */
  union my_union_type data_union;
  
  /* Callback */
  callback_func_type action;
  
  /* Lang struct */
  struct c_lang_struct lang_data;
};

/* Enum type (treated as scalar for counting) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type GTY(());

/* Function declarations that use these types */
void process_struct(struct my_struct_type *s GTY(()));
void handle_callback(callback_func_type cb);
struct c_lang_struct *create_lang_struct(void);

#endif /* TEST_GTYPE_H */
