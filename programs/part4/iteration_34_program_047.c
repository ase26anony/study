/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* ========== TYPE_SCALAR ========== */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;
typedef double my_double_scalar;

/* ========== TYPE_STRING ========== */
struct GTY(()) string_struct {
    char * GTY((length("str_len"))) data;
    int str_len;
};

/* String callback type */
typedef char * GTY((callback)) (*string_callback_t)(int len);

/* ========== TYPE_STRUCT ========== */
struct GTY(()) plain_struct {
    int field1;
    double field2;
    char field3;
};

/* Nested struct */
struct GTY(()) outer_struct {
    struct plain_struct inner;
    int outer_field;
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
};

/* Another user struct with tag */
struct GTY((user, tag("USER_TAG"))) tagged_user_struct {
    long tag_data;
    float tag_float;
};

/* ========== TYPE_UNION ========== */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char * GTY((length("str_len"))) string_val;
    struct plain_struct struct_val;
    int str_len;
};

/* Tagged union */
union GTY((desc("(%1.type == 0 ? &int_type_node : %1.type == 1 ? &double_type_node : &string_type_node)"))) typed_union {
    int type;
    int int_data;
    double double_data;
    char *string_data;
};

/* ========== TYPE_POINTER ========== */
struct GTY(()) pointer_struct {
    struct plain_struct * GTY((skip)) ptr_to_struct;
    union data_union * GTY((skip)) ptr_to_union;
    int *ptr_to_scalar;
    struct pointer_struct *next;  /* Self-referential pointer */
};

/* Chain of pointers */
struct GTY(()) pointer_chain {
    struct pointer_chain * GTY((skip)) next;
    void * GTY((skip)) data;
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_struct {
    int fixed_array[10];
    int * GTY((length("dynamic_len"))) dynamic_array;
    int dynamic_len;
    
    /* Array of pointers */
    struct plain_struct * GTY((length("ptr_count"))) ptr_array[5];
    int ptr_count;
    
    /* Multi-dimensional array */
    double matrix[3][3];
};

/* Variable length array in struct */
struct GTY(()) varray_struct {
    int count;
    char data[1];  /* Variable length */
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer type */
typedef int GTY((callback)) (*compare_func_t)(const void *, const void *);

/* Struct with callback field */
struct GTY(()) callback_container {
    compare_func_t comparator;
    string_callback_t string_generator;
    
    /* Nested callback */
    void (* GTY((callback)) nested_callback)(int, double);
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific struct (simulating GCC's tree_node) */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_struct_node {
    int code;
    union {
        int int_val;
        double real_val;
        struct string_struct *string_val;
    } GTY((desc ("TREE_CODE (&%0) == INTEGER_CST ? 0 : TREE_CODE (&%0) == REAL_CST ? 1 : 2"))) u;
    struct lang_struct_node *next;
    struct lang_struct_node *prev;
};

/* Another language struct with special handling */
struct GTY((variable_size)) variable_lang_struct {
    size_t size;
    unsigned char data[1];
};

/* ========== Complex Type Combinations ========== */

/* Struct containing all types */
struct GTY(()) mega_struct {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    char * GTY((length("mega_str_len"))) mega_string;
    int mega_str_len;
    
    /* Struct */
    struct plain_struct nested_struct;
    
    /* User struct */
    struct user_defined_struct user_field;
    
    /* Union */
    union data_union data_field;
    
    /* Pointer */
    struct pointer_struct *pointer_field;
    
    /* Array */
    int mega_array[20];
    
    /* Callback */
    compare_func_t callback_field;
    
    /* Language struct */
    struct lang_struct_node *lang_field;
};

/* Template-like macro for generating multiple struct types */
#define DEFINE_STRUCT_TYPE(name, field_type) \
    struct GTY(()) name##_struct { \
        field_type data; \
        struct name##_struct *next; \
    }

DEFINE_STRUCT_TYPE(int_list, int);
DEFINE_STRUCT_TYPE(double_list, double);
DEFINE_STRUCT_TYPE(string_list, char*);

/* ========== TYPE_UNDEFINED ========== */
/* Forward declarations that create undefined types when referenced */
struct GTY(()) forward_declared;
typedef struct forward_declared *forward_ptr_t;

/* Circular dependency creating undefined references */
struct GTY(()) type_a {
    struct type_b *link_to_b;
    int data_a;
};

struct GTY(()) type_b {
    struct type_a *link_to_a;
    double data_b;
};

/* Incomplete array type */
struct GTY(()) incomplete_container {
    int count;
    struct forward_declared *items[];  /* Flexible array member */
};

#endif /* TEST_TYPES_H */
