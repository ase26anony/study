#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
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
typedef long GTY(()) scalar_long_t;

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
    int a;
    float b;
};

typedef struct GTY(()) tagged_struct {
    int id;
    const char *name;
} tagged_struct_t;

/*
 * TYPE_USER_STRUCT: User-defined struct types
 * These will increment nb_user_struct
 */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
};

typedef struct GTY((user)) user_defined_t {
    int counter;
    struct user_defined_struct *next;
} user_defined_t;

/*
 * TYPE_UNION: Union types
 * This will increment nb_union
 */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    char *string_val;
};

typedef union GTY(()) typed_union_t {
    int as_int;
    void *as_ptr;
} typed_union_t;

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
typedef int (* GTY(()) compare_func_t)(const void *, const void *);
typedef void (* GTY(()) void_callback_t)(void);

/*
 * TYPE_LANG_STRUCT: Language-specific structs with GTY markers
 * These will increment nb_lang_struct
 */
struct GTY((desc("%0"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
    int lang_specific;
    struct lang_struct * GTY((skip)) next;
    struct lang_struct *prev;
};

typedef struct GTY((tag("LANG_TYPE"))) another_lang_struct {
    int type_code;
    union data_union data;
} lang_struct_t;

/*
 * Complex nested types to ensure traversal of all categories
 */
struct GTY(()) complex_nested {
    /* Contains scalar */
    int scalar_member;
    
    /* Contains string */
    const char * GTY((length("str_len"))) string_member;
    int str_len;
    
    /* Contains struct */
    struct simple_struct nested_struct;
    
    /* Contains union */
    union data_union nested_union;
    
    /* Contains pointer */
    int *pointer_member;
    
    /* Contains array */
    int array_member[5];
    
    /* Contains callback */
    callback_t callback_member;
    
    /* Contains lang_struct */
    struct lang_struct *lang_struct_member;
};

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
