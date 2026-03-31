/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"  /* For GTY macro definitions */

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_struct GTY((user));

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color GTY(());

typedef int scalar_int GTY(());
typedef double scalar_double GTY(());
typedef float scalar_float GTY(());

/* TYPE_STRING: String type */
typedef char* string_type GTY((length("strlen($) + 1")));

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func)(int, void*) GTY(());

/* TYPE_UNION: Union types */
union basic_union GTY(()) {
    int as_int;
    double as_double;
    void* as_ptr;
};

union tagged_union GTY(()) {
    struct {
        int tag;
        union {
            int int_val;
            double double_val;
            char* string_val;
        } data GTY(());
    };
};

/* TYPE_STRUCT: Regular struct types */
struct simple_struct GTY(()) {
    int id;
    char name[32];
    double value;
};

struct nested_struct GTY(()) {
    struct simple_struct base GTY(());
    union basic_union data GTY(());
    enum color color GTY(());
    
    /* Bit-fields */
    unsigned int flags : 4;
    unsigned int status : 2;
    
    /* Anonymous struct */
    struct {
        int x;
        int y;
    } point GTY(());
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    int magic_number;
    void* user_data;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_struct GTY(()) {
    /* Simple pointers */
    struct simple_struct* next GTY((chain_next));
    struct nested_struct* prev GTY((chain_prev));
    
    /* Pointer to pointer */
    struct simple_struct** ptr_to_ptr GTY(());
    
    /* Void pointer */
    void* void_ptr GTY(());
    
    /* Function pointer */
    callback_func handler GTY(());
    
    /* Pointer to union */
    union basic_union* union_ptr GTY(());
    
    /* Pointer to scalar */
    int* int_ptr GTY(());
    
    /* Pointer to array */
    int (*array_ptr)[10] GTY(());
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    /* Fixed-size arrays */
    int simple_array[10] GTY(());
    double multi_dim[5][5] GTY(());
    
    /* Array of structs */
    struct simple_struct struct_array[8] GTY(());
    
    /* Array of unions */
    union basic_union union_array[4] GTY(());
    
    /* Array of pointers */
    struct simple_struct* pointer_array[6] GTY(());
    
    /* Array of strings */
    char* string_array[3] GTY((length("strlen($) + 1")));
    
    /* Zero-length array (GCC extension) */
    int flexible_array[] GTY((length("flexible_len")));
    int flexible_len;
};

/* Complex type with all dependencies */
struct master_struct GTY(()) {
    /* TYPE_STRUCT */
    struct nested_struct nested GTY(());
    
    /* TYPE_USER_STRUCT */
    struct user_defined_struct* user_struct_ptr GTY(());
    
    /* TYPE_UNION */
    union tagged_union tagged GTY(());
    
    /* TYPE_POINTER */
    struct pointer_struct* pointers GTY((chain_next));
    
    /* TYPE_ARRAY */
    struct array_struct arrays[2] GTY(());
    
    /* TYPE_SCALAR */
    enum color primary_color GTY(());
    scalar_int counter GTY(());
    scalar_double precision GTY(());
    
    /* TYPE_STRING */
    string_type description GTY((length("strlen($) + 1")));
    
    /* TYPE_CALLBACK */
    callback_func on_event GTY(());
    
    /* Chain linking */
    struct master_struct* next GTY((chain_next));
    struct master_struct* prev GTY((chain_prev));
    
    /* For variable-length array */
    int dynamic_count GTY(());
    struct simple_struct* dynamic_array GTY((length("dynamic_count")));
};

/* Global variable declarations */
extern struct master_struct* global_master_list GTY((chain_next));
extern struct array_struct global_array_instance GTY(());
extern union basic_union global_union_instance GTY(());
extern callback_func global_callback GTY(());

#endif /* TEST_TYPES_H */
