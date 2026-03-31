/* test-gtype.h - Test file for gengtype type categorization coverage */
/* This file contains examples of all TYPE_* categories to ensure gengtype */
/* processes each switch case in the type counting function. */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar GTY(());
typedef unsigned long scalar_ulong GTY(());
typedef double scalar_double GTY(());

/* TYPE_STRING: String pointer types with string marker */
typedef const char *my_string GTY((string));
typedef char *mutable_string GTY((string));
typedef const char *const const_string_ptr GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_struct GTY(()) {
  int a;
  double b;
  my_string name;
};

/* Another struct to ensure multiple hits */
struct another_struct GTY(()) {
  struct my_struct *next GTY((skip));
  int counter;
};

/* TYPE_USER_STRUCT: User-defined structure type */
/* User structs are typically those defined in plugins or extensions */
struct GTY((user)) user_defined_struct {
  int user_data;
  void *user_ptr GTY((ptr));
};

/* Alternative approach: struct with special callback marker */
struct user_marked_struct GTY((user)) {
  int id;
  const char *description GTY((string));
};

/* TYPE_UNION: Union types marked with GTY */
union my_union GTY(()) {
  int i;
  double d;
  void *p GTY((ptr));
  const char *s GTY((string));
};

/* TYPE_POINTER: Pointer types with ptr marker */
typedef struct opaque_struct *opaque_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef struct my_struct *struct_ptr GTY((ptr));

/* Forward declaration for opaque pointer */
struct opaque_struct;

/* TYPE_ARRAY: Array types with length markers */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));
typedef struct my_struct *struct_array[] GTY((length("sizeof(struct my_struct*)")));

/* Array within a struct */
struct array_container GTY(()) {
  int count;
  int elements[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types with callback marker */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *input GTY((string)), 
                               void *context GTY((ptr))) GTY((callback));

/* Callback in a struct */
struct callback_holder GTY(()) {
  simple_callback cb GTY((callback));
  void *data GTY((ptr));
};

/* TYPE_LANG_STRUCT: Language-specific structure types */
/* These are typically identified by tag or location */
struct GTY((tag("LANG"))) lang_specific_struct {
  int lang_data;
  void *lang_private GTY((ptr));
};

/* Another language struct with different tag */
struct GTY((tag("CPLUSPLUS"))) cxx_lang_struct {
  int cxx_specific;
  struct lang_specific_struct *base GTY((ptr));
};

/* TYPE_UNDEFINED: Incomplete/forward declarations or malformed types */
/* Forward declaration without definition */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr GTY(());

/* Malformed GTY annotation (missing parentheses) */
typedef int malformed_scalar GTY;

/* Struct with incomplete array (no length specified) */
struct incomplete_container GTY(()) {
  int data;
  int incomplete_array[];
};

/* Enum type (often treated as undefined by gengtype) */
typedef enum {
  VALUE_A,
  VALUE_B
} my_enum GTY(());

/* Function type without callback marker (may be undefined) */
typedef void (*plain_func_ptr)(int);

/* Complex nested example to test traversal */
struct complex_nested GTY(()) {
  /* Scalar */
  int id GTY(());
  
  /* String */
  const char *label GTY((string));
  
  /* Pointer */
  struct complex_nested *next GTY((ptr));
  
  /* Array */
  int scores[] GTY((length("5")));
  
  /* Union */
  union {
    int as_int;
    double as_double;
  } value GTY(());
  
  /* Callback */
  simple_callback handler GTY((callback));
  
  /* Pointer to language struct */
  struct lang_specific_struct *lang_data GTY((ptr));
};

/* Template-like macro to generate multiple instances */
#define DEFINE_SPECIALIZED_STRUCT(name, field_type) \
  struct GTY(()) name { \
    field_type data; \
    struct name *next GTY((ptr)); \
  }

/* Instantiate the macro */
DEFINE_SPECIALIZED_STRUCT(special_int, int);
DEFINE_SPECIALIZED_STRUCT(special_ptr, void*);

/* Variable length struct with trailing pointer array */
struct var_len_struct GTY(()) {
  unsigned count;
  unsigned capacity;
  struct my_struct *items[1] GTY((length("count")));
};

/* Self-referential structure */
struct self_ref GTY(()) {
  int value;
  struct self_ref *left GTY((ptr));
  struct self_ref *right GTY((ptr));
};

/* Bitfield structure */
struct with_bitfields GTY(()) {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Anonymous struct/union */
struct anonymous_member GTY(()) {
  struct {
    int x;
    int y;
  } point;
  union {
    int i;
    float f;
  } value;
};

#endif /* TEST_GTYPE_H */
