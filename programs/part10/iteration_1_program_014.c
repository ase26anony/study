/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
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
  char* GTY((skip)) field2;  /* Skip this pointer field */
};  /* TYPE_STRUCT */

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with discriminator
   ============================================ */
union GTY((desc("$1"))) my_union {
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
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_len;  /* Length variable for the array */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void* GTY((skip)) p;
  } u;
  tree dummy_tree;  /* Use dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar type with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("my_string_len"))) my_string;  /* TYPE_STRING */
extern int my_string_len;  /* Length variable for the string */

/* ============================================
   TYPE_CALLBACK: Function pointer type
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, char*);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure thorough traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("outer_struct"))) outer_struct {
  struct my_struct* GTY((skip)) ptr1;
  union my_union data;
  my_callback_fn callback;
};

/* Array of pointers */
struct my_struct* GTY((length("ptr_array_len"))) ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
extern int ptr_array_len;

/* Struct containing all types */
struct GTY((tag("mega_struct"))) mega_struct {
  my_scalar_t scalar;           /* TYPE_SCALAR */
  const char* GTY((length("str_len"))) str;  /* TYPE_STRING */
  int GTY((length("arr_len"))) arr[20];      /* TYPE_ARRAY */
  union my_union uni;           /* TYPE_UNION */
  struct my_lang_struct* GTY((skip)) lang_ptr;  /* TYPE_POINTER to TYPE_LANG_STRUCT */
  my_callback_fn handlers[3];   /* Array of TYPE_CALLBACK */
};

/* Global variables to exercise different storage classes */
extern struct my_struct GTY((tag("global_struct"))) global_var;
static union my_union GTY((desc("0"))) static_union_var;

/* Function pointer table */
typedef void (*GTY((user)) func_ptr_t)(void);
func_ptr_t GTY((length("func_count"))) func_table[10];  /* TYPE_ARRAY of TYPE_CALLBACK */
extern int func_count;

#endif /* TEST_STATE_GTY_H */
