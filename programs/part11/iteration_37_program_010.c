/* test-gtype.h - Comprehensive test file for gengtype type categorization */
/* This file should be placed in gcc/test-gtype/ directory */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar GTY(());
typedef unsigned long scalar_ulong GTY(());
typedef double scalar_double GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string GTY((string));
typedef char *mutable_string GTY((string));
typedef const char *const const_string_ptr GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_struct GTY(()) {
  int a;
  double b;
  my_string str;
};

/* Forward declaration for TYPE_UNDEFINED test */
struct undefined_struct;

/* Another undefined type reference */
typedef struct incomplete *incomplete_ptr GTY(());

/* TYPE_USER_STRUCT: User-defined structure */
/* Defined in separate user context or with special marker */
struct GTY((user)) user_defined_struct {
  int user_data;
  struct my_struct *nested GTY(());
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  my_string s;
};

union tagged_union GTY((desc("tag"))) {
  int tag;
  struct {
    int type;
    void *data GTY(());
  } variant;
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_struct *struct_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef union my_union *union_ptr GTY((ptr));

/* Opaque pointer for TYPE_POINTER */
struct unknown_type;
typedef struct unknown_type *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int int_array[10] GTY(());
typedef struct my_struct *struct_ptr_array[5] GTY(());

/* Variable length array */
struct array_container GTY(()) {
  int length;
  int flexible_array[] GTY((length("0")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *input, char *output) 
  GTY((callback));
typedef void (*struct_callback)(struct my_struct *data) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
  int lang_specific_data;
  void *lang_private GTY(());
};

/* Another language struct with different tag */
struct GTY((tag("CPLUSPLUS"))) cxx_lang_struct {
  int cxx_data;
  struct lang_struct *base_lang GTY(());
};

/* Complex nested type to test traversal */
struct complex_container GTY(()) {
  /* TYPE_SCALAR */
  int counter GTY(());
  
  /* TYPE_STRING */
  my_string name GTY(());
  
  /* TYPE_STRUCT reference */
  struct my_struct embedded GTY(());
  
  /* TYPE_POINTER */
  struct_ptr ptr_field GTY(());
  
  /* TYPE_ARRAY */
  int_array number_array GTY(());
  
  /* TYPE_UNION */
  union my_union data_union GTY(());
  
  /* TYPE_CALLBACK */
  simple_callback handler GTY(());
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_struct *lang_data GTY((ptr));
  
  /* For TYPE_UNDEFINED test */
  struct undefined_struct *undefined_ref GTY(());
};

/* Enum type (should be treated as scalar) */
typedef enum {
  STATE_INIT,
  STATE_PROCESSING,
  STATE_DONE
} process_state GTY(());

/* Template-like macro usage */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct name##_container GTY(()) { \
    field_type value GTY(()); \
    struct name##_container *next GTY(()); \
  }

DEFINE_GTY_STRUCT(int_list, int);
DEFINE_GTY_STRUCT(string_list, my_string);

/* Self-referential structure */
struct tree_node GTY(()) {
  int value;
  struct tree_node *left GTY(());
  struct tree_node *right GTY(());
  struct tree_node *parent GTY((ptr));
};

/* Container with callback array */
struct callback_registry GTY(()) {
  int num_callbacks;
  filter_callback callbacks[5] GTY(());
};

#endif /* TEST_GTYPE_H */
