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
/* This is typically a structure from client/plugin code */
#define USER_STRUCT_MARKER 1
struct user_defined_struct GTY((user)) {
  int user_data;
  struct my_struct *link GTY((skip));
};

/* TYPE_UNION: Union type marked with GTY */
union my_union GTY(()) {
  int i;
  void *p GTY((ptr));
  my_string s;
};

/* Forward declaration for TYPE_UNDEFINED */
struct incomplete_struct;
typedef struct incomplete_struct *undefined_ptr GTY(());

/* TYPE_POINTER: Pointer to incomplete type */
typedef struct unknown *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array type with length specifier */
typedef int flexible_array[] GTY((length("0")));

struct array_container GTY(()) {
  int count;
  flexible_array data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

struct callback_container GTY(()) {
  callback_fn handler;
  int id;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to mark as language-specific */
struct lang_struct GTY((tag("LANG"))) {
  int lang_data;
  enum { LANG_A, LANG_B } lang_type;
};

/* Additional complex types to ensure traversal */

/* Nested structure with pointer */
struct nested_container GTY(()) {
  struct my_struct inner GTY(());
  opaque_ptr ptr;
  union my_union u;
};

/* Self-referential structure */
struct tree_node GTY(()) {
  int value;
  struct tree_node *left GTY((ptr));
  struct tree_node *right GTY((ptr));
};

/* Template-like structure with conditional fields */
#ifdef SPECIAL_FEATURE
struct feature_struct GTY(()) {
  int special_flag;
  callback_fn special_handler;
};
#endif

/* Union with struct members */
union complex_union GTY(()) {
  struct {
    int type;
    void *data GTY((ptr));
  } s;
  struct lang_struct lang;
  callback_fn cb;
};

/* Array of pointers */
typedef struct my_struct *struct_ptr_array[10] GTY(());

/* String array */
typedef const char *string_array[] GTY((length("str_count")));

/* Enumeration type */
typedef enum {
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE
} color_type GTY(());

/* Structure with bitfields */
struct bitfield_struct GTY(()) {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int value;
};

/* Opaque callback type for TYPE_CALLBACK */
typedef void (*opaque_callback)(void) GTY((callback));

/* Multiple levels of indirection */
typedef struct my_struct ***triple_ptr GTY((ptr));

/* Structure with array of strings */
struct string_list GTY(()) {
  int count;
  const char **strings GTY((length("count")));
};

/* Union containing array */
union array_union GTY(()) {
  int ints[4];
  char chars[16];
  struct my_struct structs[2];
};

/* Forward declared structure that will be TYPE_UNDEFINED */
struct forward_declared;

/* Pointer to forward declared (will be TYPE_POINTER) */
typedef struct forward_declared *forward_ptr GTY((ptr));

/* Another undefined type usage */
extern struct undefined_extern undefined_var GTY(());

/* Complex nested type definition */
typedef struct {
  struct {
    int depth;
    struct my_struct *item GTY((ptr));
  } level1;
  union {
    int as_int;
    struct lang_struct *as_lang GTY((ptr));
  } level2;
} deeply_nested GTY(());

#endif /* TEST_GTYPE_H */
