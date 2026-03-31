/* test-gtype-coverage.h - Comprehensive type definitions for gengtype coverage */
/* This file should be placed in the gcc/ directory and included in the build */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_struct;
union opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef long scalar_long;
typedef _Bool scalar_bool;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING: String types */
const char *global_string = "test string";
char string_array[] = "hello world";

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct my_struct *struct_ptr;
typedef int (*simple_func_ptr)(void);

/* TYPE_CALLBACK: Function pointer types with parameters */
typedef int GTY((callback)) (*comparator_fn)(const void *, const void *);
typedef void GTY((callback)) (*traversal_fn)(void *data, void *user_data);

/* TYPE_ARRAY: Array types */
extern int incomplete_array[];
int fixed_size_array[100];
typedef int array_of_ints[10];
typedef struct my_struct *array_of_ptrs[5];

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
  int id;
  char *name;
  struct my_struct *next;
  int scores[5];
};

struct GTY(()) nested_struct {
  int value;
  struct my_struct embedded;
  struct nested_struct *link;
};

/* TYPE_UNION: Union types */
union GTY(()) data_union {
  int int_val;
  float float_val;
  double double_val;
  void *ptr_val;
  char *string_val;
};

/* Complex struct with union field */
struct GTY(()) variant_record {
  int type;
  union GTY(()) {
    int int_data;
    float float_data;
    char *string_data;
  } u;
  struct variant_record *next;
};

/* TYPE_USER_STRUCT: Struct with special handling */
/* Using GCC vector extension to trigger special type handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) user_special {
  v4si vector_data;
  int regular_field;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* Mimicking tree-like structures used in GCC internals */
struct GTY(()) tree_common {
  int code;
  union tree_node *chain;
  union tree_node *type;
};

struct GTY(()) tree_int_cst {
  struct tree_common common;
  long int value;
};

union GTY((desc ("TREE_CODE (&%h)"), 
           chain_next ("TREE_CHAIN (&%h)"))) tree_node {
  struct tree_common common;
  struct tree_int_cst int_cst;
};

/* Recursive and nested type structures for deep traversal */
struct GTY((chain_next ("%h.next"))) complex_node {
  int id;
  char *GTY((length ("%h.name_len"))) name;
  int name_len;
  union data_union data;
  struct complex_node *next;
  struct complex_node *children[4];
  void (*processor)(struct complex_node *);
};

/* Array of complex structures */
typedef struct complex_node *node_array[20];

/* Function pointer returning struct */
struct my_struct GTY((returns_struct)) (*struct_maker)(int id, const char *name);

/* Callback in struct */
struct GTY(()) callback_container {
  void *data;
  comparator_fn compare;
  traversal_fn traverse;
};

/* Incomplete array of structs */
extern struct my_struct partial_array[];

/* Self-referential structure with multiple pointer types */
struct GTY(()) graph_node {
  int id;
  char *label;
  struct graph_node **neighbors;  /* Dynamic array of pointers */
  int neighbor_count;
  void (*visit)(struct graph_node *);
};

/* Union containing pointers of different types */
union GTY(()) multi_ptr {
  int *int_ptr;
  char **string_ptr_ptr;
  struct graph_node **node_ptr_ptr;
  void (*func_ptr)(void);
};

/* Struct with all type kinds combined */
struct GTY(()) type_kitchen_sink {
  /* SCALAR */
  int counter;
  color_enum color;
  
  /* STRING */
  char *message;
  char fixed_string[50];
  
  /* POINTER */
  void *user_data;
  struct type_kitchen_sink *self;
  
  /* ARRAY */
  int numbers[10];
  struct graph_node *nodes[5];
  
  /* UNION */
  union data_union storage;
  
  /* CALLBACK */
  traversal_fn callback;
  
  /* Nested STRUCT */
  struct my_struct embedded_struct;
  
  /* Function pointer array */
  int (*operations[3])(int, int);
  
  /* Pointer to array */
  int (*matrix)[10];
};

/* External declarations to ensure TYPE_UNDEFINED is encountered */
extern struct undefined_external *external_ptr;
extern union undefined_union *external_union_ptr;

/* Template for language-specific structure (like C++ templates) */
struct GTY(()) template_base {
  int template_id;
  void *instantiations;
};

/* Vector types for TYPE_LANG_STRUCT */
typedef float GTY(()) v8sf __attribute__((vector_size(32)));
typedef long GTY(()) v4di __attribute__((vector_size(32)));

struct GTY(()) simd_data {
  v8sf floats;
  v4di integers;
  int flags;
};

/* Mark all relevant types for garbage collection */
typedef struct my_struct * GTY(()) my_struct_ptr;
typedef union data_union GTY(()) data_union_t;
typedef struct complex_node GTY(()) complex_node_t;
typedef struct type_kitchen_sink GTY(()) kitchen_sink_t;

#endif /* TEST_GTYPE_COVERAGE_H */
