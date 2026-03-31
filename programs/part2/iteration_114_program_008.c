#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());
typedef char char_scalar_t GTY(());

/* Forward declarations for TYPE_UNDEFINED handling */
struct forward_declared_struct GTY(());
union forward_declared_union GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t field1;
    another_scalar_t field2;
    char_scalar_t field3;
};

/* Nested struct for complexity */
struct outer_struct GTY(()) {
    struct base_struct inner;
    int extra_field;
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_pointer;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY(()) {
    my_scalar_t as_scalar;
    struct base_struct* as_struct_ptr;
    char* as_string;
};

/* TYPE_POINTER: Struct containing pointer fields with circular references */
struct pointer_container GTY(()) {
    struct forward_declared_struct* forward_ptr;  /* TYPE_UNDEFINED initially */
    struct pointer_container* self_ptr;           /* Recursive pointer */
    struct base_struct* base_ptr;
    union data_union* union_ptr;
};

/* TYPE_ARRAY: Structs containing various array types */
struct array_container GTY(()) {
    int fixed_array[10];
    struct base_struct* ptr_array[5];
    int zero_length_array[0];  /* Zero-length array */
    int variable_length_array GTY((length("vl_length")));
    int vl_length;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* regular_string;
    char* counted_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler;
    void (*direct_callback)(struct base_struct*) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct(1))) {
    int lang_data;
    void* lang_pointer;
};

/* Complete the forward declarations (now TYPE_STRUCT/TYPE_UNION) */
struct forward_declared_struct GTY(()) {
    struct pointer_container* container;
    int value;
};

union forward_declared_union GTY(()) {
    int int_val;
    struct forward_declared_struct* struct_ptr;
};

/* Complex nested type with multiple interdependencies */
struct master_container GTY(()) {
    struct base_struct base;
    union data_union data;
    struct pointer_container* pointers;
    struct array_container arrays;
    struct string_container strings;
    struct callback_container callbacks;
    struct lang_specific_struct* lang_struct;
    struct user_handled_struct* user_struct;
    struct forward_declared_struct forward_instance;
};

#endif /* TEST_GENGYPE_H */
