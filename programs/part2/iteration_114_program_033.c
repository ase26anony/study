#ifndef TEST_GENGTPYE_H
#define TEST_GENGTPYE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int my_other_scalar_t GTY((user));

/* TYPE_STRUCT: Standard C structs */
struct my_struct GTY((tag("my_struct"))) {
    int field1;
    char field2;
    my_scalar_t field3;
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user)) {
    int user_field1;
    double user_field2;
};

/* TYPE_UNION: Union with GTY members */
union my_union GTY((tag("my_union"))) {
    int int_val;
    double double_val;
    char* string_val;
    struct my_struct* struct_ptr;
};

/* TYPE_POINTER: Pointer types and recursive references */
struct pointer_struct GTY((tag("pointer_struct"))) {
    struct my_struct* direct_ptr GTY((skip));
    struct pointer_struct* self_ptr;  /* Recursive pointer */
    struct undefined_struct* forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_ARRAY: Arrays with various attributes */
struct array_struct GTY((tag("array_struct"))) {
    int fixed_array[10];
    struct my_struct* ptr_array[5] GTY((length("5")));
    int zero_length_array[0];
    char* variable_array GTY((length("var_len")));
    int var_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_struct GTY((tag("string_struct"))) {
    char* regular_string;
    char* counted_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char*) GTY((callback));

struct callback_struct GTY((tag("callback_struct"))) {
    callback_func_t handler;
    void (*direct_callback)(struct my_struct*) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((tag("lang_struct"), lang_struct(1))) {
    int lang_field1;
    void* lang_field2;
};

/* Nested structures for complex type graph */
struct container_struct GTY((tag("container"))) {
    /* Nested union */
    union {
        int nested_int;
        struct my_struct* nested_ptr;
    } nested_union GTY((tag("nested_union")));
    
    /* Nested struct */
    struct {
        int inner_field1;
        char inner_field2;
    } nested_struct GTY((tag("nested_struct")));
    
    /* Array of pointers to various types */
    void* mixed_array[3] GTY((length("3")));
};

/* Circular references */
struct node_a GTY((tag("node_a"))) {
    int value;
    struct node_b* next;
};

struct node_b GTY((tag("node_b"))) {
    int value;
    struct node_a* next;
};

/* Provide definition for forward-declared struct to avoid errors */
struct undefined_struct GTY((tag("undefined"))) {
    int defined_field;
    struct pointer_struct* ptr_field;
};

#endif /* TEST_GENGTPYE_H */
