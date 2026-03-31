#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_UNDEFINED: Forward declaration (incomplete type) */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t value;
    struct undefined_struct* forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled GTY((user)) {
    int user_data;
    void* user_pointer;
};

/* TYPE_UNION */
union data_union GTY(()) {
    my_scalar_t as_scalar;
    struct base_struct* as_struct;
    char* as_string;
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY(()) {
    int fixed_array[10];
    int variable_array GTY((length("len")));
    int zero_length_array[];
    int len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* regular_string;
    char* counted_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, char*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void (*regular_func)(void);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct (1))) {
    int lang_data;
    void* lang_pointer;
};

/* TYPE_POINTER: Complex pointer relationships */
struct pointer_network GTY(()) {
    struct pointer_network* self_ptr;      /* Self-referential */
    struct base_struct* to_base;
    union data_union* to_union;
    struct array_container* to_array;
    struct undefined_struct** double_ptr;  /* Pointer to pointer */
};

/* Nested struct/union combinations */
struct outer_container GTY(()) {
    struct {
        int nested_data;
        struct pointer_network* network;
    } inner_struct GTY(());
    
    union {
        my_scalar_t scalar_val;
        struct string_container* str_container;
    } inner_union GTY(());
    
    struct array_container arrays[5];
};

/* Now define the previously undefined struct */
struct undefined_struct GTY(()) {
    int finally_defined;
    struct outer_container* outer_link;
};

/* Circular reference completion */
struct circular_ref GTY(()) {
    struct pointer_network* network;
    struct undefined_struct* undefined;
    struct circular_ref* next;
};

#endif /* TEST_GENGTYPE_H */
