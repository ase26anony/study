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
typedef float GTY(()) scalar_float;
typedef char GTY(()) scalar_char;
typedef long GTY(()) scalar_long;

/* TYPE_STRING: String types */
typedef char* GTY(()) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, void*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int as_int;
    double as_double;
    void* as_ptr;
};

/* Tagged union with discriminator */
struct GTY(()) tagged_union_container {
    int tag;
    union GTY(()) {
        int int_val;
        double double_val;
        char* string_val;
    } data;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct GTY(()) simple_struct* GTY(()) ptr_array[5];

/* TYPE_STRUCT: Basic struct type */
struct GTY(()) simple_struct {
    int id;
    char name[32];
    double value;
    enum color color;
};

/* Struct with nested anonymous struct */
struct GTY(()) nested_struct {
    int outer_id;
    struct GTY(()) {
        int inner_x;
        int inner_y;
    } point;
    union GTY(()) {
        int mode;
        char flag;
    } state;
};

/* Struct with bit-fields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Struct with chain_next/chain_prev for linked list */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
    int data;
    struct linked_node* GTY((skip)) next;
    struct linked_node* GTY((skip)) prev;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int user_id;
    char* GTY((length("%h.name_len"))) user_name;
    int name_len;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_container {
    /* Simple pointers */
    int* GTY(()) int_ptr;
    double* GTY(()) double_ptr;
    void* GTY(()) void_ptr;
    
    /* Pointer to struct */
    struct simple_struct* GTY(()) struct_ptr;
    
    /* Pointer to union */
    union basic_union* GTY(()) union_ptr;
    
    /* Pointer to pointer */
    int** GTY(()) int_ptr_ptr;
    
    /* Function pointer */
    callback_func GTY(()) callback;
    
    /* Pointer to array */
    int (*GTY(()) array_ptr)[10];
    
    /* Pointer to opaque/undefined type */
    struct opaque_struct* GTY(()) opaque_ptr;
};

/* Multi-dimensional array */
struct GTY(()) matrix_container {
    int matrix[3][3];
    double values[2][4][6];
};

/* Complex struct with all type kinds */
struct GTY(()) master_struct {
    /* SCALAR */
    int id;
    enum color color;
    double weight;
    
    /* STRING */
    char* GTY((length("%h.str_len"))) dynamic_string;
    int str_len;
    
    /* ARRAY */
    int scores[5];
    struct simple_struct objects[3];
    
    /* POINTER */
    struct master_struct* GTY(()) self_ptr;
    struct pointer_container* GTY(()) ptr_container;
    
    /* UNION */
    union basic_union data;
    
    /* CALLBACK */
    callback_func GTY(()) handler;
    
    /* Nested anonymous struct */
    struct GTY(()) {
        int x, y;
    } position;
    
    /* Bit fields */
    unsigned int flags : 4;
    
    /* Chain for linked structure */
    struct master_struct* GTY((chain_next("%h.next"))) next;
};

/* Variable length array using length option */
struct GTY(()) var_len_struct {
    int count;
    int GTY((length("%h.count"))) items[];
};

#endif /* TEST_TYPES_H */
