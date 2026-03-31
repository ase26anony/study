#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Various scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;

/* TYPE_STRUCT: Regular struct with various members */
struct GTY(()) my_struct {
    int GTY(()) x;
    double GTY(()) y;
    char GTY(()) name[32];
    struct my_struct* GTY((chain_next)) next;
    struct my_struct* GTY((chain_prev)) prev;
    
    /* Anonymous struct */
    struct GTY(()) {
        int GTY(()) a;
        int GTY(()) b;
    } nested;
    
    /* Bit fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) mode : 2;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) my_user_struct {
    int GTY(()) id;
    char GTY(())* GTY((length("strlen($)"))) name;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY(()) string_val;
    struct my_struct* GTY(()) struct_ptr;
};

/* Tagged union within a struct */
struct GTY(()) tagged_union_container {
    enum GTY(()) { INT_TYPE, DOUBLE_TYPE, STRING_TYPE } tag;
    union GTY(()) {
        int GTY(()) i;
        double GTY(()) d;
        char* GTY(()) s;
    } value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_struct* GTY(()) struct_ptr_t;
typedef union my_union* GTY(()) union_ptr_t;
typedef void (*GTY(()) func_ptr_t)(int, double);
typedef void* GTY(()) void_ptr_t;
typedef struct_ptr_t* GTY(()) ptr_to_ptr_t;

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct my_struct GTY(()) struct_array[5];
typedef int GTY(()) multi_dim_array[3][4][5];

/* TYPE_STRING: String types */
typedef char* GTY(()) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY(()) callback_func)(const char*, int);
typedef void (*GTY(()) void_callback)(void);

/* Struct containing a callback */
struct GTY(()) callback_container {
    callback_func GTY(()) handler;
    void_callback GTY(()) cleanup;
    int GTY(()) data;
};

/* Complex type with all kinds of references */
struct GTY(()) master_struct {
    /* Scalar */
    int GTY(()) id;
    
    /* String */
    char* GTY((length("strlen($)"))) name;
    
    /* Struct */
    struct my_struct GTY(()) data;
    
    /* Union */
    union my_union GTY(()) value;
    
    /* Pointer */
    struct master_struct* GTY(()) next;
    
    /* Array */
    int GTY(()) scores[5];
    
    /* Array of structs */
    struct my_struct GTY(()) items[3];
    
    /* Pointer to array */
    int (*GTY(()) matrix)[4];
    
    /* Callback */
    callback_func GTY(()) notify;
    
    /* Pointer to undefined type */
    struct opaque_struct* GTY(()) opaque;
    
    /* User struct */
    struct my_user_struct GTY(()) user_data;
};

/* Chain of structures for testing chain_next/chain_prev */
struct GTY(()) chain_node {
    int GTY(()) value;
    struct chain_node* GTY((chain_next)) next;
    struct chain_node* GTY((chain_prev)) prev;
};

#endif /* TEST_TYPES_H */
