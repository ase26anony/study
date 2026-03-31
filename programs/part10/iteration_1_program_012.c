/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid dependencies */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

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
  struct my_struct *c;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer declaration
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */

/* Helper for array length */
extern int my_array_length;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void * GTY((skip)) p;
  } u;
  rtx dummy_rtx;  /* Use dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("my_string_length"))) my_string;  /* TYPE_STRING */

/* Helper for string length */
extern int my_string_length;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
  struct my_struct * GTY((skip)) ptr1;
  union my_union * GTY((skip)) ptr2;
  my_scalar_t scalar_field;
};

/* Array of pointers */
struct my_struct * GTY((length("ptr_array_len"))) ptr_array[5];  /* TYPE_ARRAY */
extern int ptr_array_len;

/* Struct with callback field */
struct GTY((tag("with_callback"))) struct_with_callback {
  my_callback_fn callback;
  int data;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
  struct my_struct s;
  struct nested_struct ns;
  my_scalar_t scalar;
};

/* Forward declaration for pointer chain */
struct GTY(()) forward_declared;
struct GTY((tag("uses_forward"))) uses_forward {
  struct forward_declared * GTY((skip)) fwd_ptr;
};

/* Actually define it later */
struct GTY((tag("forward_declared"))) forward_declared {
  int value;
};

#endif /* TEST_STATE_GTY_H */
