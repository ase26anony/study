#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());
typedef unsigned long another_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(()) {
    char * GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_marked_struct GTY((user)) {
    void *custom_data;
    int user_tag;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int as_int;
    double as_double;
    void *as_ptr;
    struct plain_struct *as_struct;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_container GTY(()) {
    struct plain_struct * GTY((skip)) direct_ptr;
    union my_union * GTY((tag("union_tag"))) union_ptr;
    struct string_struct ** GTY((skip)) double_ptr;
};

/* TYPE_ARRAY: Array types - fixed size and variable length */
struct array_container GTY(()) {
    int fixed_array[10];
    char * GTY((length("var_len"))) variable_array;
    size_t var_len;
    
    /* Nested array in struct */
    struct {
        float matrix[3][3];
    } nested GTY(());
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (*callback_func)(int, const char *) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void (* GTY((callback)) another_handler)(struct plain_struct *);
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

/* Complex nested type to ensure deep traversal */
struct complex_nested GTY(()) {
    struct pointer_container *pointers;
    struct array_container arrays;
    union my_union choice;
    struct string_struct str_field;
    
    /* Self-referential pointer */
    struct complex_nested * GTY((skip)) next;
};

/* Forward declaration for circular reference */
struct forward_declared GTY(());

struct circular_ref GTY(()) {
    struct forward_declared *fwd_ptr;
    int value;
};

struct forward_declared GTY(()) {
    struct circular_ref *back_ptr;
    char name[32];
};

/* TYPE_UNDEFINED: This might be triggered by incomplete types or special cases */
/* Incomplete struct declaration */
struct incomplete GTY(());

/* Enum type (treated as scalar in some contexts) */
typedef enum {
    RED,
    GREEN,
    BLUE
} color_enum GTY(());

/* Bitfield struct */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Volatile and const qualified types */
struct qualified_types GTY(()) {
    volatile int volatile_member;
    const char *const_string;
    struct plain_struct * GTY((skip)) restrict_ptr;
};

/* Packed struct with attribute */
struct packed_struct GTY(()) {
    char a;
    int b;
    char c;
} __attribute__((packed));

/* Aligned struct */
struct aligned_struct GTY(()) {
    double data;
    int counter;
} __attribute__((aligned(64)));

/* Transparent union */
typedef union transparent_union GTY(()) {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t __attribute__((transparent_union));

#endif /* TEST_TYPES_H */
