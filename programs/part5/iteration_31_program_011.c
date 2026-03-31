#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* ==================== UNDEFINED TYPES ==================== */
/* Forward declaration without definition - will be TYPE_UNDEFINED */
struct undefined_struct GTY((tag("undefined")));

/* Another undefined type */
union undefined_union GTY((tag("undefined_union")));

/* ==================== SCALAR TYPES ==================== */
/* Basic scalar types - will be TYPE_SCALAR */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef long GTY(()) scalar_long_t;
typedef unsigned int GTY(()) scalar_uint_t;

/* ==================== STRING TYPE ==================== */
/* String pointer - will be TYPE_STRING */
typedef const char * GTY(()) string_t;

/* Another string type with different annotation */
typedef const char * GTY((length("strlen(%h) + 1"))) counted_string_t;

/* ==================== STRUCT TYPES ==================== */
/* Simple struct - will be TYPE_STRUCT */
struct simple_struct GTY(())
{
  scalar_int_t field1;
  scalar_float_t field2;
  string_t name;
};

/* Nested struct */
struct outer_struct GTY(())
{
  struct simple_struct GTY((tag("simple"))) inner;
  scalar_double_t value;
};

/* Struct with array member */
struct array_struct GTY(())
{
  int GTY(()) data[10];
  int count;
};

/* ==================== USER STRUCT TYPES ==================== */
/* User-defined struct - will be TYPE_USER_STRUCT */
typedef struct simple_struct GTY(()) user_struct_t;

/* Another user struct */
typedef struct outer_struct GTY(()) outer_user_struct_t;

/* ==================== UNION TYPES ==================== */
/* Simple union - will be TYPE_UNION */
union data_union GTY(())
{
  scalar_int_t as_int;
  scalar_float_t as_float;
  string_t as_string;
  void * GTY((skip)) as_pointer;
};

/* Union within a struct */
struct union_container GTY(())
{
  int type;
  union data_union GTY((tag("type"))) data;
};

/* ==================== POINTER TYPES ==================== */
/* Various pointer typedefs - will be TYPE_POINTER */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;
typedef void (* GTY(()) void_func_ptr_t)(void);

/* Pointer to pointer */
typedef int_ptr_t * GTY(()) int_ptr_ptr_t;

/* ==================== ARRAY TYPES ==================== */
/* Array typedefs - will be TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[8];

/* Multi-dimensional array */
typedef int GTY(()) matrix_t[3][3];

/* ==================== CALLBACK TYPES ==================== */
/* Function pointer typedefs - will be TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int, const char *);
typedef int (* GTY(()) compare_func_t)(const void *, const void *);
typedef void (* GTY(()) simple_callback_t)(void);

/* Struct with callback member */
struct callback_container GTY(())
{
  callback_t handler;
  void * GTY((skip)) user_data;
};

/* ==================== LANG STRUCT TYPES ==================== */
/* Language-specific structs with GC roots - will be TYPE_LANG_STRUCT */
struct GTY((for_user)) lang_struct_base
{
  int lang_specific_field;
  void * GTY((skip)) lang_data;
};

/* Another lang struct with chain */
struct GTY((chain_next("%h.next"))) lang_struct_chain
{
  struct lang_struct_chain * GTY((skip)) next;
  int value;
};

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Complex type combining multiple categories */
struct complex_nested GTY(())
{
  /* Scalar */
  scalar_int_t id;
  
  /* String */
  string_t description;
  
  /* Struct */
  struct simple_struct GTY((tag("simple"))) base;
  
  /* Union */
  union data_union GTY((tag("id"))) variant;
  
  /* Pointer */
  struct complex_nested * GTY((skip)) self_ptr;
  
  /* Array */
  int GTY(()) scores[5];
  
  /* Callback */
  callback_t notify;
  
  /* Pointer to array */
  int (* GTY(()) matrix_ptr)[3];
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

/* ==================== TYPE WITH ALL CATEGORIES ==================== */
/* This struct should cause traversal through all type categories */
struct master_type GTY(())
{
  /* Scalar members */
  scalar_int_t counter;
  scalar_double_t precision;
  
  /* String member */
  string_t title;
  
  /* Nested struct */
  struct simple_struct GTY((tag("simple"))) nested_struct;
  
  /* User struct */
  user_struct_t user_struct;
  
  /* Union */
  union data_union GTY((tag("counter"))) data_union;
  
  /* Pointer to various types */
  int_ptr_t int_pointer;
  struct_ptr_t struct_pointer;
  callback_t callback_pointer;
  
  /* Array */
  int_array_t number_array;
  struct_array_t struct_array;
  
  /* Lang struct */
  struct lang_struct_base GTY((tag("lang"))) lang_struct;
  
  /* Pointer to undefined type */
  struct undefined_struct * GTY((skip)) undefined_ptr;
  
  /* Array of pointers */
  struct_ptr_t GTY(()) ptr_array[4];
  
  /* Union array */
  union_array_t unions;
  
  /* Multi-dimensional array */
  matrix_t transformation;
  
  /* Callback array */
  callback_t GTY(()) handlers[3];
};

#endif /* TEST_TYPES_H */
