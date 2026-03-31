/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY headers */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
enum color {
    RED,
    GREEN,
    BLUE
};

typedef enum color GTY(()) color_t;

/* TYPE_STRUCT: Regular struct with various members */
struct GTY(()) base_struct {
    int GTY((skip)) x;           /* scalar field */
    char GTY((skip)) c;
    float GTY((skip)) f;
    double GTY((skip)) d;
    long GTY((skip)) l;
    enum color GTY((skip)) col;  /* enum scalar */
    
    /* Anonymous struct within struct */
    struct GTY(()) {
        int GTY((skip)) inner_a;
        int GTY((skip)) inner_b;
    } GTY((skip)) anonymous;
    
    /* Bit-fields */
    unsigned int GTY((skip)) flags:4;
    unsigned int GTY((skip)) mode:3;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY((skip)) user_data;
    char GTY((skip)) user_name[32];
};

/* TYPE_UNION: Union types */
union GTY(()) data_union {
    int GTY((skip)) i;
    float GTY((skip)) f;
    char GTY((skip)) str[16];
    struct base_struct GTY((skip)) *s;
};

/* Tagged union within a struct */
struct GTY(()) tagged_container {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE } GTY((skip)) tag;
    union GTY(()) {
        int GTY((skip)) int_val;
        float GTY((skip)) float_val;
        char GTY((skip)) *string_val;
    } GTY((skip)) data;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_struct {
    /* Simple pointers */
    struct base_struct GTY((skip)) *next;
    union data_union GTY((skip)) *union_ptr;
    
    /* Pointer to pointer */
    struct base_struct GTY((skip)) **pp;
    
    /* Void pointer */
    void GTY((skip)) *generic;
    
    /* Function pointer (callback type) */
    int GTY((callback)) (*compare_func)(const void*, const void*);
    
    /* Chain of pointers for GC */
    struct pointer_struct GTY((chain_next)) *chain_next;
    struct pointer_struct GTY((chain_prev)) *chain_prev;
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
    /* Fixed-size arrays */
    int GTY((skip)) numbers[10];
    struct base_struct GTY((skip)) structs[5];
    union data_union GTY((skip)) unions[3];
    
    /* Multi-dimensional arrays */
    int GTY((skip)) matrix[3][3];
    char GTY((skip)) strings[4][32];
    
    /* Array of pointers */
    struct base_struct GTY((skip)) *ptr_array[8];
};

/* TYPE_STRING: String types */
struct GTY(()) string_container {
    /* String literal pointer */
    const char GTY((length("strlen(%h.str_field)"))) *str_field;
    
    /* Array of chars (string) */
    char GTY((skip)) name[64];
    
    /* Multiple string pointers */
    char GTY((length("strlen(%h.dynamic_str)"))) *dynamic_str;
    const char GTY((length("strlen(%h.const_str)"))) *const_str;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*comparator_t)(const void*, const void*);
typedef void GTY((callback)) (*callback_func_t)(int, const char*);

struct GTY(()) callback_container {
    comparator_t GTY((skip)) cmp;
    callback_func_t GTY((skip)) handler;
    
    /* Array of callbacks */
    callback_func_t GTY((skip)) handlers[4];
};

/* Complex nested structure with all type kinds */
struct GTY(()) master_container {
    /* Scalar fields */
    int GTY((skip)) id;
    enum color GTY((skip)) color;
    
    /* Struct field */
    struct base_struct GTY((skip)) base;
    
    /* User struct field */
    struct user_struct GTY((skip)) user;
    
    /* Union field */
    union data_union GTY((skip)) data;
    
    /* Pointer fields */
    struct pointer_struct GTY((skip)) *ptr;
    struct opaque_struct GTY((skip)) *opaque_ptr;
    
    /* Array fields */
    struct array_container GTY((skip)) arrays[2];
    
    /* String fields */
    struct string_container GTY((skip)) strings;
    
    /* Callback field */
    struct callback_container GTY((skip)) callbacks;
    
    /* Nested anonymous union */
    union GTY(()) {
        int GTY((skip)) a;
        struct GTY(()) {
            int GTY((skip)) x;
            int GTY((skip)) y;
        } GTY((skip)) point;
    } GTY((skip)) variant;
};

/* Variable length array structure */
struct GTY(()) varray_struct {
    int GTY((skip)) length;
    int GTY((length("%0.length"))) items[1];
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int GTY((skip)) value;
    struct tree_node GTY((skip)) *left;
    struct tree_node GTY((skip)) *right;
    struct tree_node GTY((skip)) *parent;
};

#endif /* TEST_TYPES_H */
