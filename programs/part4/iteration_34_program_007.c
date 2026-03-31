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

/* TYPE_STRUCT: Regular struct */
struct regular_struct GTY(()) {
    my_scalar value;
    struct regular_struct *next;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct user_defined GTY((user)) {
    void *custom_data;
    int (*user_callback)(void);
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    double double_val;
    void *ptr_val;
};

/* TYPE_POINTER: Pointer type */
typedef struct regular_struct *struct_ptr GTY(());

/* TYPE_ARRAY: Array types */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length callback */
    int *vla GTY((length("vla_len")));
    size_t vla_len;
    
    /* Nested array of pointers */
    struct regular_struct *ptr_array[5];
};

/* TYPE_CALLBACK: Callback function type */
typedef int (*callback_func)(int, char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int lang_field;
    void *lang_data;
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type reference */
struct undefined_struct;
struct uses_undefined GTY(()) {
    struct undefined_struct *undef_ptr;  /* TYPE_UNDEFINED when walking */
};

/* Complex nested type to ensure all cases are hit */
struct master_container GTY(()) {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct str_field;
    
    /* Regular struct */
    struct regular_struct reg_field;
    
    /* User struct */
    struct user_defined user_field;
    
    /* Union */
    union my_union union_field;
    
    /* Pointer */
    struct_ptr ptr_field;
    
    /* Array container */
    struct array_container array_field;
    
    /* Callback */
    callback_func callback_field;
    
    /* Language-specific */
    struct lang_specific lang_field;
    
    /* Undefined reference */
    struct uses_undefined undef_field;
    
    /* Self-reference pointer */
    struct master_container *self_ptr;
    
    /* Array of unions */
    union my_union union_array[3];
    
    /* Pointer to callback */
    callback_func *callback_ptr;
};

/* Function pointer table (callback array) */
struct callback_table GTY(()) {
    callback_func callbacks[4];
    int count;
};

/* Nested struct with all types */
struct nested_types GTY(()) {
    struct {
        struct regular_struct inner_struct;
        union my_union inner_union;
    } anonymous;
    
    struct array_container *container_ptr;
    
    /* Multi-dimensional array */
    int matrix[3][4];
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Function pointer returning pointer */
    struct master_container *(*factory)(int);
};

/* Enumeration type (also scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY(());

/* Bitfield struct */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    my_scalar value;
};

/* External declaration to force TYPE_UNDEFINED */
extern struct undefined_struct external_undefined;

#endif /* TEST_TYPES_H */
