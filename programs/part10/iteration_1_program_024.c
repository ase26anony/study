/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) 
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
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
  int field1;
  tree field2;  /* Using dummy GCC type */
  struct my_undefined_struct* next;  /* Pointer to undefined type */
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
  char* GTY((skip)) b;  /* Skip this pointer field */
  struct my_struct* c;
  double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Various pointer types
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union* GTY(()) another_pointer;   /* Another pointer */

/* ============================================
   TYPE_ARRAY: Arrays with length attributes
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct* GTY((length("struct_count"))) struct_array[5];

/* Variable for array length (referenced in length attribute) */
extern int my_array_length;
extern int struct_count;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void* p;
    tree t;  /* GCC type */
  } u;
  rtx insn;  /* Another GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar types with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;        /* TYPE_SCALAR */
typedef double GTY((user)) my_double_t;     /* Another scalar */

/* ============================================
   TYPE_STRING: String pointers
   ============================================ */
const char* GTY((length("strlen(my_string)+1"))) my_string;  /* TYPE_STRING */
char* GTY((length("custom_length"))) another_string;

/* ============================================
   TYPE_CALLBACK: Function pointers
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, char*);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) another_callback)(tree, rtx);

/* ============================================
   Complex nested structure to exercise more paths
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
  my_scalar_t scalar_field;
  my_user_struct_t user_struct_field;
  union my_union union_field;
  struct my_lang_struct* GTY((skip)) lang_ptr;
  int GTY((length("array_len"))) dynamic_array[];
};

/* ============================================
   Container structure referencing all types
   ============================================ */
struct GTY((tag("container"))) container {
  /* Reference to undefined type */
  struct my_undefined_struct* GTY((skip)) undefined_ptr;
  
  /* Regular struct */
  struct my_struct regular_struct;
  
  /* User struct */
  my_user_struct_t user_struct;
  
  /* Union */
  union my_union my_union;
  
  /* Pointer */
  struct my_struct* pointer_field;
  
  /* Array */
  int GTY((length("5"))) fixed_array[5];
  
  /* Lang struct */
  struct my_lang_struct lang_struct;
  
  /* Scalar */
  my_scalar_t scalar;
  
  /* String */
  const char* GTY((length("10"))) string_field;
  
  /* Callback */
  my_callback_fn callback;
  
  /* Nested complex struct */
  struct complex_struct* complex_ptr;
};

/* ============================================
   Global variables with various GTY annotations
   ============================================ */
extern struct container GTY(()) global_container;
extern struct my_struct* GTY((skip)) global_struct_array[];
extern const char* GTY((length("global_str_len"))) global_string;

#endif /* TEST_STATE_GTY_H */
