/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_struct GTY(()) {
  int a;
  my_scalar b;
};

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This is typically a structure from plugin/extension code */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_struct *link GTY((skip));
};

/* TYPE_UNION: Union type marked with GTY */
union my_union GTY(()) {
  int i;
  void *p;
  my_string s;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;

/* TYPE_POINTER: Pointer to incomplete/undefined type */
typedef struct undefined_struct *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array type with length specification */
typedef int flexible_array[] GTY((length("my_struct::a")));

/* Multi-dimensional array */
typedef int matrix[10][20] GTY(());

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to identify as language-specific */
struct GTY((tag("LANG"), desc("language_specific"))) lang_struct {
  int lang_data;
  callback_fn handler;
};

/* Another TYPE_STRUCT with nested structures */
struct container GTY(()) {
  struct my_struct inner;
  union my_union choice;
  flexible_array flex_data;
};

/* TYPE_UNDEFINED: Incomplete type declaration */
struct undefined_struct GTY(());  /* Forward declaration without definition */

/* Pointer chain for testing */
typedef struct container *container_ptr GTY(());

/* Array of pointers */
typedef struct my_struct *struct_array[5] GTY(());

/* Union with struct member */
union complex_union GTY(()) {
  struct container c;
  lang_struct ls;
  opaque_ptr unknown;
};

/* Callback with context */
typedef int (*filter_fn)(const struct my_struct *, void *context) 
  GTY((callback));

/* String array */
typedef const char *string_array[] GTY((length("0")));

/* Self-referential structure */
struct node GTY(()) {
  int value;
  struct node *next GTY((skip));
  struct node *prev GTY((skip));
};

/* Template-like structure (C doesn't have templates but we can simulate) */
#define DECLARE_CONTAINER(TYPE) \
  struct container_##TYPE GTY(()) { \
    TYPE data; \
    struct container_##TYPE *next; \
  }

/* Instantiate for different types */
DECLARE_CONTAINER(int);
DECLARE_CONTAINER(my_string);

/* Variadic structure (using zero-length array) */
struct variadic_struct GTY(()) {
  int count;
  int data[];  /* Flexible array member */
};

/* Bitfield structure */
struct bitfield_struct GTY(()) {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int value : 29;
};

/* Anonymous union within struct */
struct with_anonymous_union GTY(()) {
  int type;
  union {
    int int_val;
    my_string str_val;
    struct my_struct *struct_val;
  } GTY((desc ("type"))) u;
};

/* Constant pointer */
typedef int * const const_ptr GTY(());

/* Function returning pointer */
typedef struct my_struct *(*allocator_fn)(size_t) GTY((callback));

#endif /* TEST_GTYPE_H */
