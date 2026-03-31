#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* 
 * TYPE_UNDEFINED: Forward declaration without definition
 * This will increment nb_undefined
 */
struct undefined_struct;

/*
 * TYPE_SCALAR: Basic scalar types
 * These will increment nb_scalar
 */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/*
 * TYPE_STRING: String pointer type
 * This will increment nb_string
 */
typedef const char * GTY(()) string_t;

/*
 * TYPE_STRUCT: Regular struct types
 * These will increment nb_struct
 */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    string_t name;
    struct simple_struct * GTY(()) nested;
    int GTY(()) count;
};

/*
 * TYPE_USER_STRUCT: User-defined struct type
 * This will increment nb_user_struct
 */
typedef struct GTY(()) user_def_struct {
    int GTY(()) id;
    string_t GTY(()) description;
} user_struct_t;

/*
 * TYPE_UNION: Union type
 * This will increment nb_union
 */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    void * GTY(()) as_pointer;
};

/*
 * TYPE_POINTER: Pointer types
 * These will increment nb_pointer
 */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/*
 * TYPE_ARRAY: Array types
 * These will increment nb_array
 */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef string_t GTY(()) string_array_t[3];

/*
 * TYPE_CALLBACK: Function pointer types
 * These will increment nb_callback
 */
typedef void (* GTY(()) simple_callback_t)(int);
typedef int (* GTY(()) complex_callback_t)(struct simple_struct *, string_t);
typedef void (* GTY(()) callback_array_t[5])(void);

/*
 * TYPE_LANG_STRUCT: Language-specific struct with GTY markers
 * This will increment nb_lang_struct
 */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
    int GTY(()) lang_specific_field;
    string_t GTY(()) lang_name;
    struct lang_struct * GTY(()) next;
    struct lang_struct * GTY(()) prev;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container_struct {
    /* Contains scalar */
    scalar_int_t count;
    
    /* Contains string */
    string_t name;
    
    /* Contains struct */
    struct simple_struct nested_struct;
    
    /* Contains user struct */
    user_struct_t user_data;
    
    /* Contains union */
    union data_union data;
    
    /* Contains pointer */
    int_ptr_t int_ptr;
    
    /* Contains array */
    int_array_t numbers;
    
    /* Contains array of pointers */
    struct_ptr_t GTY(()) struct_ptrs[4];
    
    /* Contains callback */
    simple_callback_t callback;
    
    /* Contains lang struct */
    struct lang_struct * GTY(()) lang_chain;
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
