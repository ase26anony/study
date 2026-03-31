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
   TYPE_UNDEFINED: Forward declaration without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
  int field1;
  char *GTY((skip)) field2;  /* Skip this pointer field */
  tree field3;               /* GCC internal type */
};                           /* TYPE_STRUCT */

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with discriminator
   ============================================ */
union GTY((desc("0"))) my_union {
  int a;
  char *GTY((skip)) b;
  struct my_struct *GTY((tag("1"))) c;
  double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer type
   ============================================ */
struct my_struct *GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_length;  /* Length variable for the array */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void *GTY((skip)) p;
    tree t;
  } u;
  rtx insn;  /* GCC internal type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char *GTY((length("strlen(%h.my_string)+1"))) my_string;  /* TYPE_STRING */

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, char*);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("outer_struct"))) outer_struct {
  struct my_struct *GTY((tag("1"))) ptr_field;
  union my_union GTY((desc("2"))) union_field;
  my_callback_fn callback;
};

/* Array of pointers */
struct my_struct *GTY((length("ptr_array_len"))) ptr_array[5];
extern int ptr_array_len;

/* Struct containing all types */
struct GTY((tag("mega_struct"))) mega_struct {
  /* TYPE_SCALAR */
  my_scalar_t scalar_field;
  
  /* TYPE_STRING */
  const char *GTY((length("strlen(%h.string_field)+1"))) string_field;
  
  /* TYPE_ARRAY */
  int GTY((length("array_len"))) int_array[20];
  int array_len;
  
  /* TYPE_POINTER */
  struct outer_struct *GTY((skip)) outer_ptr;
  
  /* TYPE_UNION */
  union my_union GTY((desc("3"))) data_union;
  
  /* TYPE_CALLBACK */
  my_callback_fn handler;
};

/* Forward declaration for testing TYPE_NONE (should not appear in output) */
struct incomplete;

#endif /* TEST_STATE_GTY_H */
