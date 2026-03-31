/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) __attribute__((garbage_collected(x)))
#endif

/* Dummy definitions for GCC internal types to avoid dependency issues */
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
  tree field2;  /* Using dummy GCC type */
  char * GTY((skip)) skip_field;  /* Skip this pointer field */
};  /* TYPE_STRUCT */

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with descriminator
   ============================================ */
union GTY((desc("0"))) my_union {
  int a;
  char * GTY((skip)) b;
  struct my_struct * GTY((skip)) c;
  double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer type
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union * GTY((skip)) union_pointer;  /* Another pointer */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct * GTY((length("struct_array_len"))) struct_array[5];  /* Array of pointers */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void *p;
    tree t;  /* GCC type */
  } u;
  rtx r;  /* Another GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef double GTY((user)) my_double_t;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("strlen(%h.my_string)+1"))) my_string;  /* TYPE_STRING */
char * GTY((length("custom_length_func(%h.other_str)"))) other_string;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) another_callback)(tree, rtx);

/* ============================================
   Complex nested example to exercise more paths
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
  my_scalar_t scalar_field;  /* TYPE_SCALAR */
  my_user_struct_t user_field;  /* TYPE_USER_STRUCT */
  union my_union union_field;  /* TYPE_UNION */
  struct my_lang_struct * GTY((skip)) lang_ptr;  /* TYPE_POINTER to TYPE_LANG_STRUCT */
  int GTY((length("array_len_field"))) dynamic_array[];  /* TYPE_ARRAY */
};

/* Global variables to ensure they're processed */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern my_callback_fn GTY(()) global_callback;

#endif /* TEST_STATE_GTY_H */
