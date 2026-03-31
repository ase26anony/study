#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
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

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_defined GTY((user)) {
    void *custom_data;
    int (*user_func)(void);
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int as_int;
    double as_double;
    char *as_string GTY((length("10")));
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_struct GTY(()) {
    struct plain_struct *next GTY((skip));
    struct string_struct *str_ptr;
    void *generic_ptr;
    union my_union *union_ptr;
};

/* TYPE_ARRAY: Struct with array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int *dynamic_array GTY((length("dyn_len")));
    size_t dyn_len;
    struct plain_struct struct_array[5];
};

/* TYPE_CALLBACK: Function pointer/callback type */
typedef int (*callback_func)(int, void *) GTY((callback));

struct callback_struct GTY(()) {
    callback_func handler;
    void *user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int lang_id;
    void *lang_data;
    struct lang_specific *next;
};

/* Complex nested type to ensure thorough traversal */
struct complex_nested GTY(()) {
    /* TYPE_UNDEFINED might be triggered by incomplete types */
    struct forward_decl *fwd_ptr;  /* Forward declaration */
    
    /* Mix of different type kinds */
    my_scalar scalar_field;
    struct string_struct string_field;
    struct plain_struct struct_field;
    union my_union union_field;
    struct pointer_struct *pointer_field;
    struct array_struct array_field;
    struct callback_struct callback_field;
    struct lang_specific lang_field;
    
    /* Nested arrays of different types */
    struct plain_struct *struct_ptr_array[3];
    callback_func callback_array[2];
};

/* Forward declaration for undefined type testing */
struct forward_decl;

/* Another complex type with variable length arrays */
struct vla_struct GTY(()) {
    int count;
    int data[] GTY((length("count")));
};

/* Union containing struct with callback */
union complex_union GTY(()) {
    struct callback_struct cb;
    struct array_struct arr;
    struct lang_specific lang;
};

/* Template-like structure using macros */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct name##_container GTY(()) { \
        field_type value; \
        struct name##_container *next; \
    }

/* Instantiate template-like structures */
DECLARE_GTY_STRUCT(int, int);
DECLARE_GTY_STRUCT(double, double);
DECLARE_GTY_STRUCT(string, struct string_struct);

/* Enumeration type */
typedef enum {
    MODE_A,
    MODE_B,
    MODE_C
} operation_mode GTY(());

/* Struct with enum and bitfields */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    operation_mode mode;
};

#endif /* TEST_TYPES_H */
