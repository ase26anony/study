#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED - will be defined later */
struct forward_declared GTY(());

/* TYPE_STRUCT: Standard C struct */
struct base_struct GTY(()) {
    my_scalar_t id;
    struct forward_declared* next;  /* Pointer to forward-declared type */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION */
union data_union GTY(()) {
    int int_val;
    double double_val;
    char* string_val;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(()) {
    int fixed_array[10];
    int variable_array GTY((length("len"))) [];
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
    void* data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct(1))) {
    int lang_id;
    void* lang_data;
};

/* Now define the forward-declared struct (was TYPE_UNDEFINED) */
struct forward_declared GTY(()) {
    int value;
    struct base_struct* base_ptr;  /* Circular reference */
    union data_union data;
};

/* TYPE_POINTER: Struct focusing on pointer relationships */
struct pointer_network GTY(()) {
    struct base_struct* base_ptr;
    struct forward_declared* forward_ptr;
    struct pointer_network* self_ptr;  /* Self-referential */
    struct array_container* array_ptr;
};

/* Nested struct/union for complexity */
struct outer_container GTY(()) {
    struct {
        int nested_id;
        struct forward_declared* nested_ptr;
    } inner_struct GTY(());
    
    union {
        int choice_a;
        struct base_struct* choice_b;
    } inner_union GTY(());
    
    struct pointer_network* network;
};

/* Array of pointers */
typedef struct base_struct* base_ptr_array_t[5] GTY(());

/* Complex recursive type definition */
struct recursive_node GTY(()) {
    int data;
    struct recursive_node* left;
    struct recursive_node* right;
    struct recursive_node* parent;
};

#endif /* TEST_GENGYPE_H */
