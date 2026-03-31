/* test-gtype.h - Comprehensive type definitions for gengtype coverage testing */

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

/* TYPE_USER_STRUCT: User-defined structure 
   This becomes a user struct when defined in client code */
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

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_type;

/* TYPE_POINTER: Pointer to incomplete type */
typedef struct undefined_type *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array type with length attribute */
typedef int flexible_array[] GTY((length("0")));

/* Another array type */
typedef struct my_struct struct_array[10] GTY(());

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
  int lang_data;
  callback_fn handler;
};

/* More complex nested types to ensure traversal */
struct container GTY(()) {
  /* TYPE_POINTER member */
  opaque_ptr unknown GTY((skip));
  
  /* TYPE_STRING member */
  my_string name GTY((string));
  
  /* TYPE_ARRAY member */
  flexible_array data GTY((length("data_len")));
  
  /* TYPE_CALLBACK member */
  callback_fn notify GTY((callback));
  
  /* TYPE_UNION member */
  union my_union value GTY(());
  
  /* TYPE_STRUCT member */
  struct my_struct nested GTY(());
  
  /* TYPE_USER_STRUCT member */
  struct my_user_struct user GTY((user));
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_struct *lang GTY((tag("LANG")));
  
  int data_len;
};

/* Additional undefined type usage for TYPE_UNDEFINED */
extern struct undefined_type *global_undefined GTY((ptr));

/* Template-like macro to generate more types */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct name##_s GTY(()) { \
    field_type field; \
    struct name##_s *next GTY((skip)); \
  }

/* Generate more struct types */
DEFINE_GTY_STRUCT(list, int);
DEFINE_GTY_STRUCT(tree, struct my_struct *);

/* Enumeration type (should be treated as scalar) */
typedef enum {
  MODE_A,
  MODE_B,
  MODE_C
} mode_type GTY(());

/* Function declarations that use GTY types */
void process_struct(struct my_struct *obj GTY((skip)));
callback_fn get_callback(void) GTY((callback));

/* Inline structure definition */
struct GTY(()) inline_struct {
  mode_type mode;
  union {
    int num;
    my_string str;
  } GTY((desc("mode == MODE_A ? 0 : 1"))) value;
};

#endif /* TEST_GTYPE_H */
