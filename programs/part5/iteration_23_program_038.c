/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"  /* For GTY macro definitions */

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;  /* TYPE_UNDEFINED */

/* Scalar types (TYPE_SCALAR) */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;

/* String type (TYPE_STRING) */
typedef char* GTY((length("strlen($)"))) string_ptr;

/* Callback type (TYPE_CALLBACK) */
typedef void (*GTY(()) callback_func)(int, const char*);

/* Union type (TYPE_UNION) */
union GTY(()) tagged_union {
    int GTY((tag("0"))) int_val;
    float GTY((tag("1"))) float_val;
    char* GTY((tag("2"))) string_val;
    struct inner_struct* GTY((tag("3"))) struct_ptr;
};

/* Simple struct for nesting */
struct GTY(()) inner_struct {
    int x;
    int y;
    color_t color;  /* Scalar enum */
};

/* Array types (TYPE_ARRAY) */
typedef struct inner_struct GTY(()) inner_array[10];
typedef int GTY(()) int_matrix[5][5];
typedef union tagged_union GTY(()) union_array[8];

/* Pointer types (TYPE_POINTER) */
typedef struct inner_struct* GTY(()) struct_ptr;
typedef union tagged_union* GTY(()) union_ptr;
typedef void* GTY(()) void_ptr;
typedef struct inner_struct** GTY(()) ptr_to_ptr;
typedef callback_func GTY(()) func_ptr_array[3];

/* Complex struct with all features (TYPE_STRUCT) */
struct GTY(()) complex_struct {
    /* Scalar members */
    int id;
    scalar_float weight;
    scalar_double precision;
    
    /* String member */
    string_ptr GTY((length("strlen($)"))) name;
    
    /* Pointer members */
    struct_ptr next;
    union_ptr data;
    void_ptr user_data;
    
    /* Array members */
    inner_array items;
    int_matrix matrix;
    
    /* Union member (anonymous union) */
    union {
        int as_int;
        float as_float;
        char* as_string;
    } GTY(()) value;
    
    /* Bit fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    
    /* Callback member */
    callback_func handler;
    
    /* Nested struct (anonymous) */
    struct {
        int depth;
        struct complex_struct* parent;
    } GTY(()) nested;
    
    /* Chain pointers for GC */
    struct complex_struct* GTY((skip)) skip_ptr;
    struct complex_struct* GTY((chain_next("next"))) chain_next;
    struct complex_struct* GTY((chain_prev("prev"))) chain_prev;
};

/* User struct (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int user_id;
    char* user_name;
    struct complex_struct* data;
};

/* Struct with variable-length array */
struct GTY(()) var_struct {
    int count;
    int GTY((length("count"))) items[];
};

/* Self-referential struct */
struct GTY(()) tree_node {
    int value;
    struct tree_node* GTY((left)) left;
    struct tree_node* GTY((right)) right;
    struct tree_node* GTY((parent)) parent;
};

#endif /* TEST_TYPES_H */
