/* test-gtype-coverage.h - Comprehensive type declarations for gengtype coverage
   This file contains diverse type declarations to trigger all type serialization
   cases in gengtype-state.cc's write_state_type function. */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

/* Include necessary GCC headers for internal types */
#include "config.h"
#include "system.h"
#include "coretypes.h"

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations without definitions */
struct opaque_struct;
union opaque_union;

/* ==================== TYPE_STRUCT ==================== */
/* Basic struct with various field types */
struct GTY(()) basic_struct {
  int scalar_field;
  char *string_field;
  struct basic_struct *next;
  void *generic_pointer;
};

/* Nested struct with complex dependencies */
struct GTY(()) outer_struct {
  int id;
  struct GTY(()) inner_struct {
    float x;
    float y;
    struct outer_struct *parent;
  } inner;
  
  union GTY(()) data_union {
    int i;
    double d;
    char *s;
  } data;
  
  struct basic_struct *related;
};

/* Struct with array fields */
struct GTY(()) array_struct {
  int fixed_array[10];
  struct outer_struct *ptr_array[5];
  int (*func_ptr_array[3])(void);
};

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union GTY(()) simple_union {
  int as_int;
  float as_float;
  double as_double;
  void *as_pointer;
  struct basic_struct *as_struct;
};

/* Tagged union with struct field */
union GTY(()) tagged_union {
  struct GTY(()) {
    int type;
    union simple_union data;
  } tagged;
  
  long long raw_data;
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer type declarations */
typedef int * GTY((skip)) int_ptr;
typedef void (* GTY((callback)) void_func_ptr)(void);
typedef struct basic_struct *struct_ptr;
typedef union simple_union *union_ptr;

/* Pointer to incomplete type */
struct opaque_struct *opaque_ptr;

/* Function pointer with parameters */
typedef int (* GTY((callback)) comparator_func)(const void *, const void *);

/* Pointer to array */
typedef int (*array_ptr)[10];

/* ==================== TYPE_ARRAY ==================== */
/* Various array declarations */
extern int incomplete_array[];
int fixed_size_array[100] = {0};
struct basic_struct struct_array[5];
union simple_union union_array[8];

/* Array of pointers */
void *pointer_array[20];

/* Multi-dimensional array */
int matrix[3][4];

/* ==================== TYPE_SCALAR ==================== */
/* Enum types */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

typedef enum GTY(()) bool_type {
  FALSE,
  TRUE
} bool_t;

/* Fundamental scalar types */
typedef int integer_type;
typedef long long big_int;
typedef float float_type;
typedef double double_type;
typedef _Bool bool_scalar;
typedef char char_type;

/* ==================== TYPE_STRING ==================== */
/* String literals and character arrays */
const char * GTY((skip)) message = "Hello, gengtype!";
char string_array[] = "Test string";
const char *string_pointers[] = {"first", "second", "third"};

/* Struct with string field */
struct GTY(()) string_struct {
  const char *name;
  char buffer[256];
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types treated as callbacks */
typedef void (* GTY((callback)) simple_callback)(void);
typedef int (* GTY((callback)) complex_callback)(struct basic_struct *, union simple_union);

/* Callback with return value */
typedef struct outer_struct *(* GTY((callback)) factory_callback)(int id);

/* ==================== TYPE_USER_STRUCT / TYPE_LANG_STRUCT ==================== */
/* GCC-specific types that map to internal lang_struct categories */

/* Vector type using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Simulated tree node structure (mimics GCC's internal tree type) */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node *chain;
  union tree_node *type;
  location_t locus;
};

/* Simulated RTL structure */
struct GTY(()) rtx_def {
  int code;
  int mode;
  union {
    int rt_int;
    char *rt_str;
    struct rtx_def *rt_rtx;
  } u;
};

/* Union for tree nodes (common GCC pattern) */
union GTY((desc ("TREE_CODE (&%h)"), 
           chain_next ("TREE_CHAIN (&%h)"))) tree_node {
  struct tree_common common;
  struct tree_decl decl;
  struct tree_type type;
  /* ... other tree variants would be here in real GCC */
};

/* ==================== COMPLEX TYPE COMBINATIONS ==================== */

/* Recursive type structure */
struct GTY(()) recursive_struct {
  int value;
  struct recursive_struct *left;
  struct recursive_struct *right;
  void (* GTY((callback)) process)(struct recursive_struct *);
};

/* Struct containing all major type kinds */
struct GTY(()) comprehensive_struct {
  /* Scalar fields */
  int id;
  enum color color;
  
  /* Struct field */
  struct basic_struct basic;
  
  /* Union field */
  union tagged_union variant;
  
  /* Pointer fields */
  int *int_ptr;
  void (*func_ptr)(int);
  struct comprehensive_struct *self_ptr;
  
  /* Array fields */
  char name[50];
  int scores[10];
  struct basic_struct *refs[5];
  
  /* String field */
  const char *description;
  
  /* Callback field */
  comparator_func compare;
  
  /* Nested anonymous struct */
  struct {
    int x;
    int y;
  } position;
  
  /* GCC-specific type */
  v4si vector_data;
};

/* ==================== TYPE DEFINITIONS WITH GTY OPTIONS ==================== */

/* Chainable struct with custom options */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_node {
  int data;
  struct linked_node *next;
  struct linked_node *prev;
  void (* GTY((callback)) notify)(struct linked_node *);
};

/* Array with length annotation */
struct GTY(()) variable_array {
  size_t length;
  int GTY((length ("%h.length"))) data[];
};

/* Union with descriminator */
union GTY((desc ("%d.type"))) discriminated_union {
  struct {
    int type;
    union simple_union data;
  } d;
  long long raw;
};

/* ==================== EXTERNAL DECLARATIONS ==================== */

/* Force gengtype to consider these types */
extern struct comprehensive_struct GTY(()) global_comprehensive;
extern union tree_node * GTY(()) global_tree;
extern struct rtx_def * GTY(()) global_rtx;
extern v4si GTY(()) global_vector;

/* Array of various types */
extern void * GTY(()) mixed_array[];

/* ==================== FUNCTION DECLARATIONS ==================== */

/* Functions that use the types */
void GTY((callback)) process_struct(struct basic_struct *s);
int GTY((callback)) compare_structs(const void *a, const void *b);
struct outer_struct * GTY((callback)) create_outer(int id);

/* ==================== TYPEDEF CHAINS ==================== */

/* Chain of typedefs leading to scalar */
typedef int base_int;
typedef base_int derived_int;
typedef derived_int final_int;

/* Chain leading to pointer */
typedef struct basic_struct *base_ptr;
typedef base_ptr const const_base_ptr;

/* Chain leading to callback */
typedef void (*base_callback)(void);
typedef base_callback registered_callback;

#endif /* TEST_GTYPE_COVERAGE_H */
