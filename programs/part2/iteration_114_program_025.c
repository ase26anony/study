#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED (incomplete type) */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct my_struct GTY(()) {
    my_scalar_t field1;
    another_scalar_t field2;
    struct forward_declared_struct* forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION */
union my_union GTY(()) {
    my_scalar_t scalar_val;
    struct my_struct* struct_ptr;
    char* string_ptr;
};

/* TYPE_ARRAY: Struct with array fields */
struct array_container GTY(()) {
    int fixed_array[10] GTY(());
    int variable_array[] GTY(());
    struct my_struct* pointer_array[5] GTY(());
};

/* TYPE_STRING: String type */
struct string_container GTY(()) {
    char* regular_string GTY((length("strlen($) + 1")));
    const char* const_string GTY((length("strlen($) + 1")));
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler;
    void (*another_handler)(void) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct(1))) {
    int lang_data;
    void* lang_ptr;
};

/* Nested types for complex interdependencies */
struct outer_container GTY(()) {
    struct {
        int nested_data;
        union {
            int nested_union_int;
            double nested_union_double;
        } nested_union GTY(());
    } nested_struct GTY(());
    
    struct my_struct* recursive_ptr;
    struct outer_container* self_ptr;  /* Circular reference */
};

/* Now define the forward-declared struct (completing TYPE_UNDEFINED) */
struct forward_declared_struct GTY(()) {
    int defined_field;
    struct my_struct* back_ptr;
};

/* TYPE_POINTER: Additional pointer types */
typedef struct my_struct* my_struct_ptr GTY(());
typedef union my_union* my_union_ptr GTY(());

/* Array of pointers */
typedef struct my_struct* struct_ptr_array[3] GTY(());

#endif /* TEST_GENGYPE_H */
