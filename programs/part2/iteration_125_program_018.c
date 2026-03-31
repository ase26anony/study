/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch branches in gengtype.cc are executed. */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2;
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) test_struct_1 {
  my_scalar_1 field1;
  my_string_1 field2;
  struct test_struct_1 *next;
};

struct GTY((chain_next("%h.next"))) test_struct_2 {
  int id;
  const char *name;
  struct test_struct_2 *next;
};

struct GTY((skip)) test_struct_3 {
  double value;
  int count;
  void *data;
};

/* TYPE_USER_STRUCT: Structs with user tag */
struct GTY((user)) user_struct_1 {
  int user_data;
  void *user_ptr;
};

struct GTY((user)) user_struct_2 {
  long tag;
  const char *description;
};

/* TYPE_UNION: Union types */
union GTY(()) test_union_1 {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((desc("%0.as_int"))) test_union_2 {
  int type;
  struct test_struct_1 *s;
  union test_union_1 *u;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct test_struct_1 *my_pointer_1;
typedef union test_union_1 *my_pointer_2;
typedef my_scalar_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct test_struct_1 my_array_2[5];
typedef const char *my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, void *);
typedef struct test_struct_1 *(*my_callback_3)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_id;
  const char *lang_name;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("0"), tag("1"))) lang_struct_type_2 {
  int tag;
  union {
    int as_int;
    double as_double;
  } GTY((desc("%1.tag"))) u;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  /* Contains multiple type kinds */
  my_scalar_1 scalar_field;          /* TYPE_SCALAR */
  my_string_1 string_field;          /* TYPE_STRING */
  struct test_struct_1 *struct_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
  my_array_1 array_field;            /* TYPE_ARRAY */
  my_callback_1 callback_field;      /* TYPE_CALLBACK */
  union test_union_1 union_field;    /* TYPE_UNION */
};

/* Nested anonymous struct/union */
struct GTY(()) nested_types {
  struct {
    int x;
    int y;
  } point;
  
  union {
    int i;
    float f;
  } data;
  
  struct nested_types *next;
};

/* Forward declarations mixed with definitions */
struct forward_declared;
struct GTY(()) uses_forward {
  struct forward_declared *fd;
  int count;
};

struct forward_declared {
  int value;
  struct uses_forward *uf;
};

#endif /* TEST_GENGTYPE_TYPES_H */
