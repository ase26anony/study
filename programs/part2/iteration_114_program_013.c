#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY(());
typedef struct undefined_struct *undefined_ptr_t GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t field1;
    int field2 GTY(());
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled GTY((user)) {
    int user_data;
    void *user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(()) {
    struct base_struct *struct_ptr GTY(());
    my_scalar_t scalar_val;
    char *string_ptr GTY(());
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY(()) {
    struct base_struct *next GTY(());
    struct pointer_container *prev GTY(());
    union my_union *union_ptr GTY(());
    undefined_ptr_t undefined_ptr;  /* TYPE_UNDEFINED reference */
};

/* TYPE_ARRAY: Structs with arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    struct base_struct *flex_array[0] GTY(());
    
    /* Array with length attribute */
    char *string_array GTY((length("len")));
    int len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char *name GTY((length("name_len")));
    int name_len;
    
    const char *const_string GTY((length("const_len")));
    int const_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler GTY(());
    void *user_data GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct (1))) {
    int lang_field1;
    void *lang_field2 GTY(());
};

/* Nested types for complex interdependencies */
struct outer_container GTY(()) {
    /* Nested struct */
    struct {
        int nested_field1 GTY(());
        struct pointer_container *nested_ptr GTY(());
    } inner_struct;
    
    /* Nested union */
    union {
        struct array_container *array_ptr GTY(());
        struct string_container *string_ptr GTY(());
    } inner_union;
    
    /* Array of pointers */
    struct callback_container *callbacks[5] GTY(());
    
    /* Pointer to self for circular reference */
    struct outer_container *self GTY(());
};

/* Forward declaration for complex dependency */
struct forward_declared GTY(());

/* Struct that references forward-declared type */
struct uses_forward GTY(()) {
    struct forward_declared *fwd_ptr GTY(());
    struct uses_forward *next GTY(());
};

/* Now define the forward-declared struct */
struct forward_declared GTY(()) {
    struct uses_forward *user GTY(());
    int data GTY(());
};

/* Global type definitions for instantiation */
typedef struct base_struct *base_ptr_t GTY(());
typedef union my_union union_t GTY(());
typedef struct array_container array_t GTY(());
typedef struct string_container string_t GTY(());

#endif /* TEST_GENGYPE_H */
