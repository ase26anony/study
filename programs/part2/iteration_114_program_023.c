#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRUCT: Standard C structs */
struct my_struct GTY(()) {
    my_scalar_t field1;
    another_scalar field2;
    struct undefined_struct* forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_pointer;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(()) {
    my_scalar_t scalar_member;
    struct my_struct* struct_member;
    char* string_member;
};

/* TYPE_ARRAY: Struct containing various array types */
struct array_container GTY(()) {
    int fixed_array[10] GTY(());
    struct my_struct* ptr_array[5] GTY(());
    int zero_length_array[0] GTY(());
    char* variable_array GTY((length("len_field")));
    int len_field;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* regular_string GTY(());
    char* counted_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, char*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void (*another_handler)(void) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void* lang_pointer;
};

/* TYPE_POINTER: Complex pointer relationships */
struct pointer_network GTY(()) {
    struct my_struct* direct_ptr;
    struct pointer_network* self_ptr;  /* Self-referential */
    struct array_container* to_array;
    union my_union* to_union;
    struct undefined_struct* to_undefined;
};

/* Nested types for additional complexity */
struct outer_container GTY(()) {
    struct {
        int nested_data;
        struct my_struct* nested_ptr;
    } inner_struct GTY(());
    
    union {
        int nested_int;
        char* nested_string;
    } inner_union GTY(());
    
    struct pointer_network* network_ptr;
};

/* Circular references */
struct node_a GTY(()) {
    int data;
    struct node_b* next;
};

struct node_b GTY(()) {
    int value;
    struct node_a* prev;
    struct node_a* another_link;
};

#endif /* TEST_GENGYPE_H */
