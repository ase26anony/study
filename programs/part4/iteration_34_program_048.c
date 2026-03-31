#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct string_struct GTY(()) {
    char *data GTY((length("str_len")));
    size_t str_len;
};

/* TYPE_STRUCT: Plain struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct user_struct GTY((user)) {
    void *user_data;
    int user_tag;
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
    } vla;
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (*callback_func)(int, const char *) GTY((callback));

struct callback_struct GTY(()) {
    callback_func handler;
    void *context;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_struct GTY((lang_struct)) {
    int lang_specific_field;
    void *lang_data;
};

/* Complex nested type to ensure thorough traversal */
struct master_struct GTY(()) {
    my_scalar scalar_field;
    struct string_struct string_field;
    struct plain_struct struct_field;
    union my_union union_field;
    struct pointer_struct pointer_field;
    struct array_struct array_field;
    struct callback_struct callback_field;
    struct lang_struct *lang_ptr;
    
    /* Self-referential pointer */
    struct master_struct *next GTY((skip));
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
struct undefined_struct;
struct uses_undefined GTY(()) {
    struct undefined_struct *undef_ptr;
};

/* Now define the undefined struct */
struct undefined_struct GTY(()) {
    int defined_now;
};

/* Additional pointer types with different attributes */
typedef struct plain_struct *plain_ptr GTY(());
typedef const struct string_struct *const_string_ptr GTY(());

/* Array of pointers */
struct pointer_array GTY(()) {
    plain_ptr pointers[5];
    callback_func callbacks[3];
};

#endif /* TEST_TYPES_H */
