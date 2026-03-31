#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());
typedef unsigned long another_scalar GTY(());

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Plain C struct */
struct my_struct GTY(())
{
    my_scalar field1;
    int field2;
    struct my_struct* next GTY((skip));
};

/* TYPE_USER_STRUCT: Struct with user-defined marker */
struct user_struct GTY((user))
{
    int data;
    void* user_data;
};

/* TYPE_UNION: Union type */
union my_union GTY(())
{
    int int_val;
    double double_val;
    char* str_val GTY((length("str_len")));
    struct my_struct* struct_ptr;
};

/* TYPE_POINTER: Pointer types */
typedef struct my_struct* struct_ptr GTY(());

/* TYPE_ARRAY: Array types */
struct array_container GTY(())
{
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length callback */
    int* var_array GTY((length("var_len")));
    size_t var_len;
    
    /* Pointer to array */
    int (*array_ptr)[5] GTY((skip));
};

/* TYPE_STRING: String type with length callback */
struct string_container GTY(())
{
    char* dynamic_string GTY((length("str_length")));
    size_t str_length;
    
    const char* const_string GTY((skip));
};

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(())
{
    callback_func handler;
    void* user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef __cplusplus
    #define LANG_STRUCT_ATTR GTY((lang_struct))
#else
    #define LANG_STRUCT_ATTR GTY(())
#endif

struct lang_specific_struct LANG_STRUCT_ATTR
{
    int lang_field1;
    void* lang_field2;
};

/* Complex nested type to ensure deep traversal */
struct complex_nested GTY(())
{
    /* Contains all major type kinds */
    my_scalar scalar_field;
    struct my_struct struct_field;
    union my_union union_field;
    struct_ptr pointer_field;
    struct array_container array_field;
    struct string_container string_field;
    struct callback_container callback_field;
    
    /* Self-referential pointer */
    struct complex_nested* next GTY((skip));
    
    /* Array of pointers */
    struct my_struct* ptr_array[5] GTY((skip));
    
    /* Pointer to array */
    int (*matrix_ptr)[3][3] GTY((skip));
};

/* Enum type (also scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY(());

/* Function pointer with callback attribute */
typedef int (*comparator_func)(const void*, const void*) GTY((callback));

/* Template for generating more complex types */
#define DECLARE_PAIR_TYPE(T1, T2) \
    struct pair_##T1##_##T2 GTY(()) { \
        T1 first; \
        T2 second; \
    };

/* Generate some template instantiations */
DECLARE_PAIR_TYPE(int, char*)
DECLARE_PAIR_TYPE(struct my_struct*, union my_union)

/* Anonymous struct/union */
struct anonymous_container GTY(())
{
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } value;
};

/* Bitfield struct */
struct bitfield_struct GTY(())
{
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

#endif /* TEST_TYPES_H */
