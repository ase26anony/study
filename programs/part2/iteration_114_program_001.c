#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_UNDEFINED: Forward declaration that will remain undefined */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t scalar_field;
    int another_field;
};

/* Nested struct for complexity */
struct container_struct GTY(()) {
    struct base_struct base;
    int extra_data;
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_pointer;
};

/* TYPE_UNION: Union with GTY members */
union data_union GTY(()) {
    int int_val;
    double double_val;
    struct base_struct* struct_ptr;
};

/* TYPE_POINTER: Pointer types and recursive references */
struct node_struct GTY(()) {
    int data;
    struct node_struct* next GTY(());  /* Pointer to same type */
    struct undefined_struct* undefined_ptr GTY(());  /* Pointer to undefined */
};

/* Forward declaration for circular reference */
struct forward_declared GTY(());

/* Another struct that points to forward declared */
struct pointer_holder GTY(()) {
    struct forward_declared* fwd_ptr GTY(());
    struct node_struct* node_ptr GTY(());
};

/* Now define the forward declared struct */
struct forward_declared GTY(()) {
    int value;
    struct pointer_holder* holder GTY(());
};

/* TYPE_ARRAY: Arrays with various attributes */
struct array_container GTY(()) {
    int fixed_array[10];
    struct base_struct* ptr_array[5] GTY(());
    int zero_length_array[];
};

/* Array with length attribute */
struct dynamic_array GTY(()) {
    int count;
    int* data_array GTY((length("count")));
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* regular_string;
    char* counted_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void (*another_handler)(void) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct(1))) {
    int lang_data;
    void* lang_pointer;
};

/* Complex nested type combining multiple kinds */
struct master_container GTY(()) {
    /* Scalar */
    my_scalar_t scalar;
    
    /* Struct */
    struct base_struct embedded_struct;
    
    /* Union */
    union data_union data;
    
    /* Pointer */
    struct node_struct* node_pointer GTY(());
    
    /* Array */
    int number_array[20];
    
    /* String */
    char* description GTY((length("desc_len")));
    int desc_len;
    
    /* Callback */
    callback_func callback GTY((callback));
    
    /* Nested struct with pointer to parent type */
    struct {
        int nested_data;
        struct master_container* parent_ref GTY(());
    } nested;
    
    /* Zero-length array at end */
    char flexible_array[];
};

#endif /* TEST_GENGYPE_H */
