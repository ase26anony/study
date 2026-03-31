/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in the gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) base_struct {
  int scalar_field;
  char * GTY((length("%h.strlen + 1"))) string_field;
  size_t strlen;
};

/* Struct with nested struct and pointer chain */
struct GTY((chain_next ("%h.next"))) linked_struct {
  int id;
  struct base_struct * GTY((tag("0"))) data;
  struct linked_struct *next;
};

/* Struct containing a union */
struct GTY(()) struct_with_union {
  int type;
  union {
    int int_val;
    double double_val;
    char * GTY((tag("1"))) string_val;
  } GTY((desc ("%h.type"))) value;
};

/* TYPE_UNION: Various union types */
union GTY(()) simple_union {
  int i;
  float f;
  void *p;
};

/* Union with struct members */
union GTY((desc ("%0.code"))) tagged_union {
  int code;
  struct base_struct base;
  struct linked_struct linked;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void * GTY(()) void_ptr;
typedef struct base_struct * GTY(()) struct_ptr;
typedef const char * GTY(()) const_string_ptr;

/* Function pointer types (may become TYPE_CALLBACK) */
typedef int (* GTY(()) compare_func)(const void *, const void *);
typedef void (* GTY((callback)) cleanup_func)(void *);

/* TYPE_ARRAY: Various array types */
extern int GTY(()) external_array[];
static int GTY(()) static_array[100];
int GTY(()) global_array[50];

/* Array of pointers */
struct base_struct * GTY(()) struct_ptr_array[10];

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

typedef _Bool GTY(()) bool_type;
typedef long long GTY(()) long_long_type;
typedef unsigned GTY(()) unsigned_type;

/* TYPE_STRING: String types and literals */
const char GTY(()) *const_string = "Hello, gengtype!";
char GTY(()) string_array[] = "Test string array";

/* TYPE_CALLBACK: Explicit callback types */
typedef int GTY((callback)) (*gty_callback)(struct base_struct *, void *);
typedef void GTY((callback)) (*void_callback)(void);

/* Callback struct field */
struct GTY(()) callback_container {
  gty_callback callback;
  void *user_data;
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* Using GCC vector extensions to trigger lang_struct handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  int code;
  union tree_node *chain;
  union tree_node *type;
};

struct GTY(()) tree_decl {
  struct tree_common common;
  const char *name;
  union tree_node *initial;
};

union GTY((desc ("%h.common.code"))) tree_node {
  struct tree_common common;
  struct tree_decl decl;
};

/* RTL-like structure */
struct GTY(()) rtx_def {
  int code;
  int mode;
  union {
    int int_val;
    struct rtx_def *rtx_ptr;
    const char *string;
  } GTY((desc ("GET_CODE(%h)"))) u;
};

typedef struct rtx_def *rtx;

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Struct field */
  struct base_struct base;
  
  /* Union field */
  union tagged_union utag;
  
  /* Pointer field */
  struct complex_nested * GTY((skip)) sibling;
  
  /* Array field */
  int GTY(()) matrix[3][3];
  
  /* Pointer to array */
  int (* GTY(())) ptr_to_array)[10];
  
  /* Function pointer */
  compare_func comparator;
  
  /* String pointer */
  const char * GTY(()) description;
  
  /* Scalar */
  color_t color;
  
  /* Nested struct */
  struct {
    int depth;
    struct complex_nested *parent;
  } GTY(()) nest;
};

/* Variable declarations to ensure types are instantiated */
struct base_struct GTY(()) global_base_struct;
union simple_union GTY(()) global_simple_union;
struct linked_struct GTY(()) *global_linked_list;
color_t GTY(()) global_color = BLUE;

/* Array of various types */
union tree_node * GTY(()) tree_array[5];
struct rtx_def * GTY(()) rtx_array[3];

/* Function using these types (may help with processing) */
static void GTY(()) 
process_types(struct complex_nested *cn) {
  /* Function body not important for gengtype */
  (void)cn;
}

/* Additional types to ensure coverage */

/* Incomplete array in struct */
struct GTY(()) flexible_array_struct {
  int count;
  int data[];
};

/* Bitfield struct */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Typedef chain */
typedef int GTY(()) my_int;
typedef my_int GTY(()) my_int2;
typedef my_int2 GTY(()) my_int3;

/* Const pointer types */
typedef int * const GTY(()) const_ptr;
typedef const struct base_struct * GTY(()) const_struct_ptr;

/* Volatile types */
typedef volatile int GTY(()) volatile_int;
typedef int volatile * GTY(()) volatile_ptr;

/* Function returning struct */
struct base_struct GTY(()) *(*func_returning_struct)(int);

/* Pointer to function returning pointer to array */
int (*(* GTY(()) complex_func_ptr)(void))[10];

/* Anonymous struct/union */
struct GTY(()) anonymous_container {
  struct {
    int x;
    int y;
  } point;
  union {
    int i;
    float f;
  } value;
};

/* Ensure all basic type kinds are represented */
typedef struct GTY(()) {
  /* Empty struct - edge case */
} empty_struct;

/* Union with only scalars */
union GTY(()) scalar_union {
  char c;
  short s;
  int i;
  long l;
  float f;
  double d;
};

/* Multiple indirection */
typedef int *** GTY(()) triple_ptr;

/* Self-referential structure */
struct GTY(()) self_ref {
  int data;
  struct self_ref *next;
  struct self_ref *prev;
};

/* Mutual recursion */
struct GTY(()) mutual_a;
struct GTY(()) mutual_b;

struct GTY(()) mutual_a {
  int id;
  struct mutual_b *partner;
};

struct GTY(()) mutual_b {
  int id;
  struct mutual_a *partner;
};

/* Array of function pointers */
typedef void (* GTY(()) func_array[5])(void);

/* Complex callback signature */
typedef struct base_struct *(* GTY((callback)) 
  complex_callback)(int, const char *, struct linked_struct **, size_t);

/* Final global to ensure everything is processed */
struct GTY(()) all_types_container {
  struct base_struct *base;
  union simple_union simple_u;
  struct linked_struct *linked;
  union tagged_union tagged_u;
  int_ptr int_p;
  compare_func cmp;
  int array[5];
  color_t color;
  const char *string;
  gty_callback cb;
  v4si vector;
  union tree_node *tree;
  struct rtx_def *rtx;
  struct complex_nested *complex;
  struct flexible_array_struct *flex;
  struct bitfield_struct bits;
  my_int3 typedef_chain;
  const_struct_ptr const_p;
  volatile_ptr volatile_p;
  struct anonymous_container anon;
  empty_struct empty;
  union scalar_union scalars;
  triple_ptr triple;
  struct self_ref *self;
  struct mutual_a *mutual_a;
  struct mutual_b *mutual_b;
  func_array funcs;
  complex_callback complex_cb;
};
