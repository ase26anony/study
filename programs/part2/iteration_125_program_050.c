/* Test types for gengtype coverage testing.
   This file defines types corresponding to all type_kind enum cases
   to ensure full coverage of the switch statement in gengtype.cc */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;
typedef double my_double_scalar;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef char *my_mutable_string GTY(());
typedef const char * const my_const_string_ptr;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) my_struct {
  my_scalar field1;
  my_string field2;
  int *field3 GTY((skip));
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_struct {
  int data;
  struct linked_struct *next;
  struct linked_struct *prev;
};

struct GTY((desc ("%1.type"))) variant_struct {
  enum { TYPE_A, TYPE_B } type;
  union {
    int a_value;
    double b_value;
  } GTY((desc ("%0.type"))) u;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct {
  void *opaque_data;
  int user_id;
};

struct GTY((user)) another_user_struct {
  long custom_field;
  void *user_pointer;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int int_val;
  double double_val;
  my_string string_val;
};

union GTY((desc ("%0.tag"))) tagged_union {
  int tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    int width;
    int height;
  } rect;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct *my_pointer;
typedef my_union *union_pointer GTY(());
typedef const user_struct *const_user_struct_ptr;
typedef int *int_ptr GTY((skip));

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef my_struct struct_array[5] GTY(());
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int, const char*);
typedef int (*comparator)(const void *, const void *) GTY(());
typedef void (*simple_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("%0.kind"), tag("kind"))) lang_variant {
  enum lang_kind { LANG_A, LANG_B, LANG_C } kind;
  union {
    int a_data;
    double b_data;
    const char *c_data;
  } GTY((desc ("%0.kind"))) u;
};

/* Additional complex types to ensure thorough testing */
struct GTY(()) container_struct {
  my_array array_field;
  my_pointer ptr_field;
  my_callback callback_field;
  union GTY(()) {
    int option_a;
    my_string option_b;
  } choice;
};

/* Nested struct with various type combinations */
struct GTY(()) nested_types {
  /* Contains one of each type kind */
  my_scalar scalar_field;          /* TYPE_SCALAR */
  my_string string_field;          /* TYPE_STRING */
  struct my_struct struct_field;   /* TYPE_STRUCT */
  union my_union union_field;      /* TYPE_UNION */
  my_pointer pointer_field;        /* TYPE_POINTER */
  my_array array_field;            /* TYPE_ARRAY */
  my_callback callback_field;      /* TYPE_CALLBACK */
  struct lang_struct_type lang_field; /* TYPE_LANG_STRUCT */
};

/* Forward declarations to test TYPE_UNDEFINED */
struct forward_declared_struct;
union forward_declared_union;

/* Later definition of forward declared struct */
struct GTY(()) forward_declared_struct {
  int defined_now;
  struct forward_declared_struct *self_ptr;
};

/* Template-like structure for comprehensive coverage */
#define DECLARE_GTY_STRUCT(name, field_type) \
  struct GTY(()) name { \
    field_type data; \
    struct name *next; \
  }

DECLARE_GTY_STRUCT(int_list, int);
DECLARE_GTY_STRUCT(string_list, const char*);

/* Enum type (treated as scalar by gengtype) */
typedef enum {
  STATE_INIT,
  STATE_PROCESSING,
  STATE_DONE
} process_state;

/* More pointer variations */
typedef void (*void_callback)(void);
typedef struct my_struct *(*struct_factory)(int);
typedef union my_union (*union_accessor)(void);

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
