/* test-gtype-coverage.c - Comprehensive type declarations for gengtype coverage */
/* This file should be placed in the gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations to trigger TYPE_UNDEFINED */
struct undefined_struct;
union undefined_union;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) simple_struct {
  int a;
  char b;
};

struct GTY(()) nested_struct {
  struct simple_struct inner;
  long extra;
};

/* Recursive struct with chain_next for TYPE_STRUCT */
struct GTY((chain_next ("%h.next"))) linked_list {
  int value;
  struct linked_list * GTY((skip)) next;
};

/* Struct with union field */
struct GTY(()) struct_with_union {
  int type;
  union {
    int int_val;
    double double_val;
    void *ptr_val;
  } GTY((desc ("%0.type"))) data;
};

/* TYPE_USER_STRUCT: Struct with user-defined behavior */
typedef struct GTY((user)) user_struct {
  int id;
  char *name;
  /* User-defined marking function would be specified elsewhere */
} user_struct_t;

/* TYPE_UNION: Various union types */
union GTY(()) simple_union {
  int i;
  float f;
  char *s;
};

/* Tagged union with descriminator */
union GTY((desc ("%1.type"), tag ("1"))) tagged_union {
  int type;
  struct {
    int x;
    int y;
  } point;
  struct {
    char *name;
    int age;
  } person;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void * GTY(()) void_ptr;
typedef const char * GTY(()) const_string_ptr;

/* Pointer to incomplete type */
struct undefined_struct * GTY(()) opaque_ptr;

/* TYPE_ARRAY: Various array types */
int GTY(()) fixed_array[10];
extern int GTY(()) incomplete_array[];

/* Array of pointers */
struct simple_struct * GTY(()) struct_ptr_array[5];

/* Flexible array member in struct */
struct GTY(()) array_container {
  int count;
  int GTY((length ("%h.count"))) items[];
};

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

typedef _Bool GTY(()) bool_type;
typedef long long GTY(()) long_long_type;

/* TYPE_STRING: String types */
const char GTY(()) *message = "Hello, gengtype!";
char GTY(()) string_array[] = "Test string";

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_func)(const void *, const void *);
typedef void GTY((callback)) (*simple_callback)(void);

/* Callback with parameters */
typedef int GTY((callback)) (*binary_op)(int, int);

/* TYPE_LANG_STRUCT: GCC internal types */
/* Vector type using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  int code;
  union tree_node *chain;
  union tree_node *type;
};

union GTY((desc ("tree_code_length[(int) ((%h).common.code)]"))) tree_node {
  struct tree_common common;
  struct tree_decl decl;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container {
  /* Struct field */
  struct nested_struct nested;
  
  /* Union field */
  union simple_union data;
  
  /* Pointer field */
  int_ptr numbers;
  
  /* Array field */
  color_t colors[3];
  
  /* String field */
  const char *description;
  
  /* Callback field */
  compare_func comparator;
  
  /* Lang struct field */
  v4si vector_data;
  
  /* Pointer to user struct */
  user_struct_t *user_data;
  
  /* Array of pointers to undefined types */
  struct undefined_struct *opaque_array[2];
};

/* Function pointers in structs */
struct GTY(()) callback_container {
  binary_op operation;
  simple_callback notify;
};

/* Additional test structures for edge cases */

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
  unsigned int flag:1;
  unsigned int value:7;
  unsigned int padding:24;
};

/* Struct with nested anonymous struct */
struct GTY(()) anonymous_member {
  struct {
    int x;
    int y;
  } point;
  int z;
};

/* Typedef chain leading to scalar */
typedef int GTY(()) base_int;
typedef base_int GTY(()) wrapped_int;
typedef wrapped_int GTY(()) double_wrapped_int;

/* Union with array */
union GTY(()) union_with_array {
  int as_int;
  char as_chars[4];
};

/* Self-referential union */
union GTY(()) self_ref_union {
  int value;
  union self_ref_union *next;
};

/* Complex graph structure */
struct GTY(()) graph_node {
  int id;
  struct graph_node ** GTY((length ("%h.neighbor_count"))) neighbors;
  int neighbor_count;
};

/* Template for generating multiple instances */
#define DECLARE_STRUCT_TYPE(name, field_type) \
  struct GTY(()) name##_struct { \
    field_type value; \
    struct name##_struct *next; \
  }

DECLARE_STRUCT_TYPE(int, int);
DECLARE_STRUCT_TYPE(double, double);
DECLARE_STRUCT_TYPE(pointer, void*);

/* Ensure all types are referenced to avoid being optimized out */
static void GTY((callback)) use_all_types(void) {
  struct simple_struct s1;
  struct nested_struct s2;
  struct linked_list *list = NULL;
  struct struct_with_union su;
  user_struct_t us;
  union simple_union u1;
  union tagged_union tu;
  int_ptr ip = NULL;
  void_ptr vp = NULL;
  const_string_ptr sp = NULL;
  int arr_val = fixed_array[0];
  struct array_container ac;
  color_t c = RED;
  bool_type b = 1;
  long_long_type ll = 0;
  const char *msg = message;
  char *str = string_array;
  compare_func cmp = NULL;
  binary_op op = NULL;
  v4si vec;
  struct tree_common tc;
  union tree_node tn;
  struct container cont;
  struct callback_container cb;
  struct bitfield_struct bfs;
  struct anonymous_member am;
  double_wrapped_int dwi = 0;
  union union_with_array uwa;
  union self_ref_union sru;
  struct graph_node gn;
  struct int_struct is;
  struct double_struct ds;
  struct pointer_struct ps;
  
  /* Reference all variables to avoid warnings */
  (void)s1; (void)s2; (void)list; (void)su; (void)us;
  (void)u1; (void)tu; (void)ip; (void)vp; (void)sp;
  (void)arr_val; (void)ac; (void)c; (void)b; (void)ll;
  (void)msg; (void)str; (void)cmp; (void)op; (void)vec;
  (void)tc; (void)tn; (void)cont; (void)cb; (void)bfs;
  (void)am; (void)dwi; (void)uwa; (void)sru; (void)gn;
  (void)is; (void)ds; (void)ps;
}
