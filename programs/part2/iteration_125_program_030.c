/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch branches in gengtype.cc are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef unsigned long my_scalar_2;
typedef double my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h)+1"))) my_string_3;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_scalar_3 field1;
  my_string_2 field2;
  struct my_struct_1 * GTY((skip)) field3;
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
};

struct GTY((desc("%0.discriminator"), union_tag("true"))) my_struct_3 {
  int discriminator;
  union {
    my_scalar_1 as_scalar;
    my_string_3 as_string;
    struct my_struct_1 *as_struct;
  } GTY((desc("%1.discriminator"))) u;
};

/* TYPE_USER_STRUCT: Structs with user tag */
struct GTY((user)) my_user_struct_1 {
  void *opaque_data;
  int user_field;
};

struct GTY((user)) my_user_struct_2 {
  long custom_data;
  struct my_user_struct_1 *related;
};

/* TYPE_UNION: Union types */
union GTY((tag("UNION_1"))) my_union_1 {
  my_scalar_1 as_int;
  my_scalar_2 as_long;
  my_string_1 as_string;
};

union GTY((desc("%0.type"))) my_union_2 {
  int type;
  struct my_struct_1 as_struct;
  struct my_user_struct_1 as_user;
};

union my_union_3 {
  double as_double;
  struct my_struct_2 *as_ptr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef struct my_struct_2 * GTY((skip)) my_pointer_2;
typedef union my_union_1 *my_pointer_3;
typedef my_string_1 *my_string_pointer;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef union my_union_2 my_array_3[3];
typedef const char * GTY((length("strlen(%h[i])+1"))) string_array[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char*);
typedef int (*my_callback_2)(struct my_struct_1 *, union my_union_1);
typedef struct my_struct_2 *(*my_callback_3)(my_string_1, my_array_1);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.kind"), chain_next="%h.next")) lang_struct_type_1 {
  enum { LANG_NODE1, LANG_NODE2, LANG_NODE3 } kind;
  union {
    my_scalar_1 as_scalar;
    my_string_2 as_string;
  } u;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%0.type"), tag("LANG_STRUCT2"))) lang_struct_type_2 {
  int type;
  struct GTY((skip)) {
    int hidden_field;
    void *private_data;
  } hidden;
  my_callback_1 callback;
};

struct GTY((for_user)) lang_struct_type_3 {
  struct my_user_struct_1 *user_data;
  my_array_2 data_array;
  union my_union_3 value;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with array of pointers */
struct GTY(()) complex_nested_struct {
  my_scalar_1 id;
  my_string_3 name;
  struct my_struct_1 * GTY((length("%h.count"))) *items;
  int count;
  my_callback_2 processor;
};

/* Union with embedded struct */
union GTY((desc("%0.selector"))) complex_union {
  int selector;
  struct {
    my_scalar_2 start;
    my_scalar_2 end;
    my_string_1 label;
  } range;
  struct {
    my_array_1 values;
    int size;
  } array_data;
};

/* Struct with conditional fields */
struct GTY((desc("%1.flags"))) conditional_struct {
  unsigned int flags;
  union {
    struct my_struct_1 * GTY((tag("0"))) ptr;
    my_scalar_3 GTY((tag("1"))) value;
  } GTY((desc("%0.flags & 1"))) data;
  my_callback_3 transform;
};

/* Array of callbacks */
typedef void (*callback_array[5])(int);

/* Pointer to array */
typedef int (*pointer_to_array)[10];

/* Struct containing all type kinds */
struct GTY(()) master_test_struct {
  /* SCALAR */
  my_scalar_1 scalar_field;
  
  /* STRING */
  my_string_1 string_field;
  
  /* STRUCT */
  struct my_struct_1 nested_struct;
  
  /* USER_STRUCT */
  struct my_user_struct_1 * GTY((skip)) user_struct_ptr;
  
  /* UNION */
  union my_union_1 union_field;
  
  /* POINTER */
  my_pointer_2 pointer_field;
  
  /* ARRAY */
  my_array_2 array_field;
  
  /* CALLBACK */
  my_callback_1 callback_field;
  
  /* LANG_STRUCT */
  struct lang_struct_type_1 *lang_struct_ptr;
  
  /* Additional nested types */
  struct complex_nested_struct *complex;
  union complex_union variant;
  struct conditional_struct conditional;
  
  /* Array of strings */
  string_array strings;
  
  /* Callback array */
  callback_array callbacks;
  
  /* Pointer to array */
  pointer_to_array matrix;
};

/* Global variables for testing */
extern struct master_test_struct GTY(()) global_test_var;
extern struct lang_struct_type_2 GTY(()) *global_lang_var;
extern my_array_1 GTY(()) global_array_var;

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
