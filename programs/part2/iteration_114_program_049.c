#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY((user));

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY((tag("base"))) {
    my_scalar_t field1;
    another_scalar_t field2;
    struct undefined_struct *forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_managed_struct GTY((user)) {
    int user_field1;
    char *user_field2;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY((tag("data_union"))) {
    my_scalar_t scalar_val;
    struct base_struct *struct_ptr;
    char *string_val;
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY((tag("pointer_container"))) {
    struct base_struct *base_ptr;
    union data_union *union_ptr;
    struct pointer_container *next;  /* Recursive pointer */
    struct pointer_container *prev;  /* Another pointer */
};

/* TYPE_ARRAY: Structs containing arrays */
struct array_container GTY((tag("array_container"))) {
    /* Fixed-size array */
    struct base_struct fixed_array[10];
    
    /* Zero-length array */
    union data_union flexible_array[0];
    
    /* Array with length attribute */
    struct pointer_container *variable_array GTY((length("array_length")));
    int array_length;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY((tag("string_container"))) {
    char *regular_string;
    char *sized_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

struct callback_container GTY((tag("callback_container"))) {
    callback_func_t handler;
    void (*another_handler)(struct base_struct *) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((tag("lang_struct"), lang_struct (1))) {
    int lang_field1;
    void *lang_field2;
};

/* Nested types for additional complexity */
struct outer_struct GTY((tag("outer"))) {
    struct {
        int nested_field1;
        char *nested_field2 GTY((length("nested_len")));
        int nested_len;
    } inner GTY((tag("inner")));
    
    union {
        struct base_struct *as_base;
        struct pointer_container *as_pointer;
    } choice GTY((tag("choice")));
    
    /* Array of pointers to callbacks */
    callback_func_t callbacks[5];
};

/* Another forward declaration for circular reference */
struct circular_a GTY((tag("circular_a")));
struct circular_b GTY((tag("circular_b")));

struct circular_a {
    struct circular_b *link_to_b;
    int value_a;
};

struct circular_b {
    struct circular_a *link_to_a;
    struct circular_b *next_b;  /* Self-referential */
    int value_b;
};

/* Enumeration type */
typedef enum {
    MODE_A,
    MODE_B,
    MODE_C
} operation_mode_t GTY((user));

#endif /* TEST_GENGYPE_H */
