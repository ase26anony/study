#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will remain undefined */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t field1;
    another_scalar field2;
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(()) {
    my_scalar_t as_scalar;
    struct base_struct* as_struct_ptr;
    char* as_string;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char zero_length_array[0] GTY(());
    
    /* Variable-length array with length attribute */
    int* variable_array GTY((length("var_len")));
    size_t var_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* regular_string GTY(());
    char* counted_string GTY((length("str_len")));
    size_t str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler GTY(());
    void* user_data GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_field1;
    void* lang_field2;
};

/* TYPE_POINTER: Complex pointer relationships */
struct pointer_network GTY(()) {
    /* Pointer to forward-declared undefined struct */
    struct undefined_struct* to_undefined GTY(());
    
    /* Self-referential pointer */
    struct pointer_network* self_ptr GTY(());
    
    /* Pointer to another struct type */
    struct base_struct* to_base GTY(());
    
    /* Pointer to union */
    union my_union* to_union GTY(());
    
    /* Pointer to array container */
    struct array_container* to_array GTY(());
    
    /* Pointer to callback container */
    struct callback_container* to_callback GTY(());
    
    /* Pointer to lang struct */
    struct lang_specific_struct* to_lang_struct GTY(());
};

/* Nested struct containing another struct */
struct outer_struct GTY(()) {
    struct base_struct inner GTY(());
    struct pointer_network* network GTY(());
    
    /* Nested anonymous union */
    union {
        int option_a;
        struct base_struct* option_b GTY(());
    } choice GTY(());
};

/* Another forward declaration for circular reference */
struct circular_a GTY(());

struct circular_b GTY(()) {
    struct circular_a* link GTY(());
    int data;
};

struct circular_a GTY(()) {
    struct circular_b* link GTY(());
    float value;
};

#endif /* TEST_GENGYPE_H */
