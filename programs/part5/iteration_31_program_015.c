#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* ==================== SCALAR TYPES ==================== */
/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef long GTY(()) scalar_long_t;

/* ==================== STRING TYPE ==================== */
/* TYPE_STRING: String pointer type */
typedef const char * GTY(()) string_t;

/* ==================== UNDEFINED TYPE ==================== */
/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) undefined_struct;
typedef struct undefined_struct * GTY(()) undefined_ptr_t;

/* ==================== STRUCT TYPES ==================== */
/* TYPE_STRUCT: Regular struct types */

/* Simple struct */
struct GTY(()) simple_struct {
    int GTY(()) field1;
    float GTY(()) field2;
    string_t GTY(()) name;
};

/* Nested struct */
struct GTY(()) outer_struct {
    struct simple_struct GTY(()) inner;
    int GTY(()) count;
};

/* Struct with array member */
struct GTY(()) array_struct {
    int GTY(()) values[10];
    int GTY(()) size;
};

/* ==================== USER STRUCT ==================== */
/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct simple_struct GTY(()) user_struct_t;

/* ==================== UNION TYPE ==================== */
/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int GTY(()) int_val;
    float GTY(()) float_val;
    double GTY(()) double_val;
    char GTY(()) char_val;
    string_t GTY(()) string_val;
};

/* ==================== POINTER TYPES ==================== */
/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;
typedef void * GTY(()) void_ptr_t;

/* Pointer to pointer */
typedef int_ptr_t * GTY(()) int_ptr_ptr_t;

/* ==================== ARRAY TYPES ==================== */
/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[8];
typedef string_t GTY(()) string_array_t[3];

/* Multi-dimensional array */
typedef int GTY(()) matrix_t[3][3];

/* ==================== CALLBACK TYPES ==================== */
/* TYPE_CALLBACK: Function pointer types */

/* Simple callback */
typedef void (* GTY(()) simple_callback_t)(int);

/* Callback with return value */
typedef int (* GTY(()) compute_callback_t)(int, int);

/* Callback with struct parameter */
typedef void (* GTY(()) struct_callback_t)(struct simple_struct *);

/* Callback returning pointer */
typedef string_t (* GTY(()) string_callback_t)(void);

/* ==================== LANG STRUCT ==================== */
/* TYPE_LANG_STRUCT: Language-specific struct with GTY markers */

/* Tree-like structure (common in GCC) */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
    int GTY(()) type;
    string_t GTY(()) name;
    struct lang_struct * GTY((skip)) next;
    struct lang_struct * GTY(()) prev;
    union data_union GTY(()) data;
};

/* Another lang struct with nested callbacks */
struct GTY(()) parser_state {
    int GTY(()) line;
    int GTY(()) column;
    simple_callback_t GTY(()) error_handler;
    struct_callback_t GTY(()) node_handler;
    struct lang_struct * GTY(()) current_node;
};

/* ==================== COMPLEX NESTED TYPES ==================== */

/* Struct containing array of pointers to unions */
struct GTY(()) container_struct {
    union data_union * GTY(()) items[10];
    int GTY(()) item_count;
    simple_callback_t GTY(()) processor;
};

/* Union containing struct with callback */
union GTY(()) complex_union {
    struct {
        int GTY(()) id;
        compute_callback_t GTY(()) calculator;
    } GTY(()) calculator_data;
    struct {
        string_t GTY(()) text;
        string_callback_t GTY(()) formatter;
    } GTY(()) text_data;
};

/* ==================== TYPE WITH ALL CATEGORIES ==================== */

/* Master struct that includes examples of all type categories */
struct GTY(()) master_type {
    /* Scalar */
    scalar_int_t GTY(()) id;
    
    /* String */
    string_t GTY(()) description;
    
    /* Struct */
    struct simple_struct GTY(()) data;
    
    /* User struct */
    user_struct_t GTY(()) user_data;
    
    /* Union */
    union data_union GTY(()) variant;
    
    /* Pointer */
    int_ptr_t GTY(()) numbers;
    
    /* Array */
    int_array_t GTY(()) values;
    
    /* Callback */
    simple_callback_t GTY(()) handler;
    
    /* Lang struct */
    struct lang_struct * GTY(()) lang_node;
    
    /* Pointer to undefined */
    undefined_ptr_t GTY(()) undefined_ref;
    
    /* Nested container */
    struct container_struct GTY(()) container;
};

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
