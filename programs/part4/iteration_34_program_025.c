#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int GTY(()) my_scalar;

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_struct {
    char* GTY((length("strlen($1) + 1"))) data;
    int length;
};

/* TYPE_STRUCT: Regular struct */
struct GTY(()) regular_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct GTY((user)) user_struct {
    void* data;
    size_t size;
    /* User-defined marker function would be defined elsewhere */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int i;
    double d;
    char* GTY((tag("0"))) str;
    void* GTY((tag("1"))) ptr;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_struct {
    struct regular_struct* GTY((skip)) regular_ptr;
    struct string_struct** GTY((skip)) double_ptr;
    void (*GTY((skip)) func_ptr)(void);
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_struct {
    int GTY((length("10"))) fixed_array[10];
    char* GTY((length("$1->dynamic_len"))) dynamic_array;
    int dynamic_len;
    
    /* Nested array in struct */
    struct regular_struct GTY((length("5"))) struct_array[5];
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (*GTY((callback)) callback_func)(int, void*);

struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct)) lang_specific_struct {
    /* Language-specific fields would be defined by GCC frontends */
    void* lang_data;
    int lang_tag;
};

/* Complex nested type to ensure thorough traversal */
struct GTY(()) complex_type {
    my_scalar scalar_field;
    struct string_struct string_field;
    union my_union union_field;
    struct pointer_struct* pointer_field;
    struct array_struct array_field;
    struct callback_container callback_field;
    struct lang_specific_struct* lang_field;
    
    /* Self-referential pointer */
    struct complex_type* GTY((skip)) next;
};

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) undefined_struct;

/* Another struct that references the undefined one */
struct GTY(()) references_undefined {
    struct undefined_struct* GTY((skip)) undefined_ptr;
    int valid_data;
};

/* Enum type (also scalar) */
typedef enum GTY(()) {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum;

/* Variable-length array in struct */
struct GTY(()) var_len_struct {
    int count;
    int GTY((length("$1->count"))) items[];
};

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

#endif /* TEST_TYPES_H */
