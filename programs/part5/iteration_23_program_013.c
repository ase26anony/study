/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

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
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef char GTY(()) scalar_char;

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen($1)"))) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int GTY(()) as_int;
    double GTY(()) as_double;
    char* GTY(()) as_string;
};

union GTY(()) tagged_union {
    int GTY(()) tag;
    struct {
        int GTY(()) x;
        int GTY(()) y;
    } GTY(()) point;
    struct {
        char* GTY(()) name;
        int GTY(()) age;
    } GTY(()) person;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    double GTY(()) value;
};

struct GTY(()) nested_struct {
    int GTY(()) count;
    struct simple_struct GTY(()) items[10];
    union basic_union GTY(()) data;
};

struct GTY(()) complex_struct {
    /* Bit-fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) mode : 3;
    
    /* Anonymous struct */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
        int GTY(()) z;
    } GTY(()) position;
    
    /* Anonymous union */
    union GTY(()) {
        int GTY(()) int_val;
        float GTY(()) float_val;
    } GTY(()) value;
    
    /* Nested with chain pointers */
    struct complex_struct* GTY((chain_next("$1->next"))) next;
    struct complex_struct* GTY((chain_prev("$1->prev"))) prev;
    
    /* Array member */
    color_t GTY(()) colors[5];
    
    /* String member */
    string_ptr GTY(()) description;
    
    /* Callback member */
    callback_func GTY(()) handler;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int GTY(()) user_data;
    void* GTY(()) user_pointer;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_struct {
    /* Simple pointers */
    int* GTY(()) int_ptr;
    double* GTY(()) double_ptr;
    
    /* Pointer to struct */
    struct simple_struct* GTY(()) simple_ptr;
    
    /* Pointer to union */
    union basic_union* GTY(()) union_ptr;
    
    /* Pointer to pointer */
    int** GTY(()) int_ptr_ptr;
    
    /* Void pointer */
    void* GTY(()) void_ptr;
    
    /* Function pointer */
    callback_func* GTY(()) callback_ptr;
    
    /* Pointer to undefined type */
    struct opaque_struct* GTY(()) opaque_ptr;
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_struct {
    /* Fixed-size arrays */
    int GTY(()) int_array[20];
    double GTY(()) double_array[10][10];  /* Multi-dimensional */
    
    /* Array of structs */
    struct simple_struct GTY(()) struct_array[5];
    
    /* Array of unions */
    union basic_union GTY(()) union_array[8];
    
    /* Array of pointers */
    int* GTY(()) pointer_array[15];
    
    /* Array of strings */
    const_string_ptr GTY(()) string_array[3];
};

/* Linked list structure for chain_next/chain_prev testing */
struct GTY(()) list_node {
    int GTY(()) data;
    struct list_node* GTY((chain_next("$1->next"))) next;
    struct list_node* GTY((chain_prev("$1->prev"))) prev;
};

/* Variable length array structure */
struct GTY(()) varray_struct {
    int GTY(()) length;
    int GTY((length("$1->length"))) variable_array[1];
};

#endif /* TEST_TYPES_H */
