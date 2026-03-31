/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid dependencies */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declaration without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
  int field1;
  char* GTY((skip)) field2;  /* Skip this pointer field */
};  /* TYPE_STRUCT */

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with discriminator
   ============================================ */
union GTY((desc("0"))) my_union {
  int a;
  char* GTY((skip)) b;
  struct my_struct* GTY((skip)) c;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer type
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

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
    void* p;
  } u;
  tree dummy_tree;  /* Use dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("strlen(%h.my_string) + 1"))) my_string;  /* TYPE_STRING */

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("outer_struct"))) outer_struct {
  struct my_struct* GTY((skip)) ptr_field;
  union my_union union_field;
  int GTY((length("array_len"))) dynamic_array[1];
  int array_len;
};

/* Struct with callback field */
struct GTY((tag("callback_struct"))) callback_struct {
  my_callback_fn callback;
  my_scalar_t scalar;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
  struct my_struct s;
  struct outer_struct o;
  rtx dummy_rtx;  /* Use dummy GCC type */
};

/* Array of pointers */
struct my_struct* GTY((skip)) pointer_array[5];  /* TYPE_ARRAY of TYPE_POINTER */

/* String array */
const char* GTY((length("strlen(%h.str_array[%I]) + 1"))) str_array[3];  /* TYPE_ARRAY of TYPE_STRING */

#endif /* TEST_STATE_GTY_H */
