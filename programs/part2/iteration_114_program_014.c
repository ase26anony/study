#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED (will remain incomplete) */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Standard C struct */
struct base_struct GTY(()) {
    my_scalar_t value;
    another_scalar_t count;
    struct undefined_struct* undefined_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled_struct GTY((user)) {
    int user_data;
    char* user_name;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY(()) {
    my_scalar_t as_scalar;
    struct base_struct* as_struct_ptr;
    char* as_string;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char flexible_array[0] GTY(());
    
    /* Array with length attribute */
    struct base_struct* variable_array GTY((length("array_length")));
    int array_length;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* dynamic_string GTY((length("str_len")));
    int str_len;
    
    const char* constant_string GTY(());
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char*) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler;
    void (*plain_func_ptr)(void) GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void* lang_private;
};

/* Nested structure for complex type graph */
struct outer_struct GTY(()) {
    /* TYPE_POINTER: Multiple pointer types */
    struct base_struct* base_ptr GTY(());
    union data_union* union_ptr GTY(());
    struct outer_struct* self_ptr GTY(());  /* Recursive pointer */
    
    /* Nested union */
    union {
        int nested_int;
        struct base_struct* nested_struct_ptr;
    } nested_union GTY(());
    
    /* Nested struct */
    struct {
        int inner_value;
        char inner_char;
    } nested_struct GTY(());
    
    /* Array of pointers */
    struct base_struct* ptr_array[5] GTY(());
    
    /* Pointer to array */
    int (*matrix_ptr)[10] GTY(());
};

/* Another struct with circular reference */
struct node GTY(()) {
    int data;
    struct node* next GTY(());
    struct node* prev GTY(());
};

/* Union containing struct with callback */
union complex_union GTY(()) {
    struct callback_container callbacks;
    struct string_container strings;
    struct outer_struct* outer;
};

#endif /* TEST_GENGTYPE_H */
