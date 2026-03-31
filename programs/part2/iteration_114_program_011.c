#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED handling */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C struct */
struct my_base_struct GTY(()) {
    my_scalar_t field1;
    another_scalar_t field2;
    struct forward_declared_struct* forward_ptr;  /* TYPE_POINTER to undefined type */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION */
union my_union GTY(()) {
    my_scalar_t as_scalar;
    struct my_base_struct* as_struct_ptr;
    char* as_string;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Variable-length array with length attribute */
    int* variable_array GTY((length("var_len")));
    
    /* Zero-length array */
    char zero_array[0] GTY(());
    
    /* Pointer to array */
    int (*array_ptr)[5] GTY(());
    
    int var_len;  /* Length variable for variable_array */
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* regular_string GTY(());  /* Regular char pointer */
    char* counted_string GTY((length("str_len")));  /* String with length tracking */
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler GTY(());
    void (*regular_func_ptr)(void) GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void* lang_pointer;
};

/* Nested struct definition */
struct outer_container GTY(()) {
    struct my_base_struct base GTY(());
    
    /* Nested anonymous union */
    union {
        int nested_int;
        char nested_char;
    } GTY(()) nested_union;
    
    /* Pointer to self for recursion */
    struct outer_container* next GTY(());
    
    /* Array of structs */
    struct my_base_struct struct_array[5] GTY(());
};

/* Now define the forward-declared struct (was TYPE_UNDEFINED) */
struct forward_declared_struct GTY(()) {
    int defined_now;
    struct outer_container* container_ptr GTY(());
};

/* Circular reference between types */
struct type_a GTY(()) {
    struct type_b* b_ptr GTY(());
    int data_a;
};

struct type_b GTY(()) {
    struct type_a* a_ptr GTY(());
    struct type_a a_array[3] GTY(());
    int data_b;
};

/* Union containing various pointer types */
union pointer_union GTY(()) {
    struct my_base_struct* struct_ptr;
    struct array_container* array_ptr;
    struct string_container* string_ptr;
    void* void_ptr;
};

#endif /* TEST_GENGYPE_H */
