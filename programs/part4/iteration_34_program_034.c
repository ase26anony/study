#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(()) {
    char* data GTY((length("str_len")));
    int str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_defined GTY((user)) {
    void* custom_data;
    int tag;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int as_int;
    double as_double;
    char* as_string GTY((length("10")));
    struct plain_struct* as_struct;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_container GTY(()) {
    struct plain_struct* next GTY((skip));
    struct string_struct** string_array;
    union my_union* union_ptr;
    void* opaque GTY((tag("0")));
};

/* TYPE_ARRAY: Array types (fixed and variable length) */
struct array_container GTY(()) {
    int fixed_array[10];
    char* variable_array GTY((length("array_len")));
    int array_len;
    struct plain_struct* struct_array GTY((length("struct_count")));
    int struct_count;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void* user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int language_id;
    void* language_data;
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type */
struct undefined_type;

/* Complex nested type to ensure thorough traversal */
struct master_container GTY(()) {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct str_field;
    
    /* Plain struct */
    struct plain_struct plain_field;
    
    /* User struct */
    struct user_defined* user_field;
    
    /* Union */
    union my_union union_field;
    
    /* Pointer */
    struct pointer_container* ptr_field;
    
    /* Array */
    struct array_container array_field;
    
    /* Callback */
    struct callback_container callback_field;
    
    /* Language struct */
    struct lang_specific* lang_field;
    
    /* Undefined type pointer */
    struct undefined_type* undefined_ptr;
    
    /* Self-reference for type graph */
    struct master_container* next;
};

/* Function pointer typedef (another callback type) */
typedef int (*compare_func)(const void*, const void*) GTY((callback));

/* Array of pointers */
typedef struct plain_struct* struct_ptr_array[5] GTY(());

/* Union with tag for discrimination */
struct tagged_union GTY(()) {
    int tag;
    union {
        int as_int;
        char* as_string GTY((length("20")));
        struct array_container* as_array;
    } data GTY((desc("tag")));
};

/* Nested array in struct */
struct nested_arrays GTY(()) {
    int matrix[3][4];
    char* strings[2] GTY((length("string_lens[%0]")));
    int string_lens[2];
};

#endif /* TEST_TYPES_H */
