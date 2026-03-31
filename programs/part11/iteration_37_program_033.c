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
  double b;
  my_string str;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct GTY(());

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This is typically a struct from plugin/extension code */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_struct *link;
};

/* TYPE_UNION: Union type marked with GTY */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  struct my_struct *s;
};

/* TYPE_POINTER: Pointer type with ptr option */
typedef struct my_struct *struct_ptr GTY((ptr));

/* Another pointer type for opaque/incomplete structure */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());

/* Variable length array */
struct array_container GTY(()) {
  int length;
  int elements GTY((length("%0.length")));
};

/* Flexible array member */
struct flex_array GTY(()) {
  int count;
  int data GTY((length("%0.count")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* Structure containing callback */
struct with_callback GTY(()) {
  callback_fn handler;
  int state;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag option to mark as language-specific */
struct lang_struct GTY((tag("LANG"))) {
  int lang_data;
  void *lang_specific;
};

/* Nested structures to ensure traversal */
struct outer_struct GTY(()) {
  struct my_struct inner;
  union my_union choice;
  struct_ptr next;
  fixed_array numbers;
};

/* Enumeration type (should be treated as scalar) */
typedef enum {
  STATE_A,
  STATE_B,
  STATE_C
} my_enum GTY(());

/* Function pointer in union */
union func_union GTY(()) {
  int (*func_int)(int);
  void (*func_void)(void);
  callback_fn callback;
};

/* Self-referential structure */
struct node GTY(()) {
  int value;
  struct node *next GTY((skip));
  struct node *prev;
};

/* Array of pointers */
typedef struct my_struct *ptr_array[5] GTY(());

/* String array */
typedef const char *string_array[3] GTY((string));

/* Complex nested case */
struct complex_nested GTY(()) {
  struct {
    int x;
    int y;
  } point;
  
  union {
    int i;
    struct my_struct *s;
  } data;
  
  callback_fn handlers[2];
};

/* Incomplete type for TYPE_UNDEFINED */
/* This forward declaration without complete definition should remain undefined */
struct undefined_struct;

/* Another undefined case: type with malformed GTY annotation */
/* Note: This might generate warnings but helps test TYPE_UNDEFINED */
typedef int bad_type GTY((unknown_option));

#endif /* TEST_GTYPE_H */
