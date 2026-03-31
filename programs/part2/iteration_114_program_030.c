#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct;

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRUCT: Standard C structs */
struct my_struct GTY(()) {
    my_scalar_t field1;
    int field2;
    struct my_struct *next;  /* Recursive pointer */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void *user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(()) {
    my_scalar_t scalar_val;
    struct my_struct *struct_ptr;
    char *string_val;
};

/* Forward declaration for circular reference */
struct forward_declared;

/* Struct with pointer to forward-declared type */
struct container_struct GTY(()) {
    struct forward_declared *fwd_ptr;
    union my_union data;
};

/* Now define the forward-declared struct */
struct forward_declared GTY(()) {
    int id;
    struct container_struct *container;
};

/* TYPE_ARRAY: Structs containing arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char flexible_array[0] GTY(());
    
    /* Array with length attribute */
    struct my_struct *variable_array GTY((length("array_len")));
    int array_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    /* String with length attribute */
    char *dynamic_string GTY((length("str_len")));
    int str_len;
    
    /* Another string field */
    const char *const_string GTY(());
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void *user_data GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct(1))) {
    int lang_data;
    void *lang_ptr;
};

/* Nested types for complexity */
struct outer_struct GTY(()) {
    struct {
        int nested_field;
        union {
            int nested_union_int;
            double nested_union_double;
        } nested_union;
    } nested_struct;
    
    struct array_container arrays;
    struct string_container strings;
};

/* Another struct with multiple pointer types */
struct pointer_network GTY(()) {
    struct my_struct **double_ptr;
    struct forward_declared *fwd;
    struct container_struct *container;
    struct outer_struct *outer;
};

#endif /* TEST_GENGYPE_H */
