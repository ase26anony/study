/* test_state_gty.h - Comprehensive test of all GTY type categories */
#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
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
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
  int field1;
  tree field2;  /* Using dummy GCC type */
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
  char * GTY((skip)) b;  /* Skip this pointer field */
  struct my_struct * GTY((tag("my_struct"))) c;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer type
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_length;  /* For the length function */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void * GTY((skip)) p;
  } u;
  tree lang_tree;  /* Language-specific tree node */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar type with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("strlen(%h.my_string) + 1"))) my_string;  /* TYPE_STRING */

/* ============================================
   TYPE_CALLBACK: Function pointer type
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, void*);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("outer_struct"))) outer_struct {
  struct my_struct * GTY((tag("my_struct"))) nested_ptr;
  union my_union data;
  my_scalar_t scalar_field;
};

/* Array of pointers */
struct my_struct * GTY((length("ptr_array_len"), skip)) ptr_array[5];
extern int ptr_array_len;

/* Struct containing callback */
struct GTY((tag("callback_container"))) callback_container {
  my_callback_fn callback;
  void * GTY((skip)) user_data;
};

/* Forward declaration that will be defined later */
struct GTY(()) later_defined_struct;

/* Actually define it now */
struct GTY((tag("later_struct"))) later_defined_struct {
  int value;
  struct later_defined_struct * GTY((tag("later_struct"))) next;
};

#endif /* TEST_STATE_GTY_H */
