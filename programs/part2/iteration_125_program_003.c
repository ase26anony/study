/* Test types for gengtype coverage - covering all type_kind enum cases */

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
typedef const char * const my_const_string;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  my_string field3;
};

struct GTY(()) another_struct {
  double d;
  float f;
  char c;
};

struct GTY(()) complex_struct {
  struct my_struct *next;
  int data[5];
  union my_union *u_ptr;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) user_struct_type {
  void *data;
  int tag;
};

struct GTY((user)) another_user_struct {
  long custom_data;
  void *opaque;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int i;
  float f;
  double d;
  void *p;
};

union GTY(()) another_union {
  long l;
  char *str;
  struct my_struct s;
};

union GTY((desc("%0.tag"))) tagged_union {
  int tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    char *name;
    int age;
  } person;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *my_pointer;
typedef union my_union *union_pointer;
typedef my_scalar *scalar_pointer;
typedef int (*func_pointer)(int, int);

/* TYPE_ARRAY: Array typedefs */
typedef int my_array[10];
typedef char char_array[256];
typedef struct my_struct struct_array[5];
typedef const char *string_array[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int, const char*);
typedef int (*int_callback)(void);
typedef void (*simple_callback)(void);
typedef struct my_struct* (*struct_callback)(int);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
  void *data;
};

struct GTY((desc("%0.kind"), tag("0"), chain_next="%h.next")) another_lang_struct {
  enum { KIND_A, KIND_B, KIND_C } kind;
  struct another_lang_struct *next;
  union {
    int i;
    double d;
  } value;
};

/* Additional types with various GTY annotations */
struct GTY((skip)) skipped_struct {
  int should_be_ignored;
  void *opaque_ptr;
};

struct GTY((length("%0.count"))) variable_length_struct {
  int count;
  int data[1];
};

/* Nested types to ensure deep traversal */
struct GTY(()) container_struct {
  struct my_struct inner;
  union my_union choice;
  my_array numbers;
  my_callback handler;
  struct lang_struct_type *lang_chain;
};

#endif /* TEST_GENGTYPE_TYPES_H */
