#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"
#include "system.h"

/* ==================== UNDEFINED TYPES ==================== */
/* Forward declaration without definition - will be TYPE_UNDEFINED */
struct GTY(()) undefined_struct;
struct GTY(()) another_undefined;

/* ==================== SCALAR TYPES ==================== */
/* Basic scalar types - will be TYPE_SCALAR */
typedef GTY(()) int scalar_int_t;
typedef GTY(()) float scalar_float_t;
typedef GTY(()) double scalar_double_t;
typedef GTY(()) char scalar_char_t;
typedef GTY(()) long scalar_long_t;
typedef GTY(()) unsigned int scalar_uint_t;

/* ==================== STRING TYPE ==================== */
/* String pointer type - will be TYPE_STRING */
typedef GTY(()) const char *string_t;

/* ==================== STRUCT TYPES ==================== */
/* Regular struct - will be TYPE_STRUCT */
struct GTY(()) simple_struct {
  scalar_int_t field1;
  scalar_float_t field2;
  string_t field3;
};

/* Another struct with different composition */
struct GTY(()) complex_struct {
  struct simple_struct *nested;
  scalar_double_t values[5];
  int count;
};

/* ==================== USER STRUCT TYPES ==================== */
/* User-defined struct type - will be TYPE_USER_STRUCT */
typedef struct GTY(()) user_def_struct {
  int id;
  string_t name;
  struct user_def_struct *next;
} user_def_t;

/* Another user struct */
typedef struct GTY(()) user_container {
  user_def_t *items;
  int capacity;
  scalar_double_t *data;
} user_container_t;

/* ==================== UNION TYPES ==================== */
/* Union type - will be TYPE_UNION */
union GTY(()) data_union {
  scalar_int_t as_int;
  scalar_float_t as_float;
  scalar_double_t as_double;
  string_t as_string;
  void *as_pointer;
};

/* Tagged union */
typedef union GTY(()) tagged_union {
  int type;
  struct {
    int x, y;
  } point;
  struct {
    float radius;
    scalar_double_t area;
  } circle;
} shape_t;

/* ==================== POINTER TYPES ==================== */
/* Pointer typedefs - will be TYPE_POINTER */
typedef GTY(()) int *int_ptr_t;
typedef GTY(()) struct simple_struct *struct_ptr_t;
typedef GTY(()) union data_union *union_ptr_t;
typedef GTY(()) void (*void_func_ptr_t)(void);
typedef GTY(()) user_def_t **double_ptr_t;

/* ==================== ARRAY TYPES ==================== */
/* Array typedefs - will be TYPE_ARRAY */
typedef GTY(()) int int_array_t[10];
typedef GTY(()) scalar_float_t float_array_t[20];
typedef GTY(()) struct_ptr_t ptr_array_t[5];
typedef GTY(()) string_t string_array_t[3];

/* Multi-dimensional array */
typedef GTY(()) int matrix_t[4][4];

/* ==================== CALLBACK TYPES ==================== */
/* Function pointer typedefs - will be TYPE_CALLBACK */
typedef GTY(()) void (*simple_callback_t)(int);
typedef GTY(()) int (*complex_callback_t)(string_t, struct simple_struct*);
typedef GTY(()) void (*void_callback_t)(void);
typedef GTY(()) scalar_double_t (*compute_callback_t)(scalar_float_t, scalar_int_t);

/* Struct with callback member */
struct GTY(()) callback_container {
  simple_callback_t handler;
  complex_callback_t processor;
  void *user_data;
};

/* ==================== LANG STRUCT TYPES ==================== */
/* Language-specific structs with GTY markers - will be TYPE_LANG_STRUCT */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_struct {
  int lang_specific_field;
  string_t lang_name;
  struct lang_struct GTY((skip)) *next;
  struct lang_struct *prev;
};

/* Another language struct with different GTY options */
struct GTY((desc ("%1.type"), tag ("TYPE_ENUM"))) typed_lang_struct {
  int type;
  union {
    int int_val;
    float float_val;
    string_t str_val;
  } GTY((desc ("%0.type"))) value;
};

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Struct containing array of pointers to unions */
struct GTY(()) nested_complex {
  union_ptr_t union_array[8];
  int_array_t int_data;
  callback_container callbacks[2];
  shape_t shapes[4];
};

/* Union containing struct with array of function pointers */
union GTY(()) mega_union {
  struct GTY(()) inner_struct {
    compute_callback_t calculators[3];
    matrix_t transformation;
    user_container_t *user_data;
  } data;
  nested_complex *complex_data;
  lang_struct *lang_data;
};

/* ==================== FORWARD DECLARATIONS ==================== */
/* More undefined types for counting */
struct GTY(()) forward_declared;
typedef struct GTY(()) another_forward *another_forward_ptr_t;

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
