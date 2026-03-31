#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY((skip));

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY((tag("base"))) {
    my_scalar_t value;
    struct undefined_struct* undefined_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union with GTY members */
union data_union GTY((tag("data_union"))) {
    int int_val;
    double double_val;
    char* string_val;
    struct base_struct* struct_ptr;
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY((tag("pointer_container"))) {
    /* Simple pointer */
    struct base_struct* simple_ptr GTY((tag("ptr1")));
    
    /* Pointer to pointer */
    struct base_struct** double_ptr;
    
    /* Pointer to union */
    union data_union* union_ptr;
    
    /* Self-referential pointer */
    struct pointer_container* next GTY((tag("next")));
    
    /* Circular reference */
    struct array_container* array_ref;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY((tag("array_container"))) {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Zero-length array */
    char flexible_array[0];
    
    /* Array with length attribute */
    struct base_struct* variable_array GTY((length("array_length")));
    int array_length;
    
    /* Pointer back for circular reference */
    struct pointer_container* pointer_ref;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY((tag("string_container"))) {
    /* String with length attribute */
    char* dynamic_string GTY((length("str_len")));
    int str_len;
    
    /* Array of strings */
    char* string_array[5];
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY((tag("callback_container"))) {
    callback_func_t handler;
    void* user_data;
    
    /* Another callback as struct member */
    int (*another_callback)(const char*) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((tag("lang_struct"), lang_struct(1))) {
    int lang_data;
    void* lang_pointer;
};

/* Nested types for additional complexity */
struct outer_container GTY((tag("outer"))) {
    /* Nested struct */
    struct {
        int nested_data;
        char nested_char;
    } inner_struct;
    
    /* Nested union */
    union {
        int nested_int;
        struct base_struct* nested_ptr;
    } inner_union;
    
    /* Array of structs */
    struct base_struct struct_array[3];
    
    /* Pointer to callback */
    callback_func_t nested_callback;
};

/* Forward declaration that WILL be defined (not TYPE_UNDEFINED) */
struct forward_declared GTY((tag("forward")));

/* Struct that references forward-declared type */
struct uses_forward GTY((tag("uses_forward"))) {
    struct forward_declared* fwd_ptr;
    int some_data;
};

/* Now define the forward-declared struct */
struct forward_declared GTY((tag("forward"))) {
    struct uses_forward* back_ptr;  /* Circular reference */
    float forward_data;
};

/* Enum type (also scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum_t GTY((user));

#endif /* TEST_GENGYPE_H */
