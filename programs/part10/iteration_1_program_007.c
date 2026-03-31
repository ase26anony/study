/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef int gimple;

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
  struct my_struct * GTY((tag("my_struct"))) c;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Various pointer types
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union * GTY((tag("my_union"))) my_union_ptr;
rtx GTY((skip)) my_rtx_ptr;  /* Dummy GCC type pointer */

/* ============================================
   TYPE_ARRAY: Arrays with length attribute
   ============================================ */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct * GTY((length("struct_count"))) struct_array[5];
const char * GTY((length("str_len"))) string_array[3];

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void * GTY((skip)) p;
    tree dummy;  /* GCC internal type */
  } u;
  gimple stmt;  /* Another dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef unsigned long GTY((user)) my_ulong_t;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("strlen(my_string)+1"))) my_string;  /* TYPE_STRING */
char * GTY((length("custom_len_func()"))) dynamic_string;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) compare_fn)(const void*, const void*);

/* ============================================
   Complex nested structure to exercise more paths
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
  my_scalar_t scalar_field;
  my_user_struct_t user_struct_field;
  my_callback_fn callback_field;
  union my_union union_field;
  struct my_lang_struct * GTY((skip)) lang_struct_ptr;
  int GTY((length("array_len_field"))) dynamic_array[1];
};

/* ============================================
   Root variable declarations for gengtype to process
   ============================================ */
extern struct my_struct GTY(()) root_struct;
extern union my_union GTY(()) root_union;
extern struct my_lang_struct GTY(()) root_lang_struct;
extern struct complex_struct GTY(()) root_complex;

#endif /* TEST_STATE_GTY_H */
