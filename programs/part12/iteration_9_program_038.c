/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Basic struct with various fields */
struct GTY(()) my_struct {
  int a;                    /* TYPE_SCALAR */
  char * GTY((skip)) b;     /* TYPE_POINTER with skip attribute */
  struct my_struct *next;   /* Recursive pointer */
  union my_union *u_ptr;    /* Pointer to union */
};

/* TYPE_USER_STRUCT: Struct with user-defined properties */
struct GTY((user)) user_struct {
  int id;
  void * GTY((tag("0"))) data;
};

/* TYPE_UNION: Basic union type */
union GTY(()) my_union {
  int i;                    /* TYPE_SCALAR */
  float f;                  /* TYPE_SCALAR */
  void *p;                  /* TYPE_POINTER */
  struct my_struct *s_ptr;  /* Pointer to struct */
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void (* GTY(()) func_ptr)(void);
typedef struct my_struct * GTY(()) struct_ptr;

/* TYPE_ARRAY: Different array types */
extern int GTY(()) incomplete_array[];
int GTY(()) fixed_array[10] = {0};
struct my_struct GTY(()) struct_array[5];

/* TYPE_LANG_STRUCT: GCC-specific internal types */
/* Vector type using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  int code;
  union tree_node *chain;
};

union GTY(()) tree_node {
  struct tree_common common;
  /* Add more variants as needed */
};

/* TYPE_SCALAR: Various scalar types */
typedef enum { RED, GREEN, BLUE } GTY(()) color;
typedef _Bool GTY(()) boolean;
typedef long GTY(()) my_long;
typedef unsigned GTY(()) my_unsigned;

/* TYPE_STRING: String types */
const char * GTY(()) msg = "test string";
char GTY(()) str[] = "hello world";
static const char * GTY(()) static_msg = "static string";

/* TYPE_CALLBACK: Function pointer types with parameters */
typedef int (* GTY((callback)) comparator)(const void *, const void *);
typedef void (* GTY((callback)) cleanup_fn)(void *);

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_type {
  /* Nested union */
  union {
    int x;
    float y;
  } GTY(()) value;
  
  /* Array of pointers */
  struct my_struct * GTY(()) ptr_array[8];
  
  /* Function pointer */
  comparator cmp;
  
  /* String field */
  const char *name;
  
  /* Pointer to incomplete type */
  struct opaque_struct *opaque;
  
  /* Nested struct */
  struct {
    int depth;
    struct complex_type *parent;
  } GTY(()) nested;
};

/* Another struct with chain_next for GC */
struct GTY((chain_next("%h.next"))) linked_list {
  int value;
  struct linked_list *next;
  struct linked_list *prev;
};

/* Union containing struct */
union GTY(()) container {
  struct my_struct s;
  struct complex_type c;
  struct linked_list l;
};

/* Typedef chain leading to scalar */
typedef int GTY(()) base_int;
typedef base_int GTY(()) middle_int;
typedef middle_int GTY(()) final_int;

/* Array of function pointers */
typedef void (* GTY(()) action_func)(int);
action_func GTY(()) actions[4];

/* Struct with variable length array at end (GCC extension) */
struct GTY(()) var_struct {
  int length;
  int data[];
};

/* Mark the types for garbage collection */
/* This ensures gengtype processes them */
void GTY(()) *gc_roots[] = {
  &fixed_array,
  &struct_array,
  &msg,
  &str,
  &actions,
  NULL
};

/* Dummy function using the types to ensure they're referenced */
static void GTY(()) use_types(void) {
  struct my_struct s = {0};
  union my_union u;
  struct complex_type c;
  struct linked_list *list = NULL;
  
  s.a = 42;
  u.i = 100;
  c.value.x = 1;
  
  /* Use string */
  const char *test = msg;
  (void)test;
  
  /* Use callback type */
  comparator cmp_func = NULL;
  (void)cmp_func;
}
