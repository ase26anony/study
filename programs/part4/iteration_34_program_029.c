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

/* TYPE_STRUCT: Plain struct */
struct plain_struct GTY(()) {
    my_scalar value;
    struct plain_struct *next;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct user_struct GTY((user)) {
    void *data;
    int tag;
};

/* TYPE_UNION: Union type */
union variant GTY(()) {
    int as_int;
    double as_double;
    char *as_string GTY((length("strlen(as_string)")));
};

/* TYPE_POINTER: Pointer type in struct */
struct pointer_container GTY(()) {
    struct plain_struct *ptr;
    struct pointer_container *self_ptr;
};

/* TYPE_ARRAY: Array types */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array */
    int *vla GTY((length("vla_len")));
    size_t vla_len;
    
    /* Array of pointers */
    struct plain_struct *ptr_array[5];
};

/* TYPE_CALLBACK: Callback function pointer */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void *user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int lang_id;
    void *lang_data;
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
struct undefined_type;
struct uses_undefined GTY(()) {
    struct undefined_type *undef_ptr;
};

/* Now define the undefined type to complete it */
struct undefined_type GTY(()) {
    int value;
};

/* Complex nested type to ensure traversal */
struct root_type GTY(()) {
    my_scalar scalar_field;
    struct string_struct string_field;
    struct plain_struct struct_field;
    union variant union_field;
    struct pointer_container pointer_field;
    struct array_container array_field;
    struct callback_container callback_field;
    struct lang_specific lang_field;
    struct uses_undefined undefined_field;
    
    /* Self-reference for type graph */
    struct root_type *next;
};

/* Function pointer type (another callback variant) */
typedef int (*compare_func)(const void*, const void*) GTY((callback));

/* Union with struct (complex type) */
union complex_union GTY(()) {
    struct {
        int x;
        double y;
    } s;
    struct array_container a;
    callback_func f;
};

#endif /* TEST_TYPES_H */
