#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED and circular references */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct my_struct GTY(()) {
    my_scalar_t scalar_field;
    struct forward_declared_struct *forward_ptr;
    struct nested_struct *nested;
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void *user_pointer;
};

/* TYPE_UNION */
union my_union GTY(()) {
    my_scalar_t as_scalar;
    struct my_struct *as_struct;
    char *as_string;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char flexible_array[0] GTY(());
    
    /* Array with length attribute */
    struct my_struct *variable_array GTY((length("array_length")));
    int array_length;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char *regular_string;
    char *counted_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler;
    void (*regular_func_ptr)(void);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void *lang_pointer;
};

/* Nested struct definition */
struct nested_struct GTY(()) {
    struct my_struct *parent;
    union my_union data;
};

/* Complete the forward declaration - this creates TYPE_UNDEFINED during processing */
struct forward_declared_struct GTY(()) {
    struct my_struct *back_ref;
    struct nested_struct nested;
};

/* TYPE_POINTER: Create pointer relationships */
struct pointer_network GTY(()) {
    struct my_struct *to_struct;
    struct user_handled_struct *to_user;
    union my_union *to_union;
    struct array_container *to_array;
    struct string_container *to_string;
    struct callback_container *to_callback;
    struct lang_specific_struct *to_lang;
    struct forward_declared_struct *to_forward;
    struct pointer_network *self_ref;  /* Circular reference */
};

/* Complex nested type */
struct master_container GTY(()) {
    struct my_struct base;
    union {
        struct array_container arr;
        struct string_container str;
    } variant GTY(());
    
    struct {
        callback_func_t cb;
        struct pointer_network *net;
    } inner GTY(());
};

#endif /* TEST_GENGYPE_H */
