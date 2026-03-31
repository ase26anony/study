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

struct GTY((chain_next ("%h.next"))) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
};

struct GTY(()) complex_struct {
  /* Nested struct */
  struct base_struct nested;
  
  /* Union field */
  union {
    int i;
    float f;
  } GTY((desc ("%1.i"))) u;
  
  /* Array field */
  int array_field[10];
  
  /* Pointer to callback */
  int (* GTY((callback)) compare_func)(int, int);
};

/* TYPE_UNION: Union types */
union GTY(()) simple_union {
  int ival;
  float fval;
  double dval;
  void *pval;
};

union GTY((desc ("%0.kind"))) tagged_union {
  enum { KIND_INT, KIND_FLOAT, KIND_PTR } kind;
  struct {
    int kind;
    int value;
  } GTY((tag ("0"))) as_int;
  struct {
    int kind;
    float value;
  } GTY((tag ("1"))) as_float;
  struct {
    int kind;
    void *value;
  } GTY((tag ("2"))) as_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void * GTY(()) void_ptr;
typedef struct base_struct * GTY(()) struct_ptr;
typedef union simple_union * GTY(()) union_ptr;

/* Function pointer types (TYPE_CALLBACK) */
typedef int (* GTY((callback)) comparator)(const void *, const void *);
typedef void (* GTY((callback)) simple_callback)(void);
typedef struct base_struct * (* GTY((callback)) factory_func)(int);

/* TYPE_ARRAY: Array declarations */
extern int GTY(()) external_array[];
static int GTY(()) static_array[20];
int GTY(()) global_array[50];

/* Array of pointers */
struct base_struct * GTY(()) ptr_array[30];

/* TYPE_SCALAR: Scalar types and enums */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

typedef _Bool GTY(()) bool_t;
typedef long GTY(()) long_type;
typedef unsigned long GTY(()) ulong_type;

/* TYPE_STRING: String types */
const char GTY(()) *const_string = "Hello, gengtype!";
char GTY(()) string_array[] = "Test string";
static const char GTY(()) *static_strings[] = {
  "first",
  "second",
  "third"
};

/* TYPE_LANG_STRUCT: GCC internal types using vector extensions */
typedef int GTY(()) v4si __attribute__((vector_size(16)));
typedef float GTY(()) v4sf __attribute__((vector_size(16)));

/* Simulating tree-like structure (common in GCC) */
struct GTY(()) tree_common {
  int code;
  union tree_node *chain;
  union tree_node *type;
};

union GTY((desc ("((enum tree_code)(%h.code))"))) tree_node {
  struct tree_common common;
  /* Various tree node types would go here */
  struct {
    struct tree_common common;
    int value;
  } integer_cst;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY(()) user_defined {
  int magic;
  char *name;
  /* Use a marker to indicate user struct handling */
  void GTY((user)) *user_data;
};

/* Recursive and nested type structures */
struct GTY(()) container {
  /* Struct containing union */
  union tagged_union data;
  
  /* Array of structs */
  struct base_struct items[5];
  
  /* Pointer to function returning struct */
  struct base_struct *(* GTY((callback)) get_item)(int index);
  
  /* Pointer to opaque type */
  struct opaque_struct *opaque_ptr;
  
  /* Nested container */
  struct container * GTY((skip)) next_container;
};

/* Callback function type with complex signature */
typedef struct container *(* GTY((callback)) 
  complex_callback)(int, struct base_struct **, comparator);

/* Global variables with various types */
struct complex_struct GTY(()) global_complex;
union simple_union GTY(()) global_union;
color_t GTY(()) global_color = BLUE;
comparator GTY(()) global_comparator = NULL;

/* Incomplete array in struct */
struct GTY(()) flexible_array_struct {
  int count;
  int data[];
};

/* Multiple levels of indirection */
typedef struct container *** GTY(()) triple_ptr;

/* Const pointer types */
typedef int * const GTY(()) const_int_ptr;
typedef const struct base_struct * GTY(()) const_struct_ptr;

/* Anonymous struct/union */
struct GTY(()) with_anonymous {
  struct {
    int x;
    int y;
  } point;
  union {
    int id;
    char *name;
  } identifier;
};

/* Bitfield testing */
struct GTY(()) with_bitfields {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int value;
};

/* Type definition chains */
typedef int GTY(()) my_int;
typedef my_int GTY(()) my_int2;
typedef my_int2 GTY(()) my_int3;

/* Array of function pointers */
comparator GTY(()) comparators[10];

/* Struct with array of unions */
struct GTY(()) union_array_container {
  int count;
  union simple_union variants[4];
};

/* Forward declared struct that's later defined */
struct GTY(()) forward_declared;
struct GTY(()) uses_forward {
  struct forward_declared *ptr;
};

struct GTY(()) forward_declared {
  int value;
  struct uses_forward *back_ref;
};

/* Self-referential types */
struct GTY(()) self_ref {
  int data;
  struct self_ref *next;
  struct self_ref *prev;
};

/* Complex graph structure */
struct GTY(()) graph_node {
  int id;
  struct graph_node ** GTY((length ("%h.neighbor_count"))) neighbors;
  int neighbor_count;
};

/* Union with array */
union GTY(()) union_with_array {
  int ints[4];
  float floats[4];
  char chars[16];
};

/* Make sure all types are referenced to avoid being optimized out */
void GTY(()) reference_all_types(void) {
  /* This function doesn't need a body, just declarations */
  extern struct complex_struct *use_complex;
  extern union tagged_union *use_tagged;
  extern color_t use_color;
  extern comparator use_callback;
  extern v4si use_vector;
  extern struct user_defined *use_user;
  extern struct container *use_container;
  extern struct flexible_array_struct *use_flex;
  extern struct with_bitfields *use_bits;
  extern struct graph_node *use_graph;
  extern union union_with_array *use_union_array;
  
  /* Reference the undefined type */
  extern struct opaque_struct *use_opaque;
}
