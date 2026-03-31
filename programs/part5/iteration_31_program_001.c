#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;

/* Scalar types (TYPE_SCALAR) */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* String type (TYPE_STRING) */
typedef const char * GTY(()) string_t;

/* Basic struct types (TYPE_STRUCT) */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

typedef struct GTY(()) tagged_struct {
    int x;
    double y;
    string_t name;
} tagged_struct_t;

/* User struct (TYPE_USER_STRUCT) */
typedef struct GTY(()) user_defined {
    int id;
    char data[256];
} user_defined_t;

/* Union type (TYPE_UNION) */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    char * GTY((skip)) string_val;
};

/* Pointer types (TYPE_POINTER) */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types (TYPE_ARRAY) */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* Callback/function pointer types (TYPE_CALLBACK) */
typedef void (* GTY(()) callback_t)(int, const char*);
typedef int (* GTY(()) compare_func_t)(const void*, const void*);

/* Language-specific struct with GTY markers (TYPE_LANG_STRUCT) */
struct GTY((user)) lang_specific_struct {
    int lang_specific_field;
    void * GTY((skip)) opaque_data;
    callback_t handler;
};

/* Complex nested types to ensure traversal */
struct GTY(()) complex_nested {
    /* Contains array of pointers */
    struct_ptr_t * GTY(()) ptr_array[8];
    
    /* Contains union */
    union data_union data;
    
    /* Contains callback */
    callback_t notify;
    
    /* Nested struct */
    struct GTY(()) inner_struct {
        int inner_field;
        int_array_t numbers;
    } inner;
    
    /* Pointer to array */
    int (* GTY(()) matrix_ptr)[4][4];
};

/* Another complex type mixing different categories */
typedef union GTY(()) mixed_container {
    struct GTY(()) {
        int type;
        union data_union payload;
    } variant;
    
    struct GTY(()) {
        callback_t handlers[3];
        string_t names[3];
    } callbacks;
} mixed_container_t;

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
