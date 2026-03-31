#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_struct {
    char * GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C structs */
struct GTY(()) simple_struct {
    int x;
    double y;
};

struct GTY(()) nested_struct {
    struct simple_struct inner;
    char name[32];
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_defined_struct {
    void *custom_data;
    int (*custom_callback)(void);
};

/* TYPE_UNION: Union containing GTY-tagged fields */
union GTY(()) tagged_union {
    struct simple_struct GTY((tag("0"))) as_struct;
    struct string_struct GTY((tag("1"))) as_string;
    int GTY((tag("2"))) as_int;
};

/* TYPE_POINTER: Struct containing pointers to other GTY-tagged types */
struct GTY(()) pointer_container {
    struct simple_struct * GTY((skip)) ptr_to_struct;
    union tagged_union * GTY((skip)) ptr_to_union;
    struct string_struct ** GTY((skip)) double_ptr;
};

/* TYPE_ARRAY: Structs with various array types */
struct GTY(()) array_container {
    int fixed_array[10];
    struct simple_struct GTY((length("dynamic_len"))) *dynamic_array;
    size_t dynamic_len;
    
    /* Variable length array at end of struct */
    char GTY((length("vla_len"))) vla[];
    size_t vla_len;
};

/* TYPE_CALLBACK: Function pointer types and callback-annotated structs */
typedef int (*callback_func)(int, void *);

struct GTY((callback("my_callback"))) callback_struct {
    callback_func handler;
    void * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct with hooks */
struct GTY((lang_struct("C"))) language_specific {
    int language_id;
    void * GTY((skip)) language_data;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_type {
    struct pointer_container *pointers;
    struct array_container arrays;
    union tagged_union variant;
    struct GTY((skip)) language_specific *lang_specific;
};

/* Forward declarations to create type cycles */
struct GTY(()) forward_decl;
struct GTY(()) forward_decl {
    int value;
    struct forward_decl * GTY((skip)) next;
};

/* TYPE_UNDEFINED: Create incomplete/opaque type */
struct GTY(()) opaque_struct;
/* This will be TYPE_UNDEFINED until fully defined */

/* Template-like macro to generate multiple instances */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct GTY(()) name { \
        field_type data; \
        struct name * GTY((skip)) next; \
    }

DECLARE_GTY_STRUCT(list_int, int);
DECLARE_GTY_STRUCT(list_double, double);

/* Enumeration type (treated as scalar in some contexts) */
typedef enum {
    STATE_A,
    STATE_B,
    STATE_C
} state_enum;

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Struct with attribute for GCC extensions */
struct GTY(()) attributed_struct {
    int value __attribute__((aligned(16)));
    char name[64] __attribute__((packed));
};

#endif /* TEST_TYPES_H */
