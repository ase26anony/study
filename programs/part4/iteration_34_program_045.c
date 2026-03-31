#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct string_struct GTY(()) {
    char *data GTY((length("strlen($1->data) + 1")));
    int length;
};

/* TYPE_STRUCT: Plain struct type */
struct plain_struct GTY(()) {
    int field1;
    float field2;
    struct plain_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct user_struct GTY((user)) {
    void *data;
    size_t size;
    void (*cleanup)(void*);
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    float float_val;
    char *string_val GTY((length("strlen($1.string_val) + 1")));
    void *ptr_val;
};

/* TYPE_POINTER: Pointer type in struct */
struct pointer_struct GTY(()) {
    struct plain_struct *plain_ptr;
    struct string_struct *string_ptr;
    union my_union *union_ptr;
    int *scalar_ptr;
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int *variable_array GTY((length("$1->var_len")));
    char *string_array[5] GTY((length("strlen($1->string_array[$2]) + 1")));
    size_t var_len;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_struct GTY(()) {
    callback_func handler;
    void *user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_struct GTY((lang_struct)) {
    int lang_specific_field;
    void *lang_data;
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type */
struct undefined_struct;

/* Complex nested type to ensure thorough traversal */
struct container GTY(()) {
    my_scalar scalar_field;
    struct string_struct string_field;
    struct plain_struct struct_field;
    union my_union union_field;
    struct pointer_struct pointer_field;
    struct array_struct array_field;
    struct callback_struct callback_field;
    struct lang_struct *lang_ptr;
    struct undefined_struct *undefined_ptr;  /* TYPE_UNDEFINED */
};

/* Function pointer array */
typedef void (*func_array[5])(void) GTY((skip));

/* Variable length struct */
struct var_struct GTY(()) {
    int count;
    int items[] GTY((length("$1->count")));
};

#endif /* TEST_TYPES_H */
