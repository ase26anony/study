#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED (incomplete type) */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t field1;
    another_scalar_t field2;
    struct forward_declared_struct* ptr_field; /* Pointer to forward-declared */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION */
union my_union GTY(()) {
    my_scalar_t as_scalar;
    struct base_struct* as_struct_ptr;
    char* as_string;
};

/* TYPE_ARRAY: Array types */
struct array_container GTY(()) {
    int fixed_array[10] GTY(());
    int variable_array[] GTY(());
    struct base_struct* ptr_array[5] GTY(());
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* dynamic_string GTY((length("str_len")));
    int str_len;
    const char* const_string GTY(());
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler GTY(());
    void (*plain_func_ptr)(void) GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct(1))) {
    int lang_data;
    void* lang_ptr;
};

/* Nested struct/union for complexity */
struct outer_container GTY(()) {
    struct base_struct nested_struct;
    union my_union nested_union;
    struct {
        int anonymous_member;
        struct outer_container* self_ptr; /* Recursive reference */
    } inner_anon_struct GTY(());
};

/* Now define the forward-declared struct (completes TYPE_UNDEFINED) */
struct forward_declared_struct GTY(()) {
    int data;
    struct base_struct* back_ptr; /* Circular reference */
    struct outer_container* outer_ptr;
};

/* TYPE_POINTER: Additional pointer types */
typedef struct base_struct* base_ptr_t GTY(());
typedef union my_union* union_ptr_t GTY(());

/* Complex pointer network */
struct pointer_network GTY(()) {
    struct forward_declared_struct* fwd_ptr;
    struct outer_container* outer_ptr;
    struct pointer_network* next; /* Linked list */
    struct pointer_network* prev;
};

/* Zero-length array at end of struct */
struct zla_struct GTY(()) {
    int count;
    int data[] GTY(());
};

#endif /* TEST_GENGYPE_H */
