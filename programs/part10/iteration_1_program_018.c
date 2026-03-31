/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid parsing errors */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declared struct without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
  int field1;
  char field2;
  tree dummy_tree;  /* Use dummy GCC type */
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
  struct my_struct * GTY((tag("1"))) c;
  double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Various pointer types
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union * GTY((tag("union_ptr"))) my_union_ptr;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Arrays with length attribute
   ============================================ */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct * GTY((length("struct_array_len"))) struct_array[5];  /* TYPE_ARRAY */

/* Variable to hold array length (referenced in length attribute) */
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
    tree t;
  } u;
  rtx dummy_rtx;  /* Use dummy GCC type */
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
char * GTY((length("another_str_len"))) another_string;  /* TYPE_STRING */

/* Variable to hold string length */
extern int str_len;
extern int another_str_len;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, char*);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) another_callback)(tree, rtx);  /* TYPE_CALLBACK */

/* ============================================
   Nested structures to ensure full traversal
   ============================================ */

/* Struct containing pointers to various types */
struct GTY((tag("container"))) type_container {
  /* Reference all major types */
  struct my_struct * GTY((tag("nested_struct_ptr"))) nested_struct;
  union my_union * GTY((tag("nested_union_ptr"))) nested_union;
  my_callback_fn callback;
  const char * GTY((length("container_str_len"))) container_string;
  int GTY((length("container_array_len"))) container_array[20];
  struct my_lang_struct lang_struct_field;
};

/* Union containing various pointer types */
union GTY((desc("1"))) pointer_union {
  struct my_struct * GTY((tag("0"))) struct_ptr;
  union my_union * GTY((tag("1"))) union_ptr;
  void * GTY((skip)) void_ptr;
  const char * GTY((length("pu_str_len"))) string_ptr;
};

/* Array of pointers */
struct my_struct * GTY((length("ptr_array_len"))) ptr_array[15];  /* TYPE_ARRAY */
extern int ptr_array_len;

/* ============================================
   Global variables with GTY markers
   ============================================ */
extern struct my_struct GTY((tag("global_struct"))) global_struct_instance;
extern union my_union GTY((desc("2"))) global_union_instance;
extern struct my_lang_struct GTY((special("global_lang"))) global_lang_instance;

/* Function pointer variable */
extern my_callback_fn GTY((user)) global_callback;

#endif /* TEST_STATE_GTY_H */
