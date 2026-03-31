/* test-gengtype-types.h - Test types for gengtype coverage testing */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h)"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((desc("%0"))) my_struct_2 {
  int a;
  double b;
  struct my_struct_1 *next;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_3 {
  int value;
  struct my_struct_3 *next;
  struct my_struct_3 *prev;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_1 {
  void *data;
  int size;
};

struct GTY((user)) user_struct_2 {
  long id;
  struct user_struct_1 *ref;
};

/* TYPE_UNION: Union types */
union GTY((desc("%d"))) my_union_1 {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((tag("UNION_2"))) my_union_2 {
  struct my_struct_1 s;
  struct my_struct_2 t;
  my_scalar_1 scalar;
};

union my_union_3 {
  int x;
  long y;
  struct {
    int a;
    int b;
  } nested;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY((skip)) my_pointer_2;
typedef my_scalar_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef union my_union_2 my_array_3[3][3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(struct my_struct_1 *, my_scalar_1);
typedef void (* GTY((skip)) my_callback_3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_specific;
  struct lang_struct_type_1 *next;
  struct my_struct_1 *data;
};

struct GTY((desc("%0"), tag("LANG_STRUCT_2"))) lang_struct_type_2 {
  long id;
  my_string_1 name;
  union my_union_1 value;
};

struct GTY((desc("%d"), chain_next="%h.link")) lang_struct_type_3 {
  int type;
  void *payload;
  struct lang_struct_type_3 *link;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_nested {
  struct my_struct_1 part1;
  union my_union_2 part2;
  my_array_1 numbers;
  my_callback_1 callback;
};

union GTY((desc("%d"))) complex_union {
  struct complex_nested nested;
  struct lang_struct_type_1 lang;
  my_pointer_1 ptr;
};

/* Pointer to undefined type (should still be processed) */
struct undefined_type_1 *undefined_ptr_1;
struct undefined_type_2 * GTY((skip)) undefined_ptr_2;

#endif /* TEST_GENGTYPE_TYPES_H */
