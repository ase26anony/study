#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macro definitions */
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
    scalar_int_t id;
    string_t name;
    struct simple_struct * GTY(()) nested;
};

/*
 * TYPE_USER_STRUCT: User-defined struct types
 * This will increment nb_user_struct
 */
typedef struct GTY(()) user_defined {
    int data;
    void * GTY(()) extra;
} user_struct_t;

/*
 * TYPE_UNION: Union type
 * This will increment nb_union
 */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
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
typedef union data_union GTY(()) union_array_t[3];

/*
 * TYPE_CALLBACK: Function pointer types
 * These will increment nb_callback
 */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) process_func_t)(const char *, void *);

/*
 * TYPE_LANG_STRUCT: Language-specific struct with GTY marker
 * This will increment nb_lang_struct
 */
struct GTY((user)) lang_specific_struct {
    int lang_specific_field;
    callback_t handler;
};

/* 
 * Complex nested type to ensure deep traversal
 * This contains multiple type categories
 */
struct GTY(()) container_struct {
    /* Scalar */
    scalar_int_t count;
    
    /* String */
    string_t description;
    
    /* Struct */
    struct simple_struct basic;
    
    /* User struct */
    user_struct_t user_data;
    
    /* Union */
    union data_union variant;
    
    /* Pointer */
    int_ptr_t int_pointer;
    
    /* Array */
    int_array_t numbers;
    
    /* Array of pointers */
    struct_ptr_t GTY((length("count"))) *struct_pointers;
    
    /* Callback */
    callback_t notify;
    
    /* Language struct */
    struct lang_specific_struct * GTY(()) lang_data;
    
    /* Forward declaration reference (undefined) */
    struct undefined_struct * GTY(()) future;
};

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
