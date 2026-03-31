#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"

/* 
 * TYPE_UNDEFINED: Forward declaration without definition
 * This should increment nb_undefined
 */
struct undefined_struct;

/*
 * TYPE_SCALAR: Basic scalar types
 * These should increment nb_scalar
 */
typedef GTY(()) int scalar_int_t;
typedef GTY(()) float scalar_float_t;
typedef GTY(()) double scalar_double_t;
typedef GTY(()) char scalar_char_t;

/*
 * TYPE_STRING: String pointer type
 * This should increment nb_string
 */
typedef GTY(()) const char *string_ptr_t;

/*
 * TYPE_STRUCT: Regular struct types
 * These should increment nb_struct
 */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t id;
    string_ptr_t name;
    struct simple_struct *nested;
};

/*
 * TYPE_USER_STRUCT: User-defined struct types
 * These should increment nb_user_struct
 */
typedef struct GTY(()) user_defined_struct {
    int data;
    float value;
} user_struct_t;

/*
 * TYPE_UNION: Union types
 * This should increment nb_union
 */
union GTY(()) data_union {
    scalar_int_t int_val;
    scalar_float_t float_val;
    string_ptr_t str_val;
};

/*
 * TYPE_POINTER: Pointer types
 * These should increment nb_pointer
 */
typedef GTY(()) int *int_ptr_t;
typedef GTY(()) struct simple_struct *struct_ptr_t;
typedef GTY(()) union data_union *union_ptr_t;

/*
 * TYPE_ARRAY: Array types
 * These should increment nb_array
 */
typedef GTY(()) int int_array_t[10];
typedef GTY(()) struct simple_struct struct_array_t[5];
typedef GTY(()) union data_union union_array_t[3];

/*
 * TYPE_CALLBACK: Function pointer types
 * These should increment nb_callback
 */
typedef GTY(()) void (*simple_callback_t)(int);
typedef GTY(()) int (*complex_callback_t)(struct simple_struct *, string_ptr_t);

/*
 * TYPE_LANG_STRUCT: Language-specific structs with GTY markers
 * These should increment nb_lang_struct
 */
struct GTY((user)) lang_specific_struct {
    int tag;
    union GTY((desc ("%0.tag"))) {
        struct GTY((tag ("0"))) {
            int int_field;
        } int_case;
        struct GTY((tag ("1"))) {
            string_ptr_t str_field;
        } str_case;
    } GTY((default)) u;
};

/*
 * Complex nested type to ensure traversal of all categories
 * This contains struct, pointer, array, and union types
 */
struct GTY(()) container_struct {
    /* Scalar types */
    scalar_int_t count;
    scalar_float_t ratio;
    
    /* String type */
    string_ptr_t description;
    
    /* Struct type */
    struct simple_struct element;
    
    /* User struct type */
    user_struct_t user_data;
    
    /* Union type */
    union data_union variant;
    
    /* Pointer types */
    int_ptr_t int_pointer;
    struct_ptr_t struct_pointer;
    
    /* Array types */
    int_array_t numbers;
    struct_array_t elements;
    
    /* Callback type */
    simple_callback_t handler;
    
    /* Language struct type */
    struct lang_specific_struct *lang_data;
    
    /* Pointer to undefined struct */
    struct undefined_struct *undefined_ptr;
};

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
