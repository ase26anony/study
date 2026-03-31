#ifndef TEST_GENGTPYE_H
#define TEST_GENGTPYE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY(());
typedef char char_scalar_t GTY((skip));

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY((tag("base"))) {
    my_scalar_t value;
    char_scalar_t flag;
};

/* Nested struct for complexity */
struct container GTY((tag("container"))) {
    struct base_struct base;
    int count;
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY((tag("data_union"))) {
    int int_val;
    double double_val;
    struct base_struct* struct_ptr;
};

/* TYPE_POINTER: Structs containing pointer fields */
struct pointer_holder GTY((tag("pointer_holder"))) {
    /* Recursive pointer */
    struct pointer_holder* next GTY((skip));
    
    /* Pointer to another struct */
    struct base_struct* data GTY((skip));
    
    /* Pointer to forward-declared undefined struct */
    struct undefined_struct* undefined_ptr GTY((skip));
    
    /* Circular reference */
    struct array_container* array_ref GTY((skip));
};

/* TYPE_ARRAY: Structs containing arrays */
struct array_container GTY((tag("array_container"))) {
    /* Fixed-size array */
    int fixed_array[10] GTY((skip));
    
    /* Zero-length array */
    char flexible_array[] GTY((skip));
    
    /* Pointer back to pointer_holder for circular reference */
    struct pointer_holder* ptr_holder GTY((skip));
};

/* TYPE_STRING: String type with length attribute */
struct string_holder GTY((tag("string_holder"))) {
    /* String with explicit length */
    char* dynamic_string GTY((length("str_len")));
    int str_len;
    
    /* Another string */
    const char* const_string GTY((skip));
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_holder GTY((tag("callback_holder"))) {
    callback_func handler;
    void* context;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((tag("lang"), lang_struct (1))) {
    int lang_data;
    void* lang_ptr;
};

/* Complex nested type definitions */
struct outer_struct GTY((tag("outer"))) {
    /* Nested union */
    union {
        int nested_int;
        struct base_struct nested_struct;
    } nested_data GTY((skip));
    
    /* Array of pointers */
    struct pointer_holder* ptr_array[5] GTY((skip));
    
    /* Pointer to array */
    int (*matrix_ptr)[10] GTY((skip));
};

/* Another forward declaration for more undefined types */
union undefined_union GTY((tag("undefined_union")));

#endif /* TEST_GENGTPYE_H */
