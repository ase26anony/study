/* Test header for gengtype coverage - defines all TYPE_* categories */

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

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct GTY(());
typedef struct undefined_struct *undefined_ptr;

/* TYPE_UNION: Union type marked with GTY */
union my_union GTY(()) {
  int i;
  void *p;
  my_string s;
};

/* TYPE_POINTER: Pointer type with ptr option */
typedef struct my_struct *struct_ptr GTY((ptr));

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
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_struct GTY((tag("LANG"))) {
  int lang_data;
  struct lang_struct *next;
};

/* Complex nested structure to ensure traversal */
struct complex_struct GTY(()) {
  my_scalar scalar_field;
  my_string string_field;
  struct my_struct *struct_ptr_field;
  union my_union union_field;
  flexible_array *array_field;
  callback_fn callback_field;
  struct lang_struct *lang_field;
  
  /* Self-referential pointer */
  struct complex_struct *next GTY((skip));
  
  /* Pointer chain */
  undefined_ptr undefined_field;
};

/* Another structure for TYPE_USER_STRUCT context */
struct user_defined_base GTY(()) {
  int user_id;
};

/* TYPE_USER_STRUCT: This will be recognized as user struct 
   when processed from a plugin or separate module */
#ifdef USER_STRUCT_MODULE
struct user_struct GTY(()) {
  struct user_defined_base *base;
  int user_data;
};
#endif

/* Incomplete type usage for TYPE_UNDEFINED */
extern struct incomplete_type *global_incomplete_ptr;

/* Template-like macro to generate more types */
#define DEFINE_GTY_TYPE(name, base) \
  typedef base name##_t GTY(()); \
  struct name##_container GTY(()) { \
    name##_t value; \
    struct name##_container *next; \
  }

DEFINE_GTY_TYPE(double_wrapper, double);
DEFINE_GTY_TYPE(float_wrapper, float);

/* Enumeration type */
typedef enum {
  MODE_A,
  MODE_B,
  MODE_C
} operation_mode GTY(());

struct mode_container GTY(()) {
  operation_mode mode;
  union my_union data;
};

/* Chain of structures for depth testing */
struct chain_link GTY(()) {
  int id;
  struct chain_link *next;
  struct chain_link *prev;
};

/* Root structure that references everything */
struct root_container GTY(()) {
  my_scalar scalar_member;
  my_string string_member;
  struct my_struct struct_member;
  union my_union union_member;
  struct_ptr pointer_member;
  struct array_container array_member;
  struct callback_container callback_member;
  struct lang_struct lang_member;
  struct complex_struct complex_member;
  struct chain_link *chain_head;
  operation_mode current_mode;
  
#ifndef USER_STRUCT_MODULE
  /* This creates a forward reference that might be TYPE_UNDEFINED */
  struct user_struct *user_ref;
#endif
};

/* Global variables with GTY markup */
extern struct root_container *global_root GTY((root));
extern struct chain_link *global_chain GTY((chain));

/* Function declarations */
void process_struct(struct my_struct *s) GTY((callback));
void handle_callback(callback_fn fn);

#endif /* TEST_GTYPE_H */
