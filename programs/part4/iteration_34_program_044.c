/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar GTY(());
typedef unsigned long my_unsigned_scalar GTY(());
typedef double my_float_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(())
{
    char * GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C structs */
struct plain_struct GTY(())
{
    int field1;
    double field2;
    void *ptr_field;
};

/* Nested struct to create type graph */
struct nested_struct GTY(())
{
    struct plain_struct inner;
    struct string_struct *str_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with marker */
struct user_defined GTY((user))
{
    int user_data;
    void *user_pointer;
};

/* TYPE_UNION: Union type */
union variant_type GTY(())
{
    int int_val;
    double double_val;
    char * GTY((length("strlen(&u.str_val)"))) str_val;
    struct plain_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_container GTY(())
{
    struct plain_struct * GTY((skip)) direct_ptr;
    struct nested_struct ** GTY((chain)) double_ptr;
    union variant_type *union_ptr;
    void * GTY((tag("0"))) tagged_ptr;
};

/* TYPE_ARRAY: Array types */
struct array_container GTY(())
{
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length annotation */
    char * GTY((length("vl_len"))) variable_length_array;
    size_t vl_len;
    
    /* Array of pointers */
    struct plain_struct * GTY((length("ptr_count"))) ptr_array[5];
    size_t ptr_count;
    
    /* Nested array */
    double matrix[3][4];
};

/* TYPE_CALLBACK: Callback function types */
typedef void (*simple_callback)(int) GTY((callback));

struct callback_container GTY(())
{
    simple_callback cb;
    void (* GTY((callback("my_callback_compare"))) compare_func)(void*, void*);
    int (*processor)(struct array_container *);
};

/* Forward declaration for mutual recursion */
struct recursive_struct;

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct))
{
    int lang_id;
    void *lang_data;
    struct recursive_struct *next;
};

/* Recursive structure for complex type graph */
struct recursive_struct GTY(())
{
    int id;
    struct recursive_struct * GTY((skip)) left;
    struct recursive_struct *right;
    struct lang_specific *lang_info;
};

/* Complex union with nested structures */
union complex_union GTY(())
{
    struct {
        int type_tag;
        union {
            int int_data;
            struct recursive_struct *recursive_data;
        } payload;
    } tagged;
    
    struct array_container array_data;
    struct callback_container callback_data;
};

/* Container that references many types */
struct master_container GTY(())
{
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct str_field;
    
    /* Plain struct */
    struct plain_struct plain_field;
    
    /* User struct */
    struct user_defined *user_field;
    
    /* Union */
    union variant_type union_field;
    
    /* Pointer */
    struct pointer_container *ptr_field;
    
    /* Array */
    struct array_container array_field;
    
    /* Callback */
    struct callback_container callback_field;
    
    /* Language struct */
    struct lang_specific lang_field;
    
    /* Recursive structure */
    struct recursive_struct *recursive_field;
    
    /* Complex union */
    union complex_union complex_field;
};

/* Template-like macro to generate parameterized types */
#define DECLARE_PARAM_TYPE(name, T) \
    struct name##_container GTY(()) { \
        T data; \
        struct name##_container *next; \
    }

DECLARE_PARAM_TYPE(int_param, int);
DECLARE_PARAM_TYPE(ptr_param, void*);
DECLARE_PARAM_TYPE(struct_param, struct plain_struct);

/* Opaque type for TYPE_UNDEFINED testing */
struct opaque_type;
typedef struct opaque_type * GTY(()) opaque_ptr;

/* Function pointer typedefs with different attributes */
typedef int (*__attribute__((stdcall)) stdcall_func)(int, int) GTY((callback));
typedef void (*__attribute__((fastcall)) fastcall_func)(void) GTY((callback));

/* GCC attribute examples */
struct __attribute__((packed, aligned(4))) packed_struct GTY(())
{
    char a;
    int b;
    short c;
};

/* Bitfield structure */
struct bitfield_struct GTY(())
{
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

#endif /* TEST_TYPES_H */
