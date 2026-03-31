/* test-gtype-coverage.h - Comprehensive type declarations for gengtype coverage
   This file contains diverse type declarations to trigger all TYPE_* cases
   in write_state_type() function in gengtype-state.cc */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

/* Include GCC internal headers if needed */
#include "config.h"
#include "system.h"

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations without definitions */
struct opaque_struct;
union opaque_union;
typedef struct incomplete *incomplete_ptr_t;

/* ==================== TYPE_STRUCT ==================== */
/* Basic struct with various field types */
struct GTY(()) basic_struct {
  int scalar_field;
  char *string_field;
  double float_field;
};

/* Nested struct with pointers */
struct GTY(()) nested_struct {
  struct basic_struct GTY((skip)) *data;
  int count;
  struct nested_struct *next;
};

/* Struct with array field */
struct GTY(()) array_struct {
  int ids[10];
  char name[32];
  struct basic_struct items[5];
};

/* Struct with union field */
struct GTY(()) struct_with_union {
  int type;
  union {
    int int_val;
    double float_val;
    char *str_val;
  } GTY((desc("%0.type"))) value;
};

/* Chainable struct for GC */
struct GTY((chain_next("%h.next"))) chainable_struct {
  int id;
  char *name;
  struct chainable_struct *next;
};

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union GTY(()) simple_union {
  int i;
  float f;
  char *s;
  void *p;
};

/* Tagged union with desc */
union GTY((desc("%0.tag"))) tagged_union {
  int tag;
  struct {
    int x, y;
  } point;
  struct {
    float radius;
  } circle;
  char *text;
};

/* Union containing structs */
union GTY(()) complex_union {
  struct basic_struct s;
  struct nested_struct n;
  union simple_union u;
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
typedef int *GTY((skip)) int_ptr;
typedef void *GTY((skip)) void_ptr;
typedef const char *GTY((skip)) const_string_ptr;
typedef struct basic_struct *struct_ptr;
typedef union simple_union *union_ptr;

/* Pointer to pointer */
typedef int **int_ptr_ptr;

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Opaque pointer type */
typedef struct opaque_struct *opaque_ptr_t;

/* ==================== TYPE_ARRAY ==================== */
/* Fixed size arrays */
int GTY((skip)) global_array[100];
extern char GTY((skip)) extern_array[];

/* Array of pointers */
struct basic_struct *GTY((skip)) ptr_array[20];

/* Multi-dimensional array */
int GTY((skip)) matrix[3][3];

/* Array in struct (already covered above) */

/* ==================== TYPE_SCALAR ==================== */
/* Fundamental scalar types */
typedef int my_int;
typedef unsigned long my_ulong;
typedef _Bool my_bool;
typedef char my_char;
typedef double my_double;

/* Enum types */
enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

typedef enum GTY(()) status {
  STATUS_OK,
  STATUS_ERROR,
  STATUS_PENDING
} status_t;

/* Bitfield struct */
struct GTY(()) bitfield_struct {
  unsigned int flag1:1;
  unsigned int flag2:2;
  unsigned int flag3:3;
  int value:8;
};

/* ==================== TYPE_STRING ==================== */
/* String literals and char arrays */
const char GTY((skip)) *global_string = "Hello, World!";
static const char GTY((skip)) *static_string = "Static string";

/* Char array initialized with string */
char GTY((skip)) char_array[] = "Initialized array";

/* String in struct */
struct GTY(()) string_struct {
  const char *GTY((skip)) message;
  char name[32];
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*GTY((callback)) compare_func)(const void *, const void *);
typedef void (*GTY((callback)) simple_callback)(void);
typedef char *(*GTY((callback)) string_generator)(int);

/* Callback with parameters */
typedef int (*GTY((callback)) binary_op)(int, int);

/* Struct with callback field */
struct GTY(()) callback_container {
  compare_func GTY((skip)) comparator;
  simple_callback GTY((skip)) handler;
  void *GTY((skip)) user_data;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Using GCC vector extension - may be treated as user struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));
typedef float GTY(()) v4sf __attribute__((vector_size(16)));

/* Struct with GCC attributes */
struct GTY(()) aligned_struct {
  int data;
  char pad;
} __attribute__((aligned(16)));

/* Packed struct */
struct GTY(()) packed_struct {
  char a;
  int b;
  char c;
} __attribute__((packed));

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Tree-like structures mimicking GCC internals */
struct GTY(()) tree_common {
  int code;
  union tree_node *chain;
  union tree_node *type;
};

struct GTY(()) tree_int_cst {
  struct tree_common common;
  long int value;
};

union GTY((desc ("((enum tree_code) (%h.common.code))"))) tree_node {
  struct tree_common common;
  struct tree_int_cst int_cst;
  /* Add more tree types as needed */
};

/* RTL-like structures */
struct GTY(()) rtx_def {
  int code;
  int mode;
  union {
    long int int_val;
    char *str_val;
    struct rtx_def *rtx;
  } GTY((desc ("%0.code"))) u;
};

typedef struct rtx_def *rtx;

/* Vector types for GCC internals */
struct GTY(()) tree_vector {
  struct tree_common common;
  union tree_node *elements;
  int length;
};

/* ==================== COMPLEX TYPE COMBINATIONS ==================== */
/* Recursive type structure */
struct GTY(()) tree_node_wrapper {
  union tree_node *GTY((skip)) node;
  struct tree_node_wrapper *left;
  struct tree_node_wrapper *right;
  int balance;
};

/* Array of unions */
union GTY(()) variant_array[10];

/* Struct containing all type kinds */
struct GTY(()) master_struct {
  /* TYPE_STRUCT (nested) */
  struct basic_struct nested;
  
  /* TYPE_UNION */
  union tagged_union variant;
  
  /* TYPE_POINTER */
  struct master_struct *self_ptr;
  
  /* TYPE_ARRAY */
  int scores[5];
  
  /* TYPE_SCALAR */
  enum color color;
  _Bool flag;
  
  /* TYPE_STRING */
  char *description;
  
  /* TYPE_CALLBACK */
  compare_func cmp;
  
  /* TYPE_USER_STRUCT */
  v4si vector_data;
  
  /* TYPE_LANG_STRUCT */
  union tree_node *tree;
  
  /* Chain for GC */
  struct master_struct *next;
};

/* ==================== TYPEDEF CHAINS ==================== */
typedef int base_type;
typedef base_type derived_type;
typedef derived_type final_type;

typedef struct basic_struct *ptr_to_struct;
typedef ptr_to_struct alias_ptr;

/* ==================== EXTERNAL DECLARATIONS ==================== */
/* Force gengtype to consider external types */
extern struct opaque_struct *external_opaque;
extern int (*external_callback)(void);
extern union tree_node *external_tree;

/* ==================== FUNCTION DECLARATIONS ==================== */
/* Functions using the types */
struct master_struct *GTY((skip)) create_master(void);
void process_tree(union tree_node *GTY((skip)) node);
int compare_masters(const struct master_struct *a, 
                    const struct master_struct *b);

#endif /* TEST_GTYPE_COVERAGE_H */
