/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) __attribute__((garbage_collected(x)))
#endif

/* Dummy definitions for GCC internal types to avoid dependencies */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declaration without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
  int field1;
  char field2;
  tree gcc_tree_field;  /* Use dummy GCC type */
};  /* TYPE_STRUCT */

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc tag
   ============================================ */
union GTY((desc("0"))) my_union {
  int a;
  char * GTY((skip)) b;  /* Skip this pointer field */
  struct my_struct * GTY((tag("my_struct"))) c;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Various pointer types
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union * GTY((tag("my_union"))) my_union_ptr;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Arrays with length attributes
   ============================================ */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct * GTY((length("struct_array_len"))) struct_array[5];  /* TYPE_ARRAY */

/* Helper variable for array length (referenced in length attribute) */
extern int my_array_len;
extern int struct_array_len;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void * GTY((skip)) p;
  } u;
  tree base;  /* Common GCC type field */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef unsigned long GTY((user)) my_ulong_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("str_len"))) my_string;  /* TYPE_STRING */
char * GTY((length("dynamic_str_len"))) dynamic_string;  /* TYPE_STRING */

/* Helper variable for string length */
extern int str_len;
extern int dynamic_str_len;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, char*);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) compare_fn)(const void*, const void*);  /* TYPE_CALLBACK */

/* ============================================
   Complex nested example to ensure full traversal
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
  /* Mix of different field types */
  my_scalar_t scalar_field;  /* TYPE_SCALAR via typedef */
  struct my_struct * GTY((tag("my_struct"))) struct_ptr;  /* TYPE_POINTER */
  union my_union nested_union;  /* TYPE_UNION */
  int GTY((length("nested_len"))) nested_array[5];  /* TYPE_ARRAY */
  const char * GTY((length("name_len"))) name;  /* TYPE_STRING */
  my_callback_fn callback;  /* TYPE_CALLBACK */
  struct my_lang_struct lang_member;  /* TYPE_LANG_STRUCT */
};

/* Helper for nested array length */
extern int nested_len;
extern int name_len;

/* ============================================
   Additional pointer chains for traversal
   ============================================ */
struct GTY((tag("node"))) list_node {
  int value;
  struct list_node * GTY((tag("node"))) next;
};

/* Root pointer to list */
struct list_node * GTY((tag("node"))) list_head;  /* TYPE_POINTER */

#endif /* TEST_STATE_GTY_H */
