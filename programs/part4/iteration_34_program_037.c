/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar GTY(());
typedef unsigned char my_uchar_scalar;

/* TYPE_UNDEFINED: Forward declarations that create undefined types */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr;

/* TYPE_STRING: String type with length annotation */
typedef char *my_string GTY((length("strlen($1) + 1")));

/* TYPE_STRUCT: Plain C structs */
struct my_struct GTY(()) {
    int field1;
    char field2;
    struct my_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct user_defined GTY((user)) {
    void *data;
    size_t size;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    double double_val;
    char *string_val GTY((length("strlen($1) + 1")));
    void *ptr_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_struct *struct_ptr GTY(());
typedef union my_union *union_ptr;
typedef int (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
struct array_container GTY(()) {
    int fixed_array[10];
    int *variable_array GTY((length("$1->array_length")));
    size_t array_length;
    
    /* Nested array in struct */
    struct {
        char nested_array[5][10];
    } nested;
};

/* TYPE_CALLBACK: Callback function types */
typedef void (*callback_func)(int, void *) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void *user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef __cplusplus
extern "C" {
#endif

struct lang_specific GTY((lang_struct)) {
    /* Language-specific fields would go here */
    void *lang_data;
    int lang_tag;
};

#ifdef __cplusplus
}
#endif

/* Complex type graph to ensure thorough traversal */
struct complex_type_graph GTY(()) {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    my_string str_field GTY((length("strlen($1->str_field) + 1")));
    
    /* Pointer to struct */
    struct my_struct *struct_ptr_field;
    
    /* Pointer to union */
    union my_union *union_ptr_field;
    
    /* Array */
    int int_array[20];
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Nested struct */
    struct {
        int nested_scalar;
        char *nested_string GTY((length("strlen($1->complex_type_graph::nested_string) + 1")));
    } nested_struct;
    
    /* Union field */
    union my_union union_field;
    
    /* Callback */
    callback_func cb_field;
    
    /* Pointer to lang struct */
    struct lang_specific *lang_ptr;
    
    /* Self-referential pointer */
    struct complex_type_graph *next GTY((skip));
    
    /* Variable length array at end */
    int flexible_array[] GTY((length("$1->flexible_length")));
    size_t flexible_length;
};

/* Another struct to ensure user struct coverage */
struct another_user_struct GTY((user)) {
    struct user_defined *ud_ptr;
    int user_id;
};

/* Enumeration type (also scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY(());

/* Function pointer typedef */
typedef int (*comparison_func)(const void *, const void *) GTY((callback));

/* Container with all types */
struct type_container GTY(()) {
    /* TYPE_SCALAR */
    my_scalar scalar_member;
    my_enum enum_member;
    
    /* TYPE_STRING */
    char *string_member GTY((length("strlen($1->string_member) + 1")));
    
    /* TYPE_STRUCT */
    struct my_struct struct_member;
    
    /* TYPE_USER_STRUCT */
    struct user_defined user_struct_member;
    
    /* TYPE_UNION */
    union my_union union_member;
    
    /* TYPE_POINTER */
    void *void_ptr;
    struct my_struct *struct_ptr_member;
    
    /* TYPE_ARRAY */
    int array_member[15];
    struct array_container array_struct_member;
    
    /* TYPE_CALLBACK */
    callback_func callback_member;
    comparison_func compare_member;
    
    /* TYPE_LANG_STRUCT */
    struct lang_specific *lang_struct_ptr;
    
    /* Chain of complex types */
    struct complex_type_graph *graph_head;
};

/* Global variable declarations to ensure types are instantiated */
extern struct type_container global_container;
extern struct complex_type_graph *global_graph;

#endif /* TEST_TYPES_H */
