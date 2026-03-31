#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length callback */
struct string_struct GTY(()) {
    char *data GTY((length("str_len")));
    size_t str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct user_defined GTY((user)) {
    void *custom_data;
    int (*user_callback)(void);
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int as_int;
    double as_double;
    char *as_string GTY((length("10")));
};

/* TYPE_POINTER: Pointer types */
struct pointer_struct GTY(()) {
    struct plain_struct *next GTY((skip));
    struct string_struct *str_ptr;
    void *generic_ptr;
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int *dynamic_array GTY((length("dyn_len")));
    size_t dyn_len;
    
    /* Variable length array in struct */
    struct {
        int count;
        int items GTY((variable_length));
    } var_struct;
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef int (*callback_func)(int, char *) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void *context;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int lang_specific_field;
    void *lang_data;
};

/* Complex nested type to ensure traversal */
struct master_container GTY(()) {
    my_scalar scalar_field;
    struct string_struct string_field;
    struct plain_struct struct_field;
    struct user_defined *user_field GTY((skip));
    union my_union union_field;
    struct pointer_struct pointer_field;
    struct array_struct array_field;
    struct callback_container callback_field;
    struct lang_specific *lang_field;
    
    /* Self-referential pointer */
    struct master_container *next GTY((skip));
};

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* Function using undefined type */
void use_undefined(struct undefined_type *param GTY((skip)));

#endif /* TEST_TYPES_H */
