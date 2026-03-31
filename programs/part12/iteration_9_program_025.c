/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) simple_struct {
  int a;
  char b;
};

struct GTY(()) nested_struct {
  struct simple_struct GTY((tag("0"))) simple;
  long double ld;
};

/* Struct with chain_next for GC */
struct GTY((chain_next("%h.next"))) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
};

/* TYPE_UNION: Union types */
union GTY(()) test_union {
  int i;
  float f;
  double d;
  void * GTY((skip)) p;
};

/* Union within struct */
struct GTY(()) struct_with_union {
  int type;
  union GTY((desc("%0.type"))) {
    int int_val;
    float float_val;
    char * GTY((length("%h.str_len"))) str_val;
  } GTY((tag("0.type"))) value;
  size_t str_len;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void (* GTY(()) void_func_ptr)(void);
typedef struct simple_struct * GTY(()) struct_ptr;

/* Pointer to array */
typedef int (* GTY(())) array_ptr[10];

/* TYPE_ARRAY: Array types */
int GTY(()) fixed_array[100];
extern int GTY(()) incomplete_array[];

/* Array of pointers */
struct GTY(()) array_of_ptrs {
  void * GTY((skip)) ptrs[20];
};

/* Variable length array in struct */
struct GTY(()) var_struct {
  int length;
  int GTY((length("%h.length"))) data[];
};

/* TYPE_SCALAR: Fundamental scalar types */
typedef char GTY(()) byte_type;
typedef short GTY(()) short_type;
typedef int GTY(()) int_type;
typedef long GTY(()) long_type;
typedef long long GTY(()) long_long_type;
typedef float GTY(()) float_type;
typedef double GTY(()) double_type;
typedef _Bool GTY(()) bool_type;

/* Enum type */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_type;

/* TYPE_STRING: String types */
const char GTY(()) *const_string = "Hello, gengtype!";
char GTY(()) mutable_string[] = "Test string";
char GTY(()) * GTY((skip)) string_ptr = "Pointer to string literal";

/* TYPE_CALLBACK: Function pointer types */
typedef int (* GTY((callback))) compare_func(const void *, const void *);
typedef void (* GTY((callback))) cleanup_func(void *);

/* Callback with parameters */
struct GTY(()) callback_container {
  compare_func * GTY((skip)) compare;
  cleanup_func * GTY((skip)) cleanup;
};

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
/* Using GCC vector extension to trigger special handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) vector_struct {
  v4si vectors[4];
  int count;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* Mimicking tree-like structures from GCC internals */
struct GTY(()) tree_common {
  int code;
  union tree_node * GTY((skip)) chain;
  union tree_node * GTY((skip)) type;
};

struct GTY(()) tree_int_cst {
  struct tree_common common;
  HOST_WIDE_INT int_cst;
};

union GTY((desc ("%h.common.code"))) tree_node {
  struct tree_common GTY((tag ("0"))) common;
  struct tree_int_cst GTY((tag ("1"))) int_cst;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) master_container {
  /* TYPE_STRUCT */
  struct nested_struct nested;
  
  /* TYPE_UNION */
  union test_union u;
  
  /* TYPE_POINTER */
  struct_ptr sp;
  
  /* TYPE_ARRAY */
  int GTY(()) matrix[3][3];
  
  /* TYPE_SCALAR */
  color_type color;
  
  /* TYPE_STRING */
  const char * GTY((skip)) name;
  
  /* TYPE_CALLBACK */
  compare_func * GTY((skip)) comparator;
  
  /* TYPE_USER_STRUCT */
  v4si vector;
  
  /* TYPE_LANG_STRUCT */
  union tree_node * GTY((skip)) tree;
  
  /* Recursive pointer */
  struct master_container * GTY((skip)) next;
};

/* Function pointer returning struct */
struct simple_struct (* GTY((callback))) func_returning_struct(int);

/* Array of function pointers */
typedef void (* GTY((callback))) func_array[5];

/* Union with struct and array */
union GTY(()) complex_union {
  struct {
    int x;
    int y;
  } point;
  int coords[2];
  struct master_container * GTY((skip)) container;
};

/* Incomplete array of structs */
extern struct simple_struct GTY(()) extern_struct_array[];

/* Pointer to incomplete array */
typedef int (* GTY(())) incomplete_array_ptr[];

/* Nested pointer types */
typedef int *** GTY(())) triple_ptr;

/* Struct with bitfields (scalar special case) */
struct GTY(()) bitfield_struct {
  unsigned int flag1:1;
  unsigned int flag2:2;
  unsigned int flag3:3;
  unsigned int padding:26;
};

/* Anonymous struct/union */
struct GTY(()) anon_container {
  struct {
    int x;
    int y;
  } point;
  union {
    int i;
    float f;
  } value;
};

/* Typedef chain leading to scalar */
typedef int GTY(())) base_int;
typedef base_int GTY(())) level1_int;
typedef level1_int GTY(())) level2_int;
typedef level2_int GTY(())) final_int;

/* Const and volatile qualified types */
typedef const int GTY(())) const_int;
typedef volatile char GTY(())) volatile_char;
typedef const volatile long GTY(())) cv_long;

/* Struct with array of unions */
struct GTY(()) array_of_unions {
  int count;
  union test_union GTY(()) items[10];
};

/* Forward declared struct that's later defined */
struct GTY(())) forward_declared;
struct GTY(())) forward_declared {
  int data;
  struct forward_declared * GTY((skip)) next;
};

/* Self-referential struct */
struct GTY(()) self_ref {
  int value;
  struct self_ref * GTY((skip)) ref;
};

/* Multiple indirection */
struct GTY(()) indirection_test {
  void * GTY((skip)) single;
  void ** GTY((skip)) double_ptr;
  void *** GTY((skip)) triple_ptr;
};

/* Mixed declarations for maximum coverage */
static struct simple_struct GTY(()) static_struct = {1, 'A'};
static union test_union GTY(()) static_union = {.i = 42};
static int GTY(()) static_array[5] = {1, 2, 3, 4, 5};
static const char GTY(()) *static_string = "Static string";

/* Extern declarations to ensure they're processed */
extern struct GTY(()) external_struct;
extern union GTY(()) external_union;
extern int GTY(()) external_array[];
