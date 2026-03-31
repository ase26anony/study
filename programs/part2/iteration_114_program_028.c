#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_UNDEFINED: Forward declaration (incomplete type) */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Standard C struct */
struct my_struct GTY(()) {
    my_scalar_t scalar_field;
    struct undefined_struct* undefined_ptr;  /* Pointer to undefined type */
    struct my_struct* next;  /* Recursive pointer */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_pointer;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(()) {
    my_scalar_t as_scalar;
    struct my_struct* as_struct;
    char* as_string;
};

/* TYPE_ARRAY: Struct containing various array types */
struct array_container GTY(()) {
    int fixed_array[10] GTY(());
    struct my_struct* variable_array GTY((length("array_len")));
    int zero_length_array[] GTY(());
    int array_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* dynamic_string GTY((length("str_len")));
    const char* const_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, char*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler GTY(());
    void (*direct_callback)(struct my_struct*) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void* lang_pointer;
};

/* Nested struct definition */
struct outer_container GTY(()) {
    struct my_struct inner_struct;
    union my_union inner_union;
    
    /* Nested anonymous struct */
    struct GTY(()) {
        int nested_data;
        struct my_struct* nested_ptr;
    } anonymous;
    
    /* Pointer to forward-declared type */
    struct undefined_struct* fwd_ptr;
};

/* Complete the forward-declared type */
struct undefined_struct GTY(()) {
    int finally_defined;
    struct my_struct* back_ref;
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_struct* struct_ptr_t GTY(());
typedef union my_union* union_ptr_t GTY(());
typedef struct array_container* array_ptr_t GTY(());

/* Circular reference structure */
struct circular_a GTY(());
struct circular_b GTY(());

struct circular_a GTY(()) {
    struct circular_b* b_ptr;
    int a_data;
};

struct circular_b GTY(()) {
    struct circular_a* a_ptr;
    int b_data;
};

#endif /* TEST_GENGYPE_H */
