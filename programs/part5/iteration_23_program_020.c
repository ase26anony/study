#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;  /* TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int scalar_int;
typedef char scalar_char;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    float value;
    enum color color;
};

/* TYPE_STRUCT: Complex nested struct with bitfields and anonymous struct */
struct GTY(()) complex_struct {
    /* Bitfields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* Padding */
    
    /* Anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } point;
    
    /* Nested struct */
    struct GTY(()) nested {
        int depth;
        struct basic_struct* GTY((skip)) parent;
    } inner;
    
    /* Chain pointers for GTY options */
    struct complex_struct* GTY((chain_next)) next;
    struct complex_struct* GTY((chain_prev)) prev;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int user_id;
    void* GTY((skip)) user_data;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int as_int;
    float as_float;
    char* GTY((length("strlen($)"))) as_string;
};

/* TYPE_UNION: Tagged union within a struct */
struct GTY(()) tagged_union_container {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE } tag;
    
    union GTY(()) {
        int int_value;
        float float_value;
        char* GTY((length("strlen($)"))) string_value;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct basic_struct* struct_ptr;
typedef union basic_union* union_ptr;
typedef void (*func_ptr)(int);  /* Function pointer */
typedef void* generic_ptr;

/* TYPE_ARRAY: Various array types */
typedef int int_array[10];
typedef struct basic_struct struct_array[5];
typedef union basic_union union_array[3];
typedef int multi_dim_array[3][4][5];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen($)"))) string_ptr;
typedef const char* GTY((length("strlen($)"))) const_string_ptr;

/* TYPE_CALLBACK: Callback function pointer type */
typedef int (*compare_func)(const void*, const void*);

/* Struct containing callback */
struct GTY(()) callback_container {
    compare_func GTY((skip)) comparator;
    void* GTY((skip)) user_data;
};

/* Linked list node using chain_next/chain_prev */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((chain_next)) next;
    struct list_node* GTY((chain_prev)) prev;
};

/* Variable length array structure */
struct GTY(()) var_array_struct {
    int length;
    int data[1];  /* Variable length - will use GTY((length)) */
};

/* Structure with desc tag for discriminated unions */
struct GTY((desc("%0.type"))) desc_struct {
    enum { TYPE_A, TYPE_B } type;
    union {
        int a_value;
        float b_value;
    } GTY((desc("type"))) data;
};

#endif /* TEST_TYPES_H */
