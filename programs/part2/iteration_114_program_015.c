#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED (incomplete type) */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C struct */
struct my_struct GTY(()) {
    my_scalar_t scalar_field;
    another_scalar_t another_field;
    struct forward_declared_struct *forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled_struct GTY((user)) {
    int user_data;
    char user_char;
};

/* TYPE_UNION */
union my_union GTY(()) {
    my_scalar_t as_scalar;
    struct my_struct *as_struct_ptr;
    char as_char;
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY(()) {
    int fixed_array[10] GTY(());
    int variable_array[] GTY(());
    struct my_struct *struct_array[5] GTY(());
    int *pointer_array[3] GTY(());
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char *regular_string GTY((length("strlen($1)")));
    const char *const_string GTY((length("strlen($1)")));
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler;
    void (*another_handler)(struct my_struct *) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void *lang_pointer;
};

/* Nested struct for complex type graph */
struct outer_struct GTY(()) {
    struct {
        int nested_data;
        union {
            int nested_union_int;
            char nested_union_char;
        } nested_union GTY(());
    } nested_struct GTY(());
    
    struct array_container *array_ptr GTY(());
    struct string_container string_data GTY(());
};

/* Circular reference for TYPE_POINTER */
struct node_a GTY(()) {
    int data;
    struct node_b *next GTY(());
};

struct node_b GTY(()) {
    char data;
    struct node_a *next GTY(());
    struct node_a *prev GTY(());
};

/* Self-referential struct */
struct recursive_struct GTY(()) {
    int value;
    struct recursive_struct *next GTY(());
    struct recursive_struct *prev GTY(());
};

/* Now define the forward-declared struct (was TYPE_UNDEFINED) */
struct forward_declared_struct GTY(()) {
    int finally_defined;
    struct my_struct *back_ref GTY(());
};

/* Union containing various pointer types */
union pointer_union GTY(()) {
    struct my_struct *struct_ptr;
    struct forward_declared_struct *forward_ptr;
    struct recursive_struct *recursive_ptr;
    char **string_ptr_ptr;
};

/* Container with all types */
struct master_container GTY(()) {
    /* TYPE_SCALAR */
    my_scalar_t scalar_member;
    
    /* TYPE_STRUCT */
    struct my_struct struct_member;
    
    /* TYPE_USER_STRUCT */
    struct user_handled_struct *user_struct_ptr;
    
    /* TYPE_UNION */
    union my_union union_member;
    
    /* TYPE_POINTER */
    struct master_container *self_ptr GTY(());
    
    /* TYPE_ARRAY */
    struct array_container array_member;
    int multi_dim_array[3][4] GTY(());
    
    /* TYPE_STRING */
    char *dynamic_string GTY((length("strlen($1)")));
    
    /* TYPE_CALLBACK */
    callback_func_t callback_member;
    
    /* TYPE_LANG_STRUCT */
    struct lang_specific_struct *lang_struct_ptr;
    
    /* Circular reference */
    struct node_a *circular_start GTY(());
    
    /* Nested anonymous struct */
    struct {
        int anonymous_data;
        struct outer_struct *outer_ptr GTY(());
    } anonymous GTY(());
};

#endif /* TEST_GENGYPE_H */
