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
    char *data GTY((length("str_len")));
    int str_len;
};

/* TYPE_STRUCT: Plain C structs */
struct plain_struct {
    int field1;
    double field2;
};

struct GTY(()) tagged_struct {
    int id;
    struct tagged_struct *next;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
};

/* TYPE_UNION: Union containing GTY-tagged fields */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char * GTY((tag("0"))) string_val;
    struct tagged_struct * GTY((tag("1"))) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_container {
    int *scalar_ptr;
    struct tagged_struct **double_ptr;
    void *generic_ptr;
    const char *const_string_ptr;
};

/* TYPE_ARRAY: Array types - fixed and variable length */
struct array_struct {
    int fixed_array[10];
    int *variable_array GTY((length("array_len")));
    size_t array_len;
    
    /* Multi-dimensional array */
    int matrix[5][5];
};

/* TYPE_CALLBACK: Function pointer/callback types */
typedef void (*simple_callback)(int, char*);

struct GTY(()) callback_container {
    simple_callback cb;
    void (*complex_callback)(struct tagged_struct*, union data_union*) 
        GTY((callback));
    
    /* Another callback approach */
    int (*comparator)(const void*, const void*);
};

/* TYPE_LANG_STRUCT: Language-specific struct with hooks */
struct GTY((lang_struct("c"))) language_specific {
    int lang_specific_field;
    
    /* Nested language-specific type */
    struct GTY((lang_struct("c++"))) nested_lang_struct {
        void *cpp_specific;
    } *nested;
};

/* Complex type graph to ensure thorough traversal */
struct GTY(()) complex_node {
    int id;
    
    /* Self-referential pointer */
    struct complex_node *self_ptr;
    
    /* Pointer to another node */
    struct complex_node *next;
    
    /* Array of pointers */
    struct complex_node **children GTY((length("child_count")));
    int child_count;
    
    /* Union field */
    union data_union data;
    
    /* Callback */
    simple_callback notify;
    
    /* String */
    char *name GTY((length("name_len")));
    int name_len;
};

/* TYPE_UNDEFINED: Forward declaration that might be undefined */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr;

/* Template-like macro to generate more types */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct GTY(()) name { \
        field_type value; \
        struct name *next; \
    }

/* Generate more struct types using macro */
DECLARE_GTY_STRUCT(int_list, int);
DECLARE_GTY_STRUCT(string_list, char*);

/* Anonymous struct/union */
struct GTY(()) container {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int as_int;
        float as_float;
    } value;
};

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Volatile and atomic qualified types */
struct GTY(()) volatile_struct {
    volatile int volatile_field;
    _Atomic int atomic_field;
    const int const_field;
};

/* Alignment-specified struct */
struct GTY(()) aligned_struct {
    int normal_field;
    long long GTY((aligned(16))) aligned_field;
} __attribute__((aligned(32)));

/* Packed struct */
struct GTY(()) packed_struct {
    char a;
    int b;
    char c;
} __attribute__((packed));

#endif /* TEST_TYPES_H */
