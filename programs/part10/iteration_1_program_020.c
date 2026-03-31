#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
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
  char* GTY((skip)) field2;  /* Skip this pointer field */
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
  char* GTY((skip)) b;
  struct my_struct* GTY((skip)) c;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer declaration
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_length;

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
const char* GTY((length("my_string_length"))) my_string;  /* TYPE_STRING */
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
  struct my_struct* GTY((skip)) ptr_field;
  union my_union union_field;
  int GTY((length("nested_array_len"))) nested_array[5];
};

/* Pointer to union */
union my_union* GTY((skip)) union_ptr;

/* Array of pointers */
struct my_struct* GTY((skip)) ptr_array[3];

/* Struct containing callback */
struct GTY((tag("with_callback"))) struct_with_callback {
  my_callback_fn callback;
  int data;
};

#endif /* TEST_STATE_GTY_H */
