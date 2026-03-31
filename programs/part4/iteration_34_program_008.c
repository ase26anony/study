#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
typedef char* my_string GTY((length("strlen($1) + 1")));

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (*my_callback)(void) GTY((callback));

/* TYPE_STRUCT: Plain struct type */
struct my_struct GTY(()) {
    int field1;
    char field2;
    my_scalar field3;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct my_user_struct GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    float float_val;
    char* str_val;
    struct my_struct* struct_ptr;
};

/* TYPE_ARRAY: Struct containing arrays */
struct my_array_struct GTY(()) {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length annotation */
    int* var_array GTY((length("var_len")));
    size_t var_len;
    
    /* Nested array of pointers */
    struct my_struct* struct_array[5];
};

/* TYPE_POINTER: Struct with various pointer types */
struct my_pointer_struct GTY(()) {
    /* Pointer to scalar */
    int* scalar_ptr;
    
    /* Pointer to struct */
    struct my_struct* struct_ptr;
    
    /* Pointer to union */
    union my_union* union_ptr;
    
    /* Pointer to callback */
    my_callback callback_ptr;
    
    /* Double pointer */
    struct my_struct** double_ptr;
    
    /* Pointer with attribute */
    int* __attribute__((aligned(16))) aligned_ptr;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct my_lang_struct GTY((lang_struct)) {
    int lang_specific;
    void* lang_data;
};

/* Complex nested type to ensure deep traversal */
struct complex_nested GTY(()) {
    /* String field */
    char* name GTY((length("name_len")));
    size_t name_len;
    
    /* Array of structs */
    struct my_struct items[8];
    
    /* Pointer to union */
    union my_union* variant;
    
    /* Callback */
    my_callback handler;
    
    /* Self-referential pointer */
    struct complex_nested* next;
    
    /* Array of pointers */
    struct my_pointer_struct* ptr_array[4];
};

/* Forward declarations for circular references */
struct forward_decl GTY(());

struct forward_decl GTY(()) {
    int data;
    struct forward_decl* next;
    struct my_struct* sibling;
};

/* Enum type (treated as scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY(());

/* Typedef chain */
typedef struct my_struct my_struct_alias GTY(());
typedef my_struct_alias* my_struct_ptr GTY(());

/* Struct with bitfields */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

#endif /* TEST_TYPES_H */
