/* test-gtype-coverage.c - Comprehensive type declarations for gengtype coverage
 * This file should be placed in the gcc/ directory and processed during GCC build.
 * It contains GTY-annotated types covering all TYPE_* enum values.
 */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
typedef struct opaque_struct *GTY((skip)) opaque_ptr;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRING: String types */
const char GTY(()) *string_ptr = "test string";
char GTY(()) string_array[] = "string literal";

/* TYPE_ARRAY: Array types */
int GTY(()) fixed_array[10];
extern int GTY(()) incomplete_array[];
typedef int GTY(()) (*array_of_func_ptrs[5])(void);

/* TYPE_POINTER: Various pointer types */
typedef int *GTY(()) int_ptr;
typedef void (*GTY((callback)) void_func_ptr)(void);
typedef struct GTY(()) my_struct *GTY(()) struct_ptr;

/* TYPE_CALLBACK: Function pointer types with parameters */
typedef int GTY((callback)) (*comparator_fn)(const void *, const void *);
typedef void GTY((callback)) (*traverse_fn)(void *, void *);

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
  int GTY(()) a;
  char *GTY(()) b;
  struct my_struct *GTY(()) next;
  int GTY(()) array_field[5];
};

struct GTY(()) nested_struct {
  struct my_struct GTY(()) inner;
  union my_union GTY(()) u;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int GTY(()) i;
  float GTY(()) f;
  void *GTY(()) p;
  struct my_struct GTY(()) s;
};

union GTY(()) complex_union {
  int GTY(()) tag;
  struct {
    int GTY(()) x;
    int GTY(()) y;
  } GTY(()) point;
  struct {
    char *GTY(()) name;
    int GTY(()) age;
  } GTY(()) person;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  int GTY(()) data;
  void (*GTY((skip)) custom_mark)(void *);
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific structs */
/* Vector types using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  int GTY(()) code;
  union tree_node *GTY((skip)) chain;
};

union GTY(()) tree_node {
  struct tree_common GTY(()) common;
  struct {
    int GTY(()) type;
    void *GTY(()) value;
  } GTY(()) expr;
};

/* Complex type graph with recursion and nesting */
struct GTY((chain_next("%h.next"))) gc_list {
  int GTY(()) value;
  struct gc_list *GTY(()) next;
  union my_union GTY(()) data;
  comparator_fn GTY(()) compare;
};

/* Array of pointers to structs */
struct GTY(()) *GTY(()) ptr_array[10];

/* Function returning struct */
struct my_struct GTY(()) *GTY((returns_struct)) create_struct(int size);

/* Typedef chain leading to scalar */
typedef int GTY(()) base_type;
typedef base_type GTY(()) derived_type;
typedef derived_type GTY(()) final_type;

/* Callback in struct */
struct GTY(()) processor {
  void *GTY(()) data;
  traverse_fn GTY(()) process;
  void (*GTY((callback)) cleanup)(struct processor *);
};

/* Union containing callback */
union GTY(()) callback_union {
  int GTY(()) id;
  void (*GTY((callback)) handler)(int, void *);
};

/* Nested anonymous struct/union */
struct GTY(()) container {
  struct {
    int GTY(()) x;
    int GTY(()) y;
  } GTY(()) coord;
  union {
    int GTY(()) ival;
    float GTY(()) fval;
  } GTY(()) value;
};

/* External references to ensure TYPE_UNDEFINED */
extern struct GTY(()) undefined_struct;
extern union GTY(()) undefined_union;

/* Global variables with various types */
struct my_struct GTY(()) global_struct;
union my_union GTY(()) global_union;
int_ptr GTY(()) global_ptr;
color_enum GTY(()) global_enum = RED;
