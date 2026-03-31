/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (as in standalone gengtype test) */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid parsing errors */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declaration without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_SCALAR: Scalar type with user annotation
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {  /* TYPE_STRUCT */
  int field1;
  my_scalar_t field2;
  struct my_undefined_struct* next;  /* Pointer to undefined type */
};

/* ============================================
   TYPE_USER_STRUCT: Typedef of struct with user annotation
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with descriminator
   ============================================ */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
  int a;
  char* GTY((skip)) b;  /* Skip this pointer field */
  struct my_struct* c;
};

/* ============================================
   TYPE_POINTER: Pointer type with skip annotation
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length annotation
   ============================================ */
int GTY((length("10"))) my_array[10];  /* TYPE_ARRAY */

/* ============================================
   TYPE_STRING: String pointer with length annotation
   ============================================ */
const char* GTY((length)) my_string;  /* TYPE_STRING */

/* ============================================
   TYPE_CALLBACK: Function pointer with user annotation
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {  /* TYPE_LANG_STRUCT */
  int lang_specific;
  union {
    int a;
    void* p;
    struct my_struct* s;
  } u;
  tree dummy_tree;  /* Use dummy GCC type */
};

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
  struct my_struct* GTY((skip)) ptr1;
  union my_union data;
  int GTY((length("5"))) small_array[5];
};

/* Struct with callback field */
struct GTY((tag("with_callback"))) struct_with_callback {
  my_callback_fn callback;
  const char* GTY((length)) name;
};

/* Union with nested array */
union GTY((desc("1"))) union_with_array {
  int GTY((length("8"))) numbers[8];
  struct my_struct* items;
};

/* Pointer chain for depth testing */
struct GTY((tag("chain"))) pointer_chain {
  struct pointer_chain* GTY((skip)) next;
  struct pointer_chain* GTY((skip)) prev;
};

/* Array of pointers */
struct my_struct* GTY((length("4"))) ptr_array[4];

/* String array */
const char* GTY((length)) string_array[] = {"one", "two", "three"};

/* Mixed struct with all annotation types */
struct GTY((tag("kitchen_sink"))) kitchen_sink {
  /* Scalar */
  my_scalar_t scalar;
  
  /* Pointer with skip */
  struct my_struct* GTY((skip)) skipped_ptr;
  
  /* Regular pointer */
  struct nested_struct* regular_ptr;
  
  /* Array */
  int GTY((length("3"))) int_array[3];
  
  /* String */
  const char* GTY((length)) str_field;
  
  /* Union */
  union my_union union_field;
  
  /* Callback */
  my_callback_fn callback_field;
  
  /* Nested lang struct */
  struct my_lang_struct lang_field;
};

#endif /* TEST_STATE_GTY_H */
