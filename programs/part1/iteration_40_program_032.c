/* test_gty.h - Header file with GTY annotations for all type categories */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED - Forward declared struct
   ============================================ */
struct GTY(()) undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_SCALAR - Basic scalar type with user attribute
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING - String with length attribute
   ============================================ */
const char * GTY((length)) my_string;  /* TYPE_STRING */

/* ============================================
   TYPE_STRUCT - Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {  /* TYPE_STRUCT */
  int field;
  my_scalar_t scalar_field;
};

/* ============================================
   TYPE_USER_STRUCT - User-defined struct type
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION - Union with desc attribute
   ============================================ */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
  int a;
  char * GTY((skip)) b;
  struct my_struct *c;
};

/* ============================================
   TYPE_POINTER - Pointer with skip attribute
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY - Array with length attribute
   ============================================ */
int GTY((length)) my_array[10];  /* TYPE_ARRAY */

/* ============================================
   TYPE_CALLBACK - Function pointer with user attribute
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */

/* ============================================
   TYPE_LANG_STRUCT - Language-specific structure
   ============================================ */
/* Create a language-specific structure pattern */
struct GTY((special("lang_struct"))) lang_specific_struct {  /* TYPE_LANG_STRUCT */
  int lang_code;
  union GTY((desc("lang_code"))) {
    int int_val;
    char *string_val;
    struct my_struct *struct_ptr;
  } GTY((tag("0"))) u;
};

/* ============================================
   Additional complex types to ensure thorough parsing
   ============================================ */

/* Nested struct with pointer array */
struct GTY(()) complex_struct {
  struct my_struct * GTY((length)) ptr_array[5];
  union my_union data;
  my_callback_fn callback;
};

/* Typedef with chain of pointers */
typedef struct my_struct *** GTY((user)) triple_ptr_t;

/* Array of pointers */
struct my_struct * GTY((length)) ptr_list[20];

/* Struct containing all types */
struct GTY((tag("all_types"))) container {
  my_scalar_t scalar;           /* TYPE_SCALAR */
  const char * GTY((length)) str; /* TYPE_STRING */
  struct my_struct regular;     /* TYPE_STRUCT */
  my_user_struct_t user;        /* TYPE_USER_STRUCT */
  union my_union uni;           /* TYPE_UNION */
  struct my_struct *ptr;        /* TYPE_POINTER */
  int GTY((length)) arr[5];     /* TYPE_ARRAY */
  my_callback_fn cb;            /* TYPE_CALLBACK */
  struct lang_specific_struct lang; /* TYPE_LANG_STRUCT */
};

#endif /* TEST_GTY_H */
