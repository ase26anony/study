/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"  /* GCC's type description header */

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int GTY((skip)) x;           /* Skip this field in marking */
    double y;
    char GTY((length("str_len"))) *name;  /* String with length */
    int str_len;
    struct basic_struct *GTY((chain_next)) next;  /* Linked list */
};

/* TYPE_STRUCT with bitfields and anonymous struct */
struct GTY(()) complex_struct {
    unsigned int GTY((bitfield("3"))) flags:3;
    unsigned int GTY((bitfield("5"))) mode:5;
    
    struct GTY(()) {  /* Anonymous struct */
        int inner_x;
        double inner_y;
    } anonymous;
    
    union GTY(()) {   /* Anonymous union */
        int as_int;
        double as_double;
    } value;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marking */
struct GTY((user)) user_struct {
    int id;
    char *GTY((tag("0"))) data;
};

/* TYPE_UNION: Tagged union */
union GTY(()) tagged_union {
    int GTY((tag("type == 0"))) int_value;
    double GTY((tag("type == 1"))) double_value;
    char *GTY((tag("type == 2"))) string_value;
    struct basic_struct *GTY((tag("type == 3"))) struct_ptr;
};

struct GTY(()) union_container {
    int type;
    union tagged_union value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *GTY(()) struct_ptr_t;
typedef union tagged_union *GTY(()) union_ptr_t;
typedef void *GTY(()) void_ptr_t;
typedef int *GTY(()) int_ptr_t;
typedef int (*GTY(()) func_ptr_t)(int, double);  /* Function pointer */

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef int GTY(()) multi_dim_array[3][4][5];

/* TYPE_STRING: String types */
typedef char *GTY((string)) string_ptr;
typedef const char *GTY((string)) const_string_ptr;

/* TYPE_CALLBACK: Callback function pointer in struct */
typedef void (*GTY(()) callback_func)(int, void*);

struct GTY(()) callback_container {
    callback_func GTY((callback)) handler;
    void *GTY((skip)) user_data;
};

/* Complex type with all dependencies */
struct GTY(()) master_struct {
    /* TYPE_STRUCT */
    struct basic_struct nested;
    
    /* TYPE_POINTER to various types */
    struct opaque_struct *opaque_ptr;      /* TYPE_UNDEFINED pointer */
    struct user_struct *user_ptr;          /* TYPE_USER_STRUCT pointer */
    union tagged_union *union_ptr;         /* TYPE_UNION pointer */
    
    /* TYPE_ARRAY */
    int_array numbers;
    struct_array objects;
    
    /* TYPE_SCALAR */
    color_t color;
    scalar_double weight;
    
    /* TYPE_STRING */
    string_ptr name;
    const_string_ptr const_name;
    
    /* TYPE_CALLBACK */
    callback_func on_event;
    
    /* TYPE_POINTER chains */
    struct master_struct *GTY((chain_next)) next;
    struct master_struct *GTY((chain_prev)) prev;
    
    /* Array of pointers */
    struct basic_struct *GTY(()) *ptr_array[8];
    
    /* Multi-dimensional array */
    multi_dim_array matrix;
    
    /* Function pointer array */
    callback_func GTY(()) handlers[3];
};

/* Global variable declarations for gengtype to process */
extern struct master_struct GTY(()) *global_master;
extern struct basic_struct GTY(()) global_basic_array[3];
extern union tagged_union GTY(()) global_union;
extern int GTY(()) global_scalar;
extern string_ptr GTY(()) global_string;

#endif /* TEST_TYPES_H */
