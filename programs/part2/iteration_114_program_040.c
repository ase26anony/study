#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct;

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t field1;
    another_scalar field2;
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY(()) {
    my_scalar_t as_scalar;
    struct base_struct* as_struct_ptr;
    char* as_string;
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY(()) {
    /* Pointer to scalar */
    my_scalar_t* scalar_ptr GTY(());
    
    /* Pointer to struct */
    struct base_struct* struct_ptr GTY(());
    
    /* Pointer to forward-declared struct (TYPE_UNDEFINED reference) */
    struct undefined_struct* undefined_ptr GTY(());
    
    /* Self-referential pointer */
    struct pointer_container* next GTY(());
    
    /* Pointer to union */
    union data_union* union_ptr GTY(());
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char zero_length_array[0] GTY(());
    
    /* Array with length attribute */
    struct base_struct* variable_array GTY((length("array_length")));
    int array_length;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    /* String field */
    char* dynamic_string GTY((length("str_len")));
    int str_len;
    
    /* Another string */
    const char* const_string GTY(());
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler GTY(());
    void* user_data GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct (1))) {
    int lang_data;
    void* lang_ptr;
};

/* Nested types for complexity */
struct outer_container GTY(()) {
    /* Nested struct */
    struct {
        int nested_data;
        struct pointer_container* ptr_field GTY(());
    } inner_struct GTY(());
    
    /* Nested union */
    union {
        int int_val;
        struct string_container* str_container GTY(());
    } inner_union GTY(());
    
    /* Array of pointers */
    struct base_struct* struct_array[5] GTY(());
    
    /* Pointer to array container */
    struct array_container* array_ptr GTY(());
};

/* Circular reference structure */
struct node_a GTY(()) {
    int data;
    struct node_b* partner GTY(());
};

struct node_b GTY(()) {
    int value;
    struct node_a* partner GTY(());
};

#endif /* TEST_GENGYPE_H */
