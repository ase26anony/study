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
struct user_struct GTY((user)) {
    void *custom_data;
    int (*user_method)(void);
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int as_int;
    double as_double;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer types */
struct pointer_struct GTY(()) {
    struct plain_struct *next GTY(());
    struct string_struct *str_ptr GTY(());
    void *generic_ptr GTY((skip));
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int *variable_array GTY((length("array_len")));
    size_t array_len;
    
    /* Nested array in struct */
    struct {
        char nested_array[5][20];
    } nested GTY(());
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef int (*callback_type)(int, void*) GTY((callback));

struct callback_struct GTY(()) {
    callback_type handler;
    void *user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_struct GTY((lang_struct)) {
    int language_specific_field;
    void *language_hook GTY((skip));
};

/* Complex nested type to ensure thorough traversal */
struct container GTY(()) {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct str_field;
    
    /* Plain struct */
    struct plain_struct plain_field;
    
    /* Pointer */
    struct pointer_struct *ptr_field GTY(());
    
    /* Array */
    struct array_struct array_field;
    
    /* Union */
    union my_union union_field;
    
    /* Callback */
    struct callback_struct callback_field;
    
    /* Language struct */
    struct lang_struct *lang_field GTY(());
    
    /* Self-referential pointer */
    struct container *next GTY(());
    
    /* Array of pointers */
    struct plain_struct *ptr_array[5] GTY(());
    
    /* Flexible array member */
    int flexible_array[] GTY((length("flex_len")));
    size_t flex_len;
};

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* Function prototypes that use our types */
void process_container(struct container *cont GTY(()));
callback_type get_callback(void);

#endif /* TEST_TYPES_H */
