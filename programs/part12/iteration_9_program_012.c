/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) base_struct {
  int scalar_field;
  char *string_field;
};

struct GTY((chain_next("%h.next"))) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
};

struct GTY(()) complex_struct {
  /* TYPE_ARRAY: Fixed-size array */
  int fixed_array[10];
  
  /* TYPE_ARRAY: Incomplete array */
  int incomplete_array[];
  
  /* TYPE_POINTER: Various pointers */
  void *void_ptr;
  struct base_struct *struct_ptr;
  
  /* TYPE_UNION: Union field */
  union {
    int i;
    float f;
    void *p;
  } data;
  
  /* TYPE_CALLBACK: Function pointer */
  int (* GTY((callback)) compare_func)(const void *, const void *);
  
  /* TYPE_STRING: String pointer with literal */
  const char * GTY((length("%l"))) message;
};

/* TYPE_UNION: Standalone union type */
union GTY(()) my_union {
  int int_val;
  double double_val;
  struct base_struct *struct_ptr;
  void (*func_ptr)(void);
};

/* TYPE_USER_STRUCT: Using GCC-specific type with special handling */
/* Vector types often get special treatment */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* TYPE_LANG_STRUCT: Mimic GCC internal tree-like structure */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node *chain;
  union tree_node *type;
  location_t locus;
};

/* Another lang_struct-like type with GTY options */
struct GTY((desc("%0.code"), tag("TREE_CODE"))) tree_exp {
  struct tree_common common;
  union tree_node *operands[1];
};

/* TYPE_POINTER: Various pointer typedefs */
typedef int * GTY(()) int_ptr;
typedef void (* GTY((callback)) void_func_ptr)(void);
typedef struct complex_struct * GTY(()) complex_ptr;

/* TYPE_ARRAY: Array typedefs */
typedef int GTY(()) int_array[5];
typedef struct base_struct * GTY(()) struct_ptr_array[3];

/* TYPE_SCALAR: Enum type */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

/* TYPE_SCALAR: Boolean */
typedef _Bool GTY(()) bool_t;

/* TYPE_STRING: String arrays with initialization */
const char GTY(()) global_string[] = "Global string constant";
static char GTY(()) module_name[] = "gtype-test-module";

/* TYPE_CALLBACK: Complex callback type */
typedef int (* GTY((callback)) sort_func_t)(
  const void * GTY((skip)) a,
  const void * GTY((skip)) b,
  void * GTY((skip)) user_data
);

/* Nested structure with all type kinds */
struct GTY(()) container {
  /* TYPE_STRUCT: Nested anonymous struct */
  struct {
    int x;
    int y;
  } point;
  
  /* TYPE_UNION: Nested anonymous union */
  union {
    long long_val;
    double double_val;
  } number;
  
  /* TYPE_ARRAY of TYPE_POINTER to TYPE_STRUCT */
  struct base_struct * GTY((length("%l.count"))) items[10];
  int count;
  
  /* TYPE_ARRAY of TYPE_STRING */
  char * GTY((length("%l.str_count"))) strings[5];
  int str_count;
  
  /* TYPE_POINTER to TYPE_CALLBACK */
  sort_func_t sorter;
  
  /* TYPE_POINTER to incomplete TYPE_UNDEFINED */
  struct opaque_struct *opaque;
  
  /* TYPE_SCALAR: Bitfield */
  unsigned int flags: 8;
  
  /* TYPE_ARRAY: Flexible array member */
  color_t colors[];
};

/* Recursive type structure */
struct GTY(()) tree_node {
  int value;
  struct tree_node * GTY((skip)) left;
  struct tree_node * GTY((skip)) right;
  struct tree_node * GTY((skip)) parent;
};

/* Union with struct, pointer, and scalar types */
union GTY(()) variant_data {
  struct container cont;
  struct tree_node *node;
  int scalar;
  char *str;
  void (*action)(void);
};

/* TYPE_CALLBACK: Function pointer in struct */
struct GTY(()) callback_holder {
  const char *name;
  void (* GTY((callback)) handler)(struct callback_holder *);
  void * GTY((skip)) user_data;
};

/* Array of unions */
union GTY(()) variant_array[4];

/* Pointer to array */
typedef union variant_data (* GTY(()) variant_matrix_ptr)[4];

/* Complex type graph with multiple indirections */
struct GTY(()) type_graph {
  struct type_graph * GTY((skip)) self_ptr;
  struct container *container_ptr;
  union variant_data *variant_ptr;
  int (* GTY((callback)) processor)(struct type_graph *);
  char name[32];
};

/* Global variables with GTY markers for root set */
struct container * GTY((root)) global_container;
struct tree_node * GTY((root)) global_tree;
union variant_data * GTY((root)) global_variants[5];

/* Function to force inclusion in gengtype processing */
void GTY((extern)) test_gtype_coverage_init(void) {
  /* Empty function, just to ensure file is processed */
}

/* Additional GCC-specific types that might trigger TYPE_LANG_STRUCT */

/* Simulate an rtx structure (used in GCC's RTL intermediate representation) */
struct GTY(()) rtx_def {
  int code;
  union {
    int rt_int;
    char *rt_str;
    struct rtx_def *rt_rtx;
  } u;
};

/* Simulate a gimple statement structure */
struct GTY(()) gimple_statement {
  int code;
  unsigned num_ops;
  struct tree_node *ops[1];
};

/* Union of GCC internal structures */
union GTY((tag("RTL_CODE"))) rtx_union {
  struct rtx_def rt;
  struct tree_common tc;
  int raw[4];
};

/* Vector type with GTY - might be TYPE_USER_STRUCT */
typedef float GTY(()) v4sf __attribute__((vector_size(16)));

/* Opaque pointer type */
typedef void * GTY((skip)) opaque_handle;

/* Callback with skip parameters */
typedef void (* GTY((callback)) event_callback)(
  int event_type,
  void * GTY((skip)) event_data,
  opaque_handle handle
);

/* Final structure containing references to all type kinds */
struct GTY(()) master_type {
  /* TYPE_STRUCT */
  struct base_struct base;
  
  /* TYPE_UNION */
  union my_union uni;
  
  /* TYPE_POINTER */
  struct complex_struct *complex;
  
  /* TYPE_ARRAY */
  int numbers[7];
  
  /* TYPE_SCALAR */
  color_t color;
  
  /* TYPE_STRING */
  char *title;
  
  /* TYPE_CALLBACK */
  event_callback on_event;
  
  /* TYPE_USER_STRUCT */
  v4si vectors[2];
  
  /* TYPE_LANG_STRUCT */
  struct tree_common tree;
  
  /* TYPE_UNDEFINED pointer */
  struct opaque_struct *unknown;
  
  /* Recursive pointer */
  struct master_type *next;
};

/* Global instance to ensure processing */
struct master_type GTY((root)) global_master;
