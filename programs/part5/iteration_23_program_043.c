/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY header */
#include "gtype-desc.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color_enum {
    RED,
    GREEN,
    BLUE
} color_enum;

typedef int GTY(()) scalar_int;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef long GTY(()) scalar_long;
typedef char GTY(()) scalar_char;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    float GTY(()) value;
    color_enum GTY(()) color;
    
    /* Anonymous struct within struct */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } position;
    
    /* Bit-fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) status : 2;
};

/* TYPE_STRUCT with nested structures */
struct GTY(()) complex_struct {
    struct basic_struct GTY(()) base;
    
    /* Pointer chain using GTY options */
    struct complex_struct *GTY((skip)) next;
    struct complex_struct *GTY((chain_next("next"), chain_prev("prev"))) prev;
    
    /* Array within struct */
    int GTY(()) scores[10];
    
    /* Flexible array member */
    struct basic_struct *GTY((length("flex_count"))) flex_array;
    int flex_count;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) user_id;
    char *GTY(()) user_name;
    void *GTY((skip)) user_data;
};

/* TYPE_UNION: Basic union type */
union GTY(()) basic_union {
    int GTY(()) int_val;
    float GTY(()) float_val;
    char *GTY(()) string_val;
    struct basic_struct GTY(()) struct_val;
};

/* TYPE_UNION: Tagged union within a struct */
struct GTY(()) tagged_union_container {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE, STRUCT_TYPE } GTY(()) tag;
    
    union GTY((desc("tag"))) {
        int GTY(()) int_member;
        float GTY(()) float_member;
        char *GTY(()) string_member;
        struct basic_struct GTY(()) struct_member;
    } GTY(()) value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *GTY(()) struct_ptr;
typedef union basic_union *GTY(()) union_ptr;
typedef int *GTY(()) int_ptr;
typedef void *GTY(()) void_ptr;
typedef void (*GTY(()) func_ptr)(void);

/* Function pointer type for TYPE_CALLBACK */
typedef int (*GTY(()) callback_func)(int, char *);

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[3];
typedef int *GTY(()) ptr_array[8];

/* Multi-dimensional arrays */
typedef int GTY(()) matrix[3][3];
typedef struct basic_struct GTY(()) struct_matrix[2][2];

/* TYPE_STRING: String types */
typedef char *GTY((length("strlen(%h)"))) string_ptr;
typedef const char *GTY(()) const_string_ptr;

/* Struct with string members */
struct GTY(()) string_struct {
    char *GTY((length("name_len"))) name;
    int name_len;
    const char *GTY(()) constant_string;
    char buffer[256];
};

/* TYPE_CALLBACK: Struct with callback function pointer */
struct GTY(()) callback_container {
    int GTY(()) id;
    callback_func GTY(()) handler;
    void *GTY(()) user_data;
};

/* Chain of structures for testing traversal */
struct GTY(()) chain_node {
    int GTY(()) data;
    struct chain_node *GTY((chain_next("next"))) next;
    struct chain_node *GTY((chain_prev("prev"))) prev;
};

/* Global variable declarations */
extern struct basic_struct GTY(()) global_struct;
extern union basic_union GTY(()) global_union;
extern struct complex_struct *GTY(()) global_complex_ptr;
extern int_array GTY(()) global_int_array;
extern string_ptr GTY(()) global_string;

#endif /* TEST_TYPES_H */
