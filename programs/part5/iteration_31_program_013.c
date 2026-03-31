#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
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
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/*
 * TYPE_STRING: String pointer type
 * This should increment nb_string
 */
typedef const char * GTY(()) string_t;

/*
 * TYPE_STRUCT: Regular struct types
 * These should increment nb_struct
 */
struct GTY(()) simple_struct {
    scalar_int_t id;
    scalar_float_t value;
};

struct GTY(()) complex_struct {
    scalar_int_t count;
    string_t name;
    struct simple_struct * GTY((skip)) data;
};

/*
 * TYPE_USER_STRUCT: User-defined struct types
 * These should increment nb_user_struct
 */
typedef struct GTY(()) user_defined {
    scalar_int_t uid;
    scalar_char_t initial;
    string_t username;
} user_struct_t;

/*
 * TYPE_UNION: Union type
 * This should increment nb_union
 */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    void * GTY((skip)) as_ptr;
};

/*
 * TYPE_POINTER: Pointer types
 * These should increment nb_pointer
 */
typedef scalar_int_t * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) simple_struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/*
 * TYPE_ARRAY: Array types
 * These should increment nb_array
 */
typedef scalar_int_t GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef string_t GTY(()) string_array_t[3];

/*
 * TYPE_CALLBACK: Function pointer types
 * These should increment nb_callback
 */
typedef void (* GTY(()) callback_t)(scalar_int_t, string_t);
typedef scalar_int_t (* GTY(())) compute_t)(scalar_float_t, scalar_float_t);

/*
 * TYPE_LANG_STRUCT: Language-specific struct with GC roots
 * This should increment nb_lang_struct
 */
struct GTY((user)) lang_specific_struct {
    scalar_int_t tag;
    union data_union GTY((desc("%0.tag"))) value;
    callback_t GTY((skip)) handler;
};

/*
 * Complex nested type to ensure traversal of all categories
 */
struct GTY(()) container_struct {
    /* Scalar members */
    scalar_int_t id;
    scalar_float_t weight;
    
    /* String member */
    string_t description;
    
    /* Struct member */
    struct simple_struct base;
    
    /* User struct member */
    user_struct_t user;
    
    /* Union member */
    union data_union data;
    
    /* Pointer members */
    int_ptr_t numbers;
    simple_struct_ptr_t next;
    
    /* Array members */
    int_array_t scores;
    struct_array_t items;
    
    /* Callback member */
    callback_t notify;
    
    /* Language struct member */
    struct lang_specific_struct * GTY(()) lang_data;
    
    /* Nested undefined type */
    struct undefined_struct * GTY((skip)) future;
};

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
