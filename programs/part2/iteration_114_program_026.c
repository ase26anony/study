#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_STRING: String type using GTY((length)) */
struct string_container {
    char * GTY((length("str_len"))) string_field;
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

/* Forward declarations for complex dependencies */
struct forward_declared;
struct recursive_struct;

/* TYPE_STRUCT: Standard C struct */
struct base_struct GTY(()) {
    my_scalar_t scalar_field;
    struct string_container *string_ptr;
    callback_func_t callback_field;
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user)) {
    int user_data;
    void *user_pointer;
};

/* TYPE_UNION: Union with GTY-tagged members */
union variant_union GTY(()) {
    my_scalar_t as_scalar;
    struct base_struct *as_struct;
    char * GTY((length("str_len"))) as_string;
    int str_len;
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    struct base_struct *flex_array GTY((length("flex_len")));
    int flex_len;
    
    /* Pointer to array */
    int (*array_ptr)[5] GTY(());
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY(()) {
    /* Pointer to scalar */
    my_scalar_t *scalar_ptr;
    
    /* Pointer to struct */
    struct base_struct *struct_ptr;
    
    /* Pointer to union */
    union variant_union *union_ptr;
    
    /* Recursive pointer */
    struct recursive_struct *recursive_ptr;
    
    /* Pointer to forward-declared struct */
    struct forward_declared *forward_ptr;
    
    /* Pointer to array */
    struct array_container *array_ptr;
    
    /* Double pointer */
    struct base_struct **double_ptr;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void *lang_pointer;
};

/* Now define the forward-declared struct */
struct forward_declared GTY(()) {
    int data;
    struct pointer_container *back_ref;
};

/* Define the recursive struct */
struct recursive_struct GTY(()) {
    int level;
    struct recursive_struct *next;
    struct pointer_container *container;
};

/* Nested type definitions */
struct outer_container GTY(()) {
    /* Nested struct */
    struct {
        int nested_data;
        struct base_struct *nested_ptr;
    } inner_struct GTY(());
    
    /* Nested union */
    union {
        int nested_int;
        struct base_struct *nested_struct_ptr;
    } inner_union GTY(());
    
    /* Array of structs */
    struct base_struct struct_array[5] GTY(());
    
    /* Pointer to nested type */
    struct pointer_container *ptr_field;
};

/* Function pointer typedefs with different attributes */
typedef void (*simple_func_t)(void) GTY(());
typedef int (*complex_callback_t)(struct base_struct *, union variant_union *) GTY((callback));

#endif /* TEST_GENGYPE_H */
