/* test-gtype-coverage.h - Comprehensive type declarations for gengtype coverage
   This file should be placed in the gcc/ directory and included in the build
   to ensure all type serialization cases are exercised. */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_forward_decl;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) base_struct {
  int scalar_field;
  char *string_field;
};

/* Struct with nested pointers and arrays */
struct GTY(()) complex_struct {
  int id;
  char name[32];  /* Fixed-size array */
  struct base_struct *GTY((skip)) ptr_to_struct;
  void *GTY((skip)) generic_ptr;
  struct complex_struct *next;  /* Recursive pointer */
};

/* Struct with union field */
struct GTY(()) struct_with_union {
  int type_tag;
  union {
    int int_val;
    float float_val;
    char *string_val;
  } GTY((desc("%0.type_tag"))) data;
};

/* TYPE_UNION: Various union types */
union GTY(()) simple_union {
  int i;
  float f;
  double d;
  void *p;
};

/* Tagged union for garbage collection */
union GTY((desc("%0.kind"))) tagged_union {
  struct {
    int kind;
    union {
      int int_member;
      struct base_struct *struct_ptr;
    } GTY((tag("0.kind"))) value;
  } GTY((skip)) header;
  long long_data;
};

/* TYPE_POINTER: Various pointer types */
typedef int *GTY((skip)) int_ptr;
typedef void *GTY((skip)) void_ptr;
typedef struct base_struct *GTY((skip)) struct_ptr;
typedef const char *GTY((skip)) const_string_ptr;

/* Function pointer typedef */
typedef int (*GTY((callback)) compare_func)(const void *, const void *);

/* TYPE_ARRAY: Various array declarations */
extern int GTY((skip)) external_array[];
static int GTY((skip)) static_array[100];
const char GTY((skip)) const_string_array[] = "Hello, World!";

/* Array of pointers */
struct base_struct *GTY((skip)) struct_ptr_array[10];

/* Variable length array in struct (GCC extension) */
struct GTY(()) varray_struct {
  int length;
  int data[];  /* Flexible array member */
};

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef unsigned long long uint64;
typedef _Bool bool_type;

enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

enum GTY(()) flags {
  FLAG_A = 1 << 0,
  FLAG_B = 1 << 1,
  FLAG_C = 1 << 2
};

/* TYPE_STRING: String type handling */
const char GTY((skip)) *global_string = "Global string constant";
static const char GTY((skip)) *static_string = "Static string";

struct GTY(()) string_container {
  const char *GTY((skip)) message;
  char buffer[256];
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*GTY((callback)) simple_callback)(void);
typedef int (*GTY((callback)) complex_callback)(struct base_struct *, int);

/* Callback with parameters */
struct GTY(()) callback_container {
  compare_func GTY((skip)) comparator;
  simple_callback GTY((skip)) notify;
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */

/* Vector type (GCC extension) */
typedef int GTY((skip)) v4si __attribute__((vector_size(16)));

/* Simulating tree-like structure (common in GCC) */
struct GTY(()) tree_common {
  int code;
  union tree_node *chain;
  union tree_node *type;
};

struct GTY(()) tree_int_cst {
  struct tree_common common;
  HOST_WIDE_INT int_cst;
};

/* Union tree_node definition */
union GTY((desc ("((enum tree_code) (%h.common.code))"))) tree_node {
  struct tree_common GTY((skip)) common;
  struct tree_int_cst GTY((tag ("INTEGER_CST"))) int_cst;
};

/* Simulating RTL-like structure */
struct GTY(()) rtx_def {
  int code;
  int mode;
  union {
    HOST_WIDE_INT int_val;
    struct base_struct *struct_val;
    char *string_val;
  } GTY((desc ("%0.code"))) u;
};

typedef struct rtx_def *rtx;

/* More complex type relationships */

/* Struct containing array of function pointers */
struct GTY(()) callback_dispatcher {
  int count;
  simple_callback GTY((skip)) callbacks[5];
};

/* Nested struct with multiple indirections */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int value;
    struct outer_struct *parent;
  } inner;
  
  union GTY((desc ("%0.inner.value"))) inner_union {
    int as_int;
    float as_float;
  } data;
  
  struct callback_container *GTY((skip)) callbacks;
};

/* Chain of structures for testing chain_next */
struct GTY((chain_next ("%h.next"))) chainable_struct {
  int id;
  char *name;
  struct chainable_struct *next;
};

/* Self-referential structure */
struct GTY(()) self_ref {
  int data;
  struct self_ref *GTY((skip)) pointer_to_self;
  struct self_ref *GTY((skip)) array_of_self[5];
};

/* Typedef chain leading to scalar */
typedef int my_int;
typedef my_int my_int2;
typedef my_int2 final_int;

/* Structure with bitfields */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Opaque pointer type */
typedef struct opaque_forward_decl *GTY((skip)) opaque_ptr;

/* Complete the forward declaration */
struct opaque_forward_decl {
  int revealed;
  opaque_ptr next;  /* Pointer to same opaque type */
};

/* Global variables with various types for gengtype to process */
extern struct base_struct GTY((skip)) *global_struct_ptr;
extern union simple_union GTY((skip)) global_union;
extern int GTY((skip)) global_int_array[50];
extern compare_func GTY((skip)) global_comparator;

/* Inline function using the types (not processed by gengtype but valid C) */
static inline void use_types(void) {
  struct complex_struct cs = {0};
  union tagged_union tu;
  int_ptr ip = NULL;
  
  (void)cs;
  (void)tu;
  (void)ip;
}

#endif /* TEST_GTYPE_COVERAGE_H */
