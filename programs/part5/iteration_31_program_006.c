#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"

/* ==================== UNDEFINED TYPES ==================== */
/* Forward declaration without definition - TYPE_UNDEFINED */
struct undefined_struct;
union undefined_union;

/* ==================== SCALAR TYPES ==================== */
/* Basic scalar types - TYPE_SCALAR */
typedef int GTY(()) int_type;
typedef float GTY(()) float_type;
typedef double GTY(()) double_type;
typedef char GTY(()) char_type;
typedef long GTY(()) long_type;
typedef unsigned int GTY(()) uint_type;

/* ==================== STRING TYPE ==================== */
/* String pointer type - TYPE_STRING */
typedef const char * GTY(()) string_type;

/* ==================== STRUCT TYPES ==================== */
/* Simple struct - TYPE_STRUCT */
struct GTY(()) simple_struct {
  int_type field1;
  float_type field2;
};

/* More complex struct with nested types */
struct GTY(()) complex_struct {
  struct simple_struct * GTY((skip)) nested_struct;
  int GTY(()) array_field[5];
  string_type name;
};

/* User-defined struct - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  double y;
  string_type desc;
} user_struct_t;

/* Another user struct with function pointer */
typedef struct GTY(()) {
  int id;
  void (* GTY((skip)) callback)(int);
} user_struct_with_callback;

/* ==================== UNION TYPES ==================== */
/* Simple union - TYPE_UNION */
union GTY(()) data_union {
  int int_val;
  float float_val;
  double double_val;
  char * GTY((skip)) string_val;
};

/* Tagged union with struct */
union GTY(()) tagged_union {
  struct simple_struct s;
  user_struct_t u;
  int i;
};

/* ==================== POINTER TYPES ==================== */
/* Various pointer typedefs - TYPE_POINTER */
typedef int * GTY(()) int_ptr;
typedef struct simple_struct * GTY(()) simple_struct_ptr;
typedef user_struct_t * GTY(()) user_struct_ptr;
typedef union data_union * GTY(()) union_ptr;
typedef void * GTY(()) void_ptr;

/* Pointer to pointer */
typedef int_ptr * GTY(()) int_ptr_ptr;

/* ==================== ARRAY TYPES ==================== */
/* Fixed-size arrays - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct simple_struct GTY(()) struct_array[5];
typedef string_type GTY(()) string_array[3];

/* Multi-dimensional array */
typedef int GTY(()) matrix[3][3];

/* Array of pointers */
typedef int_ptr GTY(()) ptr_array[8];

/* ==================== CALLBACK TYPES ==================== */
/* Function pointer typedefs - TYPE_CALLBACK */
typedef void (* GTY((skip)) simple_callback)(int);
typedef int (* GTY((skip)) complex_callback)(int, float, string_type);
typedef void (* GTY((skip)) struct_callback)(struct simple_struct *);

/* Callback returning pointer */
typedef user_struct_t * (* GTY((skip)) factory_callback)(int);

/* ==================== LANGUAGE STRUCT TYPES ==================== */
/* Structs with GTY markers for garbage collection - TYPE_LANG_STRUCT */
struct GTY(()) lang_struct_base {
  int GTY(()) tag;
  union data_union GTY(()) data;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_struct_linked {
  struct lang_struct_linked * GTY((skip)) next;
  struct lang_struct_linked * GTY((skip)) prev;
  int GTY(()) value;
  string_type GTY(()) name;
};

/* Tree-like structure */
struct GTY(()) lang_struct_tree {
  struct lang_struct_tree * GTY((skip)) left;
  struct lang_struct_tree * GTY((skip)) right;
  int GTY(()) key;
  string_type GTY(()) data;
};

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Struct containing array of pointers to unions */
struct GTY(()) container_struct {
  union data_union * GTY((skip)) union_ptrs[4];
  int_array counts;
  simple_callback handler;
};

/* Union containing struct with callback */
union GTY(()) mega_union {
  struct GTY(()) {
    int id;
    factory_callback create;
    user_struct_t * GTY((skip)) instance;
  } factory;
  struct container_struct container;
};

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
