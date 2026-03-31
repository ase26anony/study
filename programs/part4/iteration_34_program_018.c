/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned int my_unsigned_scalar GTY((user));

/* TYPE_STRING: String type with length annotation */
struct string_struct {
    char *data GTY((length("strlen($1.data) + 1")));
    int length;
} GTY((tag("STRING_STRUCT")));

/* TYPE_STRUCT: Plain C structs */
struct plain_struct {
    int field1;
    float field2;
    double field3;
} GTY(());

/* Nested struct for complex type graph */
struct nested_struct {
    struct plain_struct inner;
    struct string_struct *str_ptr;
} GTY(());

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_defined_struct {
    int user_id;
    void *user_data;
} GTY((user));

/* TYPE_UNION: Union containing GTY-tagged fields */
union my_union {
    int int_val;
    float float_val;
    struct plain_struct *struct_ptr;
    char *string_val;
} GTY((tag("MY_UNION")));

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_struct {
    struct plain_struct *direct_ptr;
    struct nested_struct **double_ptr;
    union my_union *union_ptr;
    void *void_ptr;
    int (*func_ptr)(void);
} GTY(());

/* TYPE_ARRAY: Structs with array fields */
struct array_struct {
    int fixed_array[10];
    struct plain_struct struct_array[5];
    int *dynamic_array GTY((length("$1.dynamic_length")));
    size_t dynamic_length;
    
    /* Variable length array */
    char vla[] GTY((length("$1.vla_length")));
    size_t vla_length;
} GTY(());

/* TYPE_CALLBACK: Callback function types */
typedef int (*comparison_callback)(const void *, const void *) GTY((callback));

struct callback_container {
    comparison_callback cmp_func;
    void (*cleanup_func)(void *) GTY((callback));
    int (*transform_func)(int) GTY((callback("default_callback")));
} GTY(());

/* Default callback implementation */
int default_callback(int x) {
    return x * 2;
}

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct {
    int lang_id;
    void *lang_data;
} GTY((lang_struct));

/* Complex type with multiple nested structures */
struct master_container {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct str_field;
    
    /* Plain struct */
    struct plain_struct plain_field;
    
    /* User struct */
    struct user_defined_struct *user_field;
    
    /* Union */
    union my_union union_field;
    
    /* Pointer */
    struct pointer_struct *ptr_field;
    
    /* Array */
    struct array_struct array_field;
    
    /* Callback */
    struct callback_container callback_field;
    
    /* Language struct */
    struct lang_specific_struct *lang_field;
    
    /* Self-referential pointer */
    struct master_container *next;
} GTY(());

/* Forward declarations for circular references */
struct forward_declared_struct;
struct another_forward_declared;

struct forward_container {
    struct forward_declared_struct *fwd_ptr;
    struct another_forward_declared **fwd_double_ptr;
} GTY(());

struct forward_declared_struct {
    int data;
    struct forward_container *back_ref;
} GTY(());

struct another_forward_declared {
    char name[50];
    struct forward_declared_struct *related;
} GTY(());

/* Enum type (treated as scalar in gengtype) */
typedef enum {
    ENUM_VALUE1,
    ENUM_VALUE2,
    ENUM_VALUE3
} my_enum_type GTY(());

/* Bitfield struct */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
} GTY(());

/* Function pointer typedef */
typedef void (*generic_handler)(void *context, int event) 
    GTY((callback("default_handler")));

void default_handler(void *context, int event) {
    /* Default implementation */
}

/* Container with function pointers */
struct handler_container {
    generic_handler handlers[5];
    void *contexts[5];
} GTY(());

/* TYPE_UNDEFINED: Incomplete/opaque type */
struct opaque_struct;
typedef struct opaque_struct *opaque_handle;

/* Anonymous struct/union */
struct anonymous_container {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } value;
} GTY(());

/* Template-like macro for generic containers */
#define DECLARE_CONTAINER(TYPE, NAME) \
    struct NAME { \
        TYPE *items; \
        size_t count; \
        size_t capacity; \
    } GTY(())

/* Instantiate template-like containers */
DECLARE_CONTAINER(int, int_container);
DECLARE_CONTAINER(struct plain_struct, struct_container);
DECLARE_CONTAINER(union my_union, union_container);

/* Aligned types using GCC attributes */
struct aligned_struct {
    int normal_field;
    int aligned_field __attribute__((aligned(64)));
    char packed_field __attribute__((packed));
} GTY(());

/* Volatile and const qualified pointers */
struct qualified_pointer_struct {
    volatile int *volatile_ptr;
    const struct plain_struct *const_ptr;
    const volatile int *cv_ptr;
} GTY(());

#endif /* TEST_TYPES_H */
