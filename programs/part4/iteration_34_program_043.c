#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(()) {
    char* data GTY((length("strlen($1.data) + 1")));
    int length;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_struct GTY((user)) {
    void* custom_data;
    int tag;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int as_int;
    double as_double;
    char* as_string GTY((length("strlen($1.as_string) + 1")));
    struct plain_struct* as_struct;
};

/* TYPE_POINTER: Pointer types */
struct pointer_struct GTY(()) {
    struct plain_struct* next GTY((skip));
    struct string_struct* str_ptr;
    union my_union* union_ptr;
    void* opaque_ptr GTY((skip));
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int* variable_array GTY((length("$1.dynamic_length")));
    size_t dynamic_length;
    
    /* Nested array in struct */
    struct {
        char* nested_array GTY((length("$1.nested_length")));
        size_t nested_length;
    } nested GTY(());
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (*callback_type)(int, const char*) GTY((callback));

struct callback_struct GTY(()) {
    callback_type handler;
    void (*raw_func_ptr)(void);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_struct GTY((lang_struct)) {
    int language_specific_field;
    void* language_data GTY((skip));
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type */
struct undefined_struct;
struct forward_ref_struct GTY(()) {
    struct undefined_struct* undefined_ptr GTY((skip));
    struct forward_ref_struct* self_ref;
};

/* Complex nested type to ensure full traversal */
struct master_container GTY(()) {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct str_field;
    
    /* Plain struct */
    struct plain_struct plain_field;
    
    /* User struct */
    struct user_struct user_field;
    
    /* Union */
    union my_union union_field;
    
    /* Pointer */
    struct pointer_struct* ptr_field;
    
    /* Array */
    struct array_struct array_field;
    
    /* Callback */
    struct callback_struct callback_field;
    
    /* Language struct */
    struct lang_struct* lang_field;
    
    /* Undefined reference */
    struct forward_ref_struct forward_field;
    
    /* Self-reference for cycles */
    struct master_container* next;
};

/* Template-like macro to generate multiple instances */
#define DECLARE_TYPE(name, base) \
    typedef base name##_t GTY(())

DECLARE_TYPE(derived_scalar, int);
DECLARE_TYPE(derived_ptr, struct plain_struct*);

/* Variable-length struct with trailing array */
struct var_len_struct GTY(()) {
    int count;
    int data[1] GTY((length("$1.count")));
};

/* Anonymous union/struct */
struct anonymous_container GTY(()) {
    union {
        int a;
        double b;
    } anonymous_union;
    
    struct {
        char* name;
        int id;
    } anonymous_struct;
};

#endif /* TEST_TYPES_H */
