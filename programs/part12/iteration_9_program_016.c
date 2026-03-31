/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_undefined;
union opaque_union_undefined;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) simple_struct {
  int a;
  char b;
};

struct GTY(()) nested_struct {
  struct simple_struct inner;
  long extra;
};

/* Struct with pointer chain for GC */
struct GTY((chain_next ("%h.next"))) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
};

/* TYPE_USER_STRUCT: Struct with user-defined behavior */
typedef struct GTY((user)) user_defined_struct {
  int custom_field;
  void * GTY((skip)) user_data;
} user_struct_t;

/* TYPE_UNION: Various union types */
union GTY(()) simple_union {
  int i;
  float f;
  double d;
  void *p;
};

/* Union within struct */
struct GTY(()) struct_with_union {
  int type;
  union GTY((desc ("%0.type"))) {
    int ival;
    float fval;
    char * GTY((tag ("1"))) sval;
  } value;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef const char * GTY(()) const_string_ptr;
typedef void (* GTY(()) void_func_ptr)(void);

/* Pointer to incomplete type */
struct forward_declared;
typedef struct forward_declared * GTY(()) forward_ptr;

/* TYPE_ARRAY: Various array types */
extern int GTY(()) external_array[];
static int GTY(()) fixed_array[10] = {0};
typedef int GTY(()) int_array_5[5];

/* Array of pointers */
struct GTY(()) array_container {
  struct simple_struct * GTY((length ("%h.count"))) items[20];
  int count;
};

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) my_int;
typedef unsigned long GTY(()) my_ulong;
typedef _Bool GTY(()) my_bool;

/* Enum type */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

/* TYPE_STRING: String types */
const char GTY(()) *global_string = "Hello, gengtype!";
static char GTY(()) local_string[] = "Test string";

/* TYPE_CALLBACK: Function pointer types */
typedef int (* GTY((callback)) compare_func)(const void *, const void *);

/* Callback with parameters */
typedef void (* GTY((callback)) traverse_func)(void *data, int depth);

/* TYPE_LANG_STRUCT: GCC-specific internal types */
/* Vector type using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node *chain;
};

struct GTY(()) tree_node {
  struct tree_common common;
  /* Various tree fields would go here */
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_container {
  /* Struct field */
  struct simple_struct base;
  
  /* Union field */
  union simple_union data;
  
  /* Pointer field */
  struct complex_container * GTY((skip)) sibling;
  
  /* Array field */
  int GTY(()) numbers[8];
  
  /* Pointer to array */
  int (* GTY(()) matrix)[4];
  
  /* Callback field */
  compare_func comparator;
  
  /* String field */
  const char * GTY(()) name;
  
  /* Nested struct with union */
  struct struct_with_union variant;
};

/* Another level of nesting */
struct GTY(()) outer_container {
  struct complex_container items[3];
  union simple_union optional;
  color_t color;
};

/* Function pointer returning struct */
struct simple_struct (* GTY(()) struct_maker)(int);

/* Array of function pointers */
typedef void (* GTY(()) action_func)(void);
action_func GTY(()) actions[] = { NULL, NULL };

/* Union with struct members */
union GTY(()) mixed_union {
  struct simple_struct as_struct;
  struct complex_container *as_ptr;
  int as_array[4];
};

/* Template for generating multiple similar types */
#define DECLARE_NUMBERED_STRUCT(n) \
  struct GTY(()) numbered_struct_##n { \
    int id; \
    char name[32]; \
    struct numbered_struct_##n *next; \
  }

DECLARE_NUMBERED_STRUCT(1);
DECLARE_NUMBERED_STRUCT(2);
DECLARE_NUMBERED_STRUCT(3);

/* Global variables with various types */
struct simple_struct GTY(()) global_struct = {1, 'A'};
union simple_union GTY(()) global_union = {.i = 42};
int_ptr GTY(()) global_ptr = NULL;
color_t GTY(()) global_color = GREEN;

/* Inline struct definition */
struct GTY(()) {
  int x;
  int y;
} anonymous_point = {0, 0};

/* Typedef chain leading to scalar */
typedef int GTY(()) base_int;
typedef base_int GTY(()) derived_int;
typedef derived_int GTY(()) final_int;

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  unsigned int padding : 26;
};

/* Union with bitfields */
union GTY(()) bitfield_union {
  struct {
    unsigned int low : 16;
    unsigned int high : 16;
  } parts;
  unsigned int whole;
};

/* Self-referential types */
struct GTY(()) tree_node_2 {
  int value;
  struct tree_node_2 * GTY((skip)) left;
  struct tree_node_2 * GTY((skip)) right;
  struct tree_node_2 * GTY((skip)) parent;
};

/* Array of unions */
union GTY(()) choice {
  int i;
  float f;
  char c;
};

struct GTY(()) union_array_container {
  union choice choices[10];
  int selected[10];
};

/* Function returning pointer to function */
typedef int (* GTY(()) int_func)(void);
int_func (* GTY(()) get_func_factory(int type))(void);

/* Const volatile qualified types */
typedef const volatile int GTY(()) cv_int;
typedef struct simple_struct * const GTY(()) const_struct_ptr;

/* Complete the forward declaration */
struct forward_declared {
  int data;
  struct forward_declared *next;
};

/* Additional test structures for edge cases */

/* Empty struct */
struct GTY(()) empty_struct {
  /* intentionally empty */
};

/* Struct with only arrays */
struct GTY(()) array_only {
  int matrix[3][3];
  char strings[5][50];
};

/* Union with nested struct */
union GTY(()) deep_union {
  struct {
    struct simple_struct inner;
    int extra;
  } nested;
  long long big_value;
};

/* Pointer to const struct */
typedef const struct simple_struct * GTY(()) const_simple_ptr;

/* Volatile pointer */
typedef volatile int * GTY(()) volatile_int_ptr;

/* Struct with all basic types */
struct GTY(()) all_types_struct {
  char c;
  signed char sc;
  unsigned char uc;
  short s;
  unsigned short us;
  int i;
  unsigned int ui;
  long l;
  unsigned long ul;
  long long ll;
  unsigned long long ull;
  float f;
  double d;
  long double ld;
  _Bool b;
  void *vp;
  const char *cp;
  volatile int *vip;
};

/* End of type definitions */
