#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar typedef */
typedef int my_scalar GTY(());
typedef unsigned long my_unsigned_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct GTY(()) string_struct {
    char* GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_CALLBACK: Function pointer/callback type */
typedef void (*callback_func)(void* GTY(())) GTY((callback));

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) plain_struct {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_struct {
    int id;
    void* GTY((skip)) user_data;  /* Skip for user handling */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int as_int;
    double as_double;
    char* GTY((tag("0"))) as_string;
    struct plain_struct* GTY((tag("1"))) as_struct;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_container {
    struct plain_struct* GTY(()) ptr_to_struct;
    struct string_struct** GTY(()) ptr_to_ptr;
    void* GTY((atomic)) atomic_ptr;
    const char* GTY(()) const_ptr;
};

/* TYPE_ARRAY: Array types (fixed and variable length) */
struct GTY(()) array_container {
    int GTY(()) fixed_array[10];
    char* GTY((length("dyn_len"))) dynamic_array;
    size_t dyn_len;
    
    /* Nested array in struct */
    struct plain_struct GTY(()) struct_array[5];
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct("c"))) lang_specific {
    int language_specific_field;
    void* GTY((lang_hook)) language_hook;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    /* TYPE_POINTER */
    struct complex_nested* GTY(()) next;
    
    /* TYPE_ARRAY of pointers */
    struct plain_struct* GTY(()) ptr_array[8];
    
    /* TYPE_UNION */
    union my_union GTY(()) data;
    
    /* TYPE_STRING */
    char* GTY((length("name_len"))) name;
    size_t name_len;
    
    /* TYPE_CALLBACK */
    callback_func GTY(()) handler;
};

/* Forward declarations to create circular references */
struct GTY(()) forward_decl;
typedef struct forward_decl forward_decl_t;

struct GTY(()) forward_decl {
    forward_decl_t* GTY(()) next;
    int value;
};

/* TYPE_UNDEFINED: Create through incomplete/opaque type */
struct GTY(()) opaque_container {
    void* GTY((skip)) opaque_data;  /* Will be treated as undefined */
    struct incomplete* GTY(()) incomplete_ptr;  /* Forward declared but not defined */
};

/* Incomplete type declaration (creates TYPE_UNDEFINED when referenced) */
struct incomplete;

/* Template-like macro to generate multiple instances */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct GTY(()) name##_struct { \
        field_type GTY(()) field; \
        int id; \
    }

/* Generate more struct types */
DECLARE_GTY_STRUCT(int_wrapper, int);
DECLARE_GTY_STRUCT(double_wrapper, double);
DECLARE_GTY_STRUCT(ptr_wrapper, void*);

/* Enumeration type (treated as scalar) */
typedef enum {
    STATE_A,
    STATE_B,
    STATE_C
} my_enum GTY(());

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Variable length struct (GCC extension) */
struct GTY(()) var_len_struct {
    int length;
    int data[];  /* Flexible array member */
};

#endif /* TEST_TYPES_H */
