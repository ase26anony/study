#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct GTY(()) string_struct {
    char* GTY((length("str_len"))) data;
    int str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) plain_struct {
    int field1;
    double field2;
    my_scalar scalar_field;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_struct {
    int user_data;
    void* GTY((skip)) user_pointer;  /* Skip this in GC */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int int_val;
    double double_val;
    char* GTY((tag("0"))) str_val;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_struct {
    struct plain_struct* GTY((desc("1"))) ptr1;
    struct string_struct** GTY((skip)) ptr2;  /* Skip double pointer */
    void* GTY((null)) null_ptr;
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_struct {
    int GTY((length("array_len"))) variable_array[1];
    int array_len;
    int fixed_array[10];
    struct plain_struct* GTY((length("ptr_array_len"))) ptr_array[1];
    int ptr_array_len;
};

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct GTY(()) callback_struct {
    callback_func handler;
    void* GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct("C"))) lang_specific_struct {
    int lang_data;
    const char* GTY((length("name_len"))) name;
    int name_len;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    union my_union variant;
    struct array_struct arrays;
    struct pointer_struct* GTY((desc("0"))) next;
    struct lang_specific_struct lang_part;
};

/* Forward declarations for circular references */
struct GTY(()) forward_decl;
struct GTY(()) another_forward;

struct GTY(()) forward_decl {
    int data;
    struct another_forward* GTY((desc("1"))) link;
};

struct GTY(()) another_forward {
    char* GTY((length("len"))) name;
    int len;
    struct forward_decl* GTY((desc("0"))) backlink;
};

/* TYPE_UNDEFINED: This might be trickier - incomplete/opaque types */
struct GTY(()) opaque_struct;  /* Forward declaration without definition */

/* Function pointer typedefs */
typedef int (*comparator_func)(const void*, const void*) GTY(());

/* Enum type (also scalar) */
typedef enum {
    STATE_A,
    STATE_B,
    STATE_C
} my_enum GTY(());

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int flag3:3;
    unsigned int padding:26;
};

/* Variable length struct with nested callbacks */
struct GTY(()) variable_struct {
    int type;
    union {
        int int_value;
        double double_value;
        callback_func callback_value;
        struct string_struct* GTY((desc("1"))) string_value;
    } GTY((desc("type"))) data;
};

#endif /* TEST_TYPES_H */
