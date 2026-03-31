/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Plain C structure */
struct my_struct GTY(()) {
  int a;
  my_scalar b;
};

/* TYPE_USER_STRUCT: User-defined structure 
   This becomes a user struct when defined in separate compilation unit */
struct user_def GTY((user)) {
  int user_data;
  my_string name;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
  int i;
  void *p;
  struct my_struct *s;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_type;

/* TYPE_POINTER: Pointer to incomplete type */
typedef struct undefined_type *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array type with length specifier */
typedef int flexible_array[] GTY((length("array_length")));
extern int array_length;

/* Multi-dimensional array */
typedef int matrix_type[10][20] GTY(());

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_struct GTY((tag("LANG_SPECIFIC"))) {
  int lang_data;
  callback_fn handler;
};

/* Another TYPE_STRUCT with nested types */
struct container GTY(()) {
  struct my_struct embedded;
  union my_union choice;
  opaque_ptr unknown;
  flexible_array flex;
  struct lang_struct *lang_ptr;
};

/* TYPE_UNDEFINED: Incomplete struct definition (forward declared) */
struct undefined_type GTY(()) {
  /* No definition here - this makes it undefined during parsing */
};

/* Enum type (treated as scalar for counting) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Function pointer in struct (callback in context) */
struct with_callback GTY(()) {
  int id;
  callback_fn notify;
};

/* Variable length array in struct */
struct with_vla GTY(()) {
  int count;
  int data[] GTY((length("count")));
};

/* Chain of pointers for testing */
typedef struct chain_node GTY(()) {
  int value;
  struct chain_node *next GTY((skip));
} chain_node;

/* Self-referential structure */
struct self_ref GTY(()) {
  int data;
  struct self_ref *next;
  struct self_ref *prev;
};

/* Union with struct members */
union complex_union GTY(()) {
  struct {
    int type;
    void *data;
  } s;
  struct {
    float x, y;
  } point;
};

#endif /* TEST_GTYPE_H */
