/* Test types for gengtype coverage - covering all type_kind cases */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String typedefs */
typedef const char *my_string;
typedef char *my_mutable_string;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) my_struct {
  my_scalar field1;
  my_string field2;
  int *field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_struct {
  int data;
  struct linked_struct *GTY((skip)) next;
  struct linked_struct *prev;
};

struct GTY((desc("%0.kind"))) tagged_struct {
  enum { KIND_A, KIND_B } kind;
  union {
    int a;
    double b;
  } GTY((desc("%1.kind"))) value;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct {
  void *data;
  int size;
};

struct GTY((user)) another_user_struct {
  long id;
  char *name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int int_val;
  double double_val;
  char *string_val;
};

union GTY((desc("%0.type"))) tagged_union {
  int type;
  struct {
    int x, y;
  } point;
  struct {
    double radius;
  } circle;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct *my_pointer;
typedef union my_union *union_pointer;
typedef int *int_pointer;
typedef void (*func_pointer)(void);

/* TYPE_ARRAY: Array typedefs */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int, const char*);
typedef int (*compare_callback)(const void*, const void*);
typedef void (*simple_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("TREE_CODE(%h)"))) tree_lang_struct {
  enum tree_code code;
  union tree_node *GTY((skip("TREE_CODE(%h)"))) chain;
};

/* Additional types to ensure multiple instances */
struct GTY(()) extra_struct_one {
  my_array arr;
  my_callback cb;
};

struct GTY(()) extra_struct_two {
  union my_union u;
  my_pointer p;
};

union GTY(()) extra_union {
  extra_struct_one s1;
  extra_struct_two s2;
};

/* Function pointer in struct to test interaction */
struct GTY(()) callback_container {
  my_callback cb;
  compare_callback cmp;
  simple_callback simple;
};

#endif /* TEST_GENGTYPE_TYPES_H */
