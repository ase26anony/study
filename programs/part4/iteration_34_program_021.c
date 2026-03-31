/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar typedef */
typedef int my_scalar GTY((user));

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) base_struct {
    int field1;
    char field2;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_marked_struct {
    void *user_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    char * GTY((length("strlen($)"))) string_val;
    struct base_struct *struct_ptr;
};

/* TYPE_POINTER: Pointer types in a struct */
struct GTY(()) pointer_container {
    struct base_struct * GTY((tag("0"))) ptr1;
    union data_union * GTY((tag("1"))) ptr2;
    void * GTY((skip)) opaque_ptr;
    int *scalar_ptr;  /* Not GTY-marked, but still a pointer type */
};

/* TYPE_ARRAY: Various array types */
struct GTY(()) array_container {
    int fixed_array[10];
    char * GTY((length("$->dynamic_len"))) variable_array[1];
    struct base_struct * GTY((length("$->struct_count"))) struct_array[1];
    int dynamic_len;
    int struct_count;
};

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_container {
    char * GTY((length("strlen($)"))) string_field;
    const char * GTY((length("strlen($)"))) const_string;
    unsigned char * GTY((length("$->data_len"))) byte_array;
    size_t data_len;
};

/* TYPE_CALLBACK: Callback function pointer */
typedef void (*callback_func)(int, void *) GTY((callback));

struct GTY(()) callback_container {
    callback_func handler;
    void * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct (simulated) */
#ifdef __cplusplus
#define GTY_LANG_STRUCT GTY((lang_struct))
#else
/* For C, we'll use a special attribute that gengtype might recognize */
#define GTY_LANG_STRUCT GTY((user))
#endif

struct GTY_LANG_STRUCT lang_specific {
    int lang_specific_field;
    void *lang_data;
};

/* Complex nested type to ensure thorough traversal */
struct GTY(()) complex_nested {
    struct pointer_container pointers;
    struct array_container arrays;
    struct string_container strings;
    union data_union data;
    struct GTY((skip)) {
        int hidden_field;
    } hidden;
    
    /* Self-referential pointer */
    struct complex_nested * GTY((tag("2"))) next;
    
    /* Array of pointers */
    struct base_struct * GTY((length("$->ptr_count"))) *ptr_array;
    int ptr_count;
};

/* Enumeration type */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY((user));

/* Function pointer typedef */
typedef int (*comparator_func)(const void *, const void *);

/* Another union with nested struct */
union GTY(()) nested_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        float r;
        float g;
        float b;
    } color;
};

/* Template-like structure using macros */
#define DECLARE_CONTAINER(TYPE, NAME) \
    struct GTY(()) NAME { \
        TYPE * GTY((length("$->count"))) items; \
        int count; \
        int capacity; \
    }

DECLARE_CONTAINER(int, int_container);
DECLARE_CONTAINER(struct base_struct, struct_container);
DECLARE_CONTAINER(char, char_container);

/* Opaque pointer type */
typedef void *opaque_handle;

/* Variable length structure */
struct GTY(()) var_len_struct {
    int length;
    char data[1];  /* Flexible array member */
};

/* Bitfield structure */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Aligned structure */
struct GTY(()) aligned_struct {
    int data __attribute__((aligned(16)));
    char padding[12];
};

/* Packed structure */
struct GTY(()) packed_struct {
    char a;
    int b __attribute__((packed));
    char c;
};

#endif /* TEST_TYPES_H */
