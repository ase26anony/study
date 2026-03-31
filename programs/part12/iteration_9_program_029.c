/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in the gcc/ directory and processed during GCC build */

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
  /* TYPE_ARRAY: Fixed-size array */
  int fixed_array[10];
  
  /* TYPE_ARRAY: Incomplete array */
  char flexible_array[];
  
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
  const char * GTY((skip)) message;
};

/* TYPE_UNION: Standalone union type */
union GTY(()) my_union {
  int int_val;
  double double_val;
  struct base_struct * GTY((tag ("0"))) struct_ptr;
  void *void_ptr;
};

/* TYPE_USER_STRUCT: Using GCC-specific type with attributes */
struct GTY(()) user_tagged_struct {
  int code;
  union my_union data;
} __attribute__((aligned(16)));

/* TYPE_LANG_STRUCT: GCC internal-like structure */
struct GTY(()) lang_struct {
  int lang_specific;
  struct GTY((desc ("%1.code"))) user_tagged_struct *tagged;
};

/* TYPE_POINTER: Various pointer typedefs */
typedef int * GTY((skip)) int_ptr;
typedef void (* GTY((callback)) void_func_ptr)(void);
typedef struct complex_struct * GTY((skip)) complex_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY((skip)) external_array[];
static const char * GTY((skip)) string_array[] = {"hello", "world", NULL};

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

typedef _Bool bool_t;
typedef long long int64_t;

/* TYPE_STRING: String types with initialization */
const char GTY((skip)) global_string[] = "Global string constant";
static char GTY((skip)) initialized_string[] = "Initialized array";

/* TYPE_CALLBACK: Function pointer types with parameters */
typedef int (* GTY((callback)) comparator_t)(const void *, const void *);
typedef void (* GTY((callback)) traversal_func)(struct complex_struct *);

/* Recursive type structure to ensure deep traversal */
struct GTY(()) recursive_struct {
  int id;
  struct recursive_struct * GTY((skip)) child;
  struct recursive_struct * GTY((skip)) siblings[5];
  void (* GTY((callback)) visit)(struct recursive_struct *);
};

/* Container struct that references many different types */
struct GTY(()) type_container {
  /* TYPE_STRUCT */
  struct base_struct base;
  
  /* TYPE_UNION */
  union my_union uni;
  
  /* TYPE_POINTER */
  struct lang_struct *lang_ptr;
  
  /* TYPE_ARRAY */
  color_t colors[3];
  
  /* TYPE_SCALAR */
  bool_t flag;
  int64_t big_num;
  
  /* TYPE_STRING */
  char name[32];
  
  /* TYPE_CALLBACK */
  comparator_t cmp;
  
  /* TYPE_USER_STRUCT */
  struct user_tagged_struct user;
  
  /* TYPE_LANG_STRUCT */
  struct lang_struct lang;
  
  /* Recursive reference */
  struct recursive_struct * GTY((skip)) recursive;
};

/* Vector type for TYPE_LANG_STRUCT coverage */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Opaque pointer type for TYPE_UNDEFINED */
typedef struct opaque_struct * GTY((skip)) opaque_ptr;

/* Function declarations using the types */
void GTY((callback)) process_struct(struct type_container *container);
struct type_container * GTY((skip)) create_container(void);

/* Global variables with GTY markers for scanning */
extern struct type_container * GTY((root)) global_container;
extern struct recursive_struct * GTY((root)) global_recursive_list;

/* Union with struct containing array of function pointers */
union GTY(()) complex_union {
  struct {
    int count;
    void (* GTY((callback)) handlers[4])(void);
  } handler_set;
  struct type_container container;
};

/* Additional GCC-specific type patterns */
struct GTY((for_user)) user_visible_struct {
  int public_field;
  void *private_data;
};

/* Mark the end of types */
static int dummy_variable = 0;
