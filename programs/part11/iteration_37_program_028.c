/* Test header for gengtype coverage testing */
/* This file defines various GTY-marked types to exercise all type categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

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
/* User structs are typically those defined in plugins or extensions */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_struct *link;
};

/* TYPE_UNION: Union type marked with GTY */
union my_union GTY(()) {
  int i;
  void *p;
  my_scalar s;
};

/* TYPE_POINTER: Pointer type with ptr option */
typedef struct opaque_type *opaque_ptr GTY((ptr));

/* Forward declaration for pointer type */
struct opaque_type;

/* TYPE_ARRAY: Array type with length specifier */
typedef int flexible_array[] GTY((length("0")));

/* Another array type */
struct array_container GTY(()) {
  int count;
  int values[10] GTY(());
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
  int lang_data;
  callback_fn handler;
};

/* TYPE_UNDEFINED: Incomplete/forward declared type without proper definition */
struct undefined_struct GTY(());

/* Another undefined case: type with malformed GTY annotation */
typedef struct {
  int x;
} /* Missing GTY here */ incomplete_type;

/* More complex nested types to ensure traversal */
struct container GTY(()) {
  /* Nested struct */
  struct nested GTY(()) {
    int id;
    my_string name;
  } item;
  
  /* Pointer member */
  opaque_ptr ptr;
  
  /* Array member */
  int dynamic_array[] GTY((length("item.id")));
  
  /* Union member */
  union my_union data;
  
  /* Callback member */
  callback_fn notify;
};

/* Template-like structure for additional coverage */
template<typename T>
struct generic_container GTY(()) {
  T* data GTY((skip));
  int size;
};

/* Explicit instantiation for gengtype */
typedef generic_container<struct my_struct> my_container GTY(());

/* Enum type (should be treated as scalar for counting) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Bitmask type */
typedef unsigned int bitmask GTY(());

/* Self-referential structure */
struct node GTY(()) {
  int value;
  struct node *next GTY((skip));
  struct node *prev GTY((skip));
};

/* Variadic structure */
struct variadic_struct GTY(()) {
  int type;
  union {
    int int_val;
    my_string str_val;
    callback_fn func_val;
  } data;
};

/* Extern declaration to test external linkage */
extern struct my_struct global_struct GTY(());

/* Static inline function using GTY types (should be ignored) */
static inline void process_struct(struct my_struct *s GTY((skip))) {
  /* Function body doesn't matter for gengtype */
}

#endif /* TEST_GTYPE_H */
