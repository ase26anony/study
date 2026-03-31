#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"

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

/* TYPE_STRUCT: Regular struct with complex members */
struct GTY(()) complex_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    struct complex_struct* GTY((tag("0"))) next;
    struct complex_struct* GTY((tag("1"))) prev;
    
    /* Anonymous struct */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } position;
    
    /* Bit-fields */
    unsigned GTY(()) flags : 4;
    unsigned GTY(()) status : 2;
    
    /* Nested struct */
    struct GTY(()) inner {
        float GTY(()) value;
        char GTY(()) label[16];
    } inner_data;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) custom_id;
    void* GTY((skip)) user_data;
};

/* TYPE_UNION: Union types */
union GTY(()) data_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY(()) string_val;
    struct complex_struct* GTY(()) struct_ptr;
};

/* Tagged union within a struct */
struct GTY(()) tagged_container {
    enum GTY(()) { INT_TYPE, DOUBLE_TYPE, STRING_TYPE } tag;
    union GTY(()) {
        int GTY(()) i;
        double GTY(()) d;
        char* GTY((length("strlen($)"))) s;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct complex_struct* GTY(()) struct_ptr_t;
typedef union data_union* GTY(()) union_ptr_t;
typedef void (*GTY(()) callback_func)(int, const char*);
typedef void* GTY(()) generic_ptr;

/* Function pointer type for TYPE_CALLBACK */
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct complex_struct GTY(()) struct_array[5];
typedef union data_union GTY(()) union_array[8];
typedef char* GTY(()) string_array[4];

/* Multi-dimensional arrays */
typedef int GTY(()) matrix[3][3];
typedef struct complex_struct* GTY(()) ptr_matrix[2][2];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen($)"))) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* Container struct that uses all type kinds */
struct GTY(()) master_container {
    /* TYPE_SCALAR */
    int GTY(()) counter;
    color_t GTY(()) color;
    
    /* TYPE_STRUCT */
    struct complex_struct GTY(()) data;
    
    /* TYPE_UNION */
    union data_union GTY(()) variant;
    
    /* TYPE_POINTER */
    struct complex_struct* GTY(()) next;
    union data_union* GTY(()) union_ref;
    generic_ptr GTY(()) opaque_data;
    
    /* TYPE_ARRAY */
    int_array GTY(()) numbers;
    string_array GTY(()) messages;
    
    /* TYPE_STRING */
    string_ptr GTY((length("strlen($)"))) dynamic_string;
    const_string_ptr GTY(()) static_string;
    
    /* TYPE_CALLBACK */
    compare_func GTY(()) sorter;
    callback_func GTY(()) notifier;
    
    /* Chain pointers for GC */
    struct master_container* GTY((chain_next("$->chain"))) chain;
    
    /* Length field for variable array */
    size_t GTY(()) item_count;
    
    /* Variable length array at end */
    struct complex_struct* GTY((length("$->item_count"))) items[];
};

/* Linked list node using chain_next */
struct GTY(()) list_node {
    int GTY(()) value;
    struct list_node* GTY((chain_next("$->next"))) next;
    struct list_node* GTY((chain_prev("$->prev"))) prev;
};

#endif /* TEST_TYPES_H */
