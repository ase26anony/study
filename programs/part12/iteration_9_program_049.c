/* test-gtype-coverage.h - Comprehensive type declarations for gengtype coverage */
/* This file should be placed in gcc/ directory and included in gcc/gtype-desc.c */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef long GTY(()) scalar_long;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRING: String types */
typedef const char * GTY(()) string_ptr;
static const char GTY(()) test_string[] = "Hello, gengtype!";

/* TYPE_POINTER: Various pointer types */
typedef void * GTY(()) void_ptr;
typedef int * GTY(()) int_ptr;
typedef struct my_struct * GTY(()) struct_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_func)(const void *, const void *);
typedef void GTY((callback)) (*void_callback)(void);

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
extern int GTY(()) incomplete_array[];
typedef struct my_struct * GTY(()) ptr_array[5];

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
  int GTY(()) id;
  char * GTY(()) name;
  struct my_struct * GTY(()) next;
  int GTY(()) values[5];
  union my_union * GTY(()) union_ptr;
};

struct GTY(()) nested_struct {
  struct GTY(()) inner {
    int GTY(()) x;
    int GTY(()) y;
  } GTY(()) point;
  struct my_struct GTY(()) data;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int GTY(()) i;
  float GTY(()) f;
  double GTY(()) d;
  void * GTY(()) p;
  struct my_struct * GTY(()) s;
};

/* TYPE_USER_STRUCT: Struct with user-defined behavior */
struct GTY((user)) user_struct {
  int GTY(()) tag;
  void * GTY(()) data;
  union my_union GTY(()) value;
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific struct types */
/* Using GCC vector extension to trigger lang_struct handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) lang_struct {
  v4si GTY(()) vector_data;
  int GTY(()) mode;
  struct lang_struct * GTY(()) chain;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_type {
  /* Scalar */
  int GTY(()) count;
  
  /* String */
  const char * GTY(()) message;
  
  /* Pointer */
  struct complex_type * GTY(()) self_ptr;
  
  /* Array */
  union my_union GTY(()) unions[3];
  
  /* Struct */
  struct nested_struct GTY(()) nested;
  
  /* Union */
  union my_union GTY(()) variant;
  
  /* Callback */
  compare_func GTY(()) comparator;
  
  /* Pointer to array */
  int (* GTY(()) matrix_ptr)[4][4];
  
  /* Nested anonymous struct */
  struct GTY(()) {
    int GTY(()) a;
    int GTY(()) b;
  } GTY(()) anonymous;
};

/* Recursive type structure */
struct GTY((chain_next("%h.next"))) recursive_struct {
  int GTY(()) value;
  struct recursive_struct * GTY(()) next;
  struct recursive_struct * GTY(()) prev;
};

/* Union with struct fields */
union GTY(()) complex_union {
  struct GTY(()) {
    int GTY(()) type;
    void * GTY(()) data;
  } GTY(()) s;
  
  struct GTY(()) {
    float GTY(()) x;
    float GTY(()) y;
    float GTY(()) z;
  } GTY(()) point;
  
  compare_func GTY(()) callback;
};

/* Array of pointers to different types */
typedef union GTY(()) {
  int GTY(()) i;
  float GTY(()) f;
  struct my_struct * GTY(()) s;
} GTY(()) variant;

variant GTY(()) variant_array[10];

/* Function pointer with complex return type */
struct my_struct * GTY((callback)) (*complex_callback)(int, union my_union *);

/* Typedef chain leading to scalar */
typedef int GTY(()) base_int;
typedef base_int GTY(()) level1_int;
typedef level1_int GTY(()) level2_int;
typedef level2_int GTY(()) final_int;

/* Struct with bitfields (scalar handling) */
struct GTY(()) bitfield_struct {
  unsigned int GTY(()) flag1 : 1;
  unsigned int GTY(()) flag2 : 2;
  unsigned int GTY(()) value : 8;
  unsigned int GTY(()) : 5; /* padding */
  unsigned int GTY(()) last : 16;
};

/* Opaque pointer typedef */
typedef struct opaque_struct * GTY(()) opaque_ptr;

/* Self-referential union */
union GTY(()) self_ref_union {
  int GTY(()) value;
  union self_ref_union * GTY(()) next;
};

/* Complete the forward declarations */
struct GTY(()) opaque_struct {
  int GTY(()) dummy;
};

union GTY(()) opaque_union {
  int GTY(()) i;
  struct opaque_struct * GTY(()) s;
};

#endif /* TEST_GTYPE_COVERAGE_H */
