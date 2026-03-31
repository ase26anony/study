/* test-gtype-coverage.c - Comprehensive type definitions for gengtype coverage testing */
/* This file should be placed in the gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Regular struct with various field types */
struct GTY(()) my_struct {
  int a;                    /* TYPE_SCALAR */
  char * GTY((skip)) b;     /* TYPE_POINTER with skip attribute */
  struct my_struct *next;   /* Recursive pointer */
  union my_union *u_ptr;    /* Pointer to union */
};

/* Another struct with array field */
struct GTY(()) struct_with_array {
  int count;
  struct my_struct * GTY((length("%h.count"))) items[];
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
typedef struct GTY((user)) user_struct {
  int id;
  void (* GTY((callback)) cleanup)(void*);
} user_struct_t;

/* TYPE_UNION: Regular union */
union GTY(()) my_union {
  int i;
  float f;
  double d;
  void *p;
  struct my_struct *s_ptr;
};

/* Tagged union for better coverage */
union GTY((desc ("%0.tag"))) tagged_union {
  int tag;
  struct {
    int tag;
    int value;
  } GTY((tag ("0"))) int_val;
  struct {
    int tag;
    float value;
  } GTY((tag ("1"))) float_val;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void (* GTY((callback)) func_ptr)(void);
typedef struct my_struct * GTY(()) struct_ptr;
typedef union my_union * GTY(()) union_ptr;

/* Function pointer with parameters */
typedef int (* GTY((callback)) comparator)(const void *, const void *);

/* TYPE_ARRAY: Different array types */
extern int GTY(()) incomplete_array[];
int GTY(()) fixed_array[10] = {0};
struct my_struct * GTY(()) ptr_array[5];

/* Variable length array in struct */
struct GTY(()) varray_struct {
  int len;
  int GTY((length("%h.len"))) data[];
};

/* TYPE_SCALAR: Various scalar types */
typedef enum { RED, GREEN, BLUE } GTY(()) color;
typedef _Bool GTY(()) boolean;
typedef long long GTY(()) long_long_type;
typedef unsigned char GTY(()) byte;

/* TYPE_STRING: String types */
const char * GTY(()) message = "Hello, gengtype!";
char GTY(()) greeting[] = "Welcome";
static const char * GTY(()) static_msg = "Static string";

/* TYPE_CALLBACK: Function pointer types */
typedef void (* GTY((callback)) simple_callback)(void);
typedef int (* GTY((callback)) complex_callback)(struct my_struct *, union my_union *);

/* Callback with return value */
typedef struct my_struct * (* GTY((callback)) factory_func)(int id);

/* TYPE_LANG_STRUCT: GCC internal types */
/* Vector types using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Simulated tree node structure (simplified) */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node *chain;
};

/* More complex lang_struct with nested types */
struct GTY(()) lang_type {
  struct tree_common common;
  union {
    struct my_struct *s;
    union my_union *u;
  } GTY((desc ("%0.common.code"))) data;
};

/* Nested type structures for deep traversal */
struct GTY(()) outer_struct {
  int id;
  struct GTY(()) inner_struct {
    int value;
    struct outer_struct *parent;
    union {
      int i;
      float f;
    } data;
  } inner;
  
  struct inner_struct * GTY((skip)) siblings[3];
  
  /* Callback field */
  complex_callback cb;
};

/* Array of pointers to callbacks */
simple_callback GTY(()) callback_array[] = {NULL, NULL};

/* Union containing struct with array */
union GTY(()) complex_union {
  struct varray_struct vs;
  struct outer_struct os;
  int_ptr *double_ptr;
};

/* Typedef chain leading to scalar */
typedef int GTY(()) base_int;
typedef base_int GTY(()) middle_int;
typedef middle_int GTY(()) final_int;

/* Struct with bitfields (scalar handling) */
struct GTY(()) bitfield_struct {
  unsigned int flag:1;
  unsigned int value:8;
  unsigned int padding:23;
};

/* Opaque pointer type */
typedef struct opaque_struct * GTY(()) opaque_ptr;

/* Self-referential type structures */
struct GTY(()) node {
  int value;
  struct node * GTY((skip)) left;
  struct node * GTY((skip)) right;
  struct node * GTY((chain_next ("%h.next"))) next;
};

/* Function pointer returning pointer to array */
typedef int (* GTY((callback)) array_getter(void))[10];

/* Null callback type */
typedef void (* GTY((callback)) null_callback)(void);

/* Test structure using all major type kinds */
struct GTY(()) comprehensive_test {
  /* SCALAR */
  int scalar_int;
  color scalar_enum;
  
  /* POINTER */
  int_ptr int_pointer;
  struct_ptr struct_pointer;
  
  /* ARRAY */
  int fixed_size_array[5];
  struct node *pointer_array[3];
  
  /* STRING */
  const char *string_ptr;
  char string_array[20];
  
  /* UNION */
  union my_union data_union;
  
  /* CALLBACK */
  comparator compare_cb;
  
  /* Nested STRUCT */
  struct inner_struct nested;
  
  /* LANG_STRUCT (simulated) */
  v4si vector_data;
  
  /* Reference to UNDEFINED */
  opaque_ptr undefined_ref;
};

/* Global instances for processing */
struct comprehensive_test GTY(()) global_test_instance;
struct node GTY(()) *global_node_list = NULL;
union complex_union GTY(()) global_union;

/* Array of various types */
void * GTY(()) mixed_array[] = {
  &global_test_instance,
  &global_node_list,
  &global_union,
  NULL
};

/* Function pointer table */
static complex_callback GTY(()) callback_table[] = {
  NULL,
  NULL
};

/* Nested array in struct */
struct GTY(()) matrix {
  int rows;
  int cols;
  int GTY((length("%h.rows * %h.cols"))) data[];
};

/* Final test: struct containing all type categories */
struct GTY(()) ultimate_test {
  /* Basic types */
  int id;                                  /* SCALAR */
  char name[50];                           /* STRING (array of char) */
  
  /* Pointer types */
  struct ultimate_test *self;              /* POINTER to self */
  void (* GTY((callback)) func)(void);     /* CALLBACK */
  
  /* Container types */
  union {
    int as_int;
    float as_float;
  } value;                                 /* UNION */
  
  struct {
    int count;
    struct node *items[];                  /* ARRAY in nested struct */
  } GTY((length ("%h.count"))) container;
  
  /* Lang struct */
  v4si vector;                             /* LANG_STRUCT */
  
  /* Reference to undefined */
  struct opaque_struct *unknown;           /* UNDEFINED reference */
};

/* Global instance to ensure processing */
struct ultimate_test GTY(()) *global_ultimate = NULL;
