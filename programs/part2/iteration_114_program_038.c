#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_UNDEFINED: Forward declaration that won't be defined */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY((skip));

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY((tag("base"))) {
    my_scalar_t value;
    struct undefined_struct* GTY((skip)) undef_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user)) {
    int data;
    void* GTY((skip)) extra;
};

/* TYPE_UNION: Union with GTY members */
union data_union GTY((tag("data_union"))) {
    int int_val;
    char* GTY((length)) str_val;  /* TYPE_STRING */
    struct base_struct* GTY((skip)) struct_ptr;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY((tag("array_container"))) {
    /* Fixed-size array */
    int fixed_array[10] GTY((tag("fixed_arr")));
    
    /* Zero-length array */
    char zero_length_array[0] GTY((tag("zla")));
    
    /* Variable-length array with length attribute */
    int* GTY((length("var_len"))) variable_array;
    unsigned int var_len;
};

/* TYPE_POINTER: Struct with pointer fields creating circular references */
struct pointer_node GTY((tag("pointer_node"))) {
    int id;
    struct pointer_node* GTY((skip)) next;  /* Self-referential pointer */
    struct pointer_node* GTY((skip)) prev;  /* Another pointer for circular ref */
};

/* TYPE_STRING: Explicit string type */
struct string_container GTY((tag("string_container"))) {
    char* GTY((length("str_len"))) dynamic_string;
    unsigned int str_len;
    const char* GTY((length)) constant_string;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY((tag("callback_container"))) {
    callback_func_t handler GTY((tag("handler")));
    void* GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((tag("lang_struct"), lang_struct(1))) {
    int lang_data;
    void* GTY((skip)) lang_extra;
};

/* Nested types for complex interdependencies */
struct outer_container GTY((tag("outer"))) {
    /* Nested struct */
    struct inner_struct GTY((tag("inner"))) {
        int inner_data;
        struct outer_container* GTY((skip)) parent;
    } nested;
    
    /* Nested union */
    union {
        int option_a;
        struct inner_struct* GTY((skip)) option_b;
    } choice GTY((tag("choice")));
    
    /* Array of pointers */
    struct inner_struct* GTY((skip)) ptr_array[5];
};

/* Complete the forward declaration to create TYPE_UNDEFINED then TYPE_STRUCT */
struct undefined_struct GTY((tag("undefined_completed"))) {
    int finally_defined;
    struct base_struct* GTY((skip)) base_ref;
};

/* Function pointer typedefs for more callback coverage */
typedef int (*compare_func_t)(const void*, const void*) GTY((callback));

#endif /* TEST_GENGTYPE_H */
