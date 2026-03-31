/* test_types.h - Type definitions to cover all gengtype type categories */

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
    int field1;
    double field2;
    my_scalar scalar_field;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_defined GTY((user)) {
    void* custom_data;
    int tag;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    double double_val;
    char* string_val GTY((length("10")));
    struct plain_struct* struct_ptr;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_struct GTY(()) {
    struct plain_struct* next GTY((skip));
    struct string_struct* str_ptr;
    union my_union* union_ptr;
    void* opaque_ptr GTY((skip));
};

/* TYPE_ARRAY: Various array types */
struct array_struct GTY(()) {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length annotation */
    int* vla GTY((length("vla_len")));
    size_t vla_len;
    
    /* Array of pointers */
    struct plain_struct* ptr_array[5];
    
    /* Nested array */
    double matrix[3][4];
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_struct GTY(()) {
    callback_func handler;
    void* user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int lang_id;
    void* lang_data;
    
    /* Chain of language structs */
    struct lang_specific* next GTY((skip));
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
struct undefined_struct;

/* Complex nested type to ensure deep traversal */
struct container GTY(()) {
    /* Scalar */
    my_scalar scalar;
    
    /* String */
    struct string_struct str;
    
    /* Plain struct */
    struct plain_struct plain;
    
    /* User struct */
    struct user_defined* user_ptr;
    
    /* Union */
    union my_union u;
    
    /* Pointer struct */
    struct pointer_struct* ptrs;
    
    /* Array struct */
    struct array_struct arrays;
    
    /* Callback */
    struct callback_struct cb;
    
    /* Language-specific */
    struct lang_specific* lang;
    
    /* Undefined (forward declared) */
    struct undefined_struct* undefined_ptr;
    
    /* Self-reference */
    struct container* next GTY((skip));
    
    /* Array of containers */
    struct container* children[5];
};

/* Now define the previously undefined struct */
struct undefined_struct GTY(()) {
    int defined_now;
    struct container* back_ref;
};

/* Function pointer typedef (another callback type) */
typedef int (*comparator_func)(const void*, const void*) GTY((callback));

/* Union with tagged fields for GC */
union tagged_union GTY(()) {
    struct {
        int tag;
        union {
            int int_val;
            double double_val;
            struct container* container_ptr;
        } data;
    } tagged;
    
    long long raw;
};

/* Struct with bitfields */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Opaque pointer type */
typedef void* opaque_handle GTY((skip));

/* Enum type (treated as scalar) */
typedef enum {
    STATE_A,
    STATE_B,
    STATE_C
} my_enum GTY(());

/* Struct with enum */
struct enum_struct GTY(()) {
    my_enum state;
    int value;
};

#endif /* TEST_TYPES_H */
