/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_undefined;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color_enum {
    RED,
    GREEN,
    BLUE
} color_enum;

typedef int GTY(()) scalar_int;
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;

/* TYPE_UNION: Various union types */
union GTY(()) basic_union {
    int int_val;
    float float_val;
    char * GTY((length("strlen($)"))) string_val;
};

union GTY(()) tagged_union {
    enum { TAG_INT, TAG_FLOAT, TAG_PTR } tag;
    struct {
        int int_val;
    } GTY((tag("TAG_INT"))) as_int;
    struct {
        float float_val;
    } GTY((tag("TAG_FLOAT"))) as_float;
    struct {
        void *ptr_val;
    } GTY((tag("TAG_PTR"))) as_ptr;
};

/* TYPE_STRUCT: Complex struct with nested members */
struct GTY(()) inner_struct {
    int x;
    float y;
    char z;
};

struct GTY(()) complex_struct {
    /* Basic scalars */
    int id;
    color_enum color;
    
    /* Nested struct */
    struct inner_struct inner;
    
    /* Anonymous struct */
    struct GTY(()) {
        int anon_x;
        float anon_y;
    } anonymous;
    
    /* Bit fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    
    /* Pointer to self (for chains) */
    struct complex_struct * GTY((skip)) next;
    struct complex_struct * GTY((skip)) prev;
    
    /* Union member */
    union basic_union data;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_data;
    char * GTY((length("strlen($)"))) user_string;
};

/* TYPE_ARRAY: Various array types */
struct GTY(()) array_container {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Multi-dimensional array */
    float matrix[3][3];
    
    /* Array of structs */
    struct inner_struct struct_array[5];
    
    /* Array of pointers */
    void * GTY((length("array_len"))) pointer_array[8];
    
    /* Variable-length array (zero-length at end) */
    int variable_array[0];
};

/* TYPE_POINTER: Various pointer types */
typedef void * GTY(()) void_ptr;
typedef int * GTY(()) int_ptr;
typedef struct complex_struct * GTY(()) struct_ptr;
typedef union tagged_union * GTY(()) union_ptr;
typedef int (* GTY(()) func_ptr)(int, char *);
typedef void (* GTY(()) callback_func)(int, void *);

/* TYPE_STRING: String types */
typedef char * GTY((length("strlen($)"))) gty_string;
typedef const char * GTY((length("strlen($)"))) const_gty_string;

/* TYPE_CALLBACK: Callback in struct */
struct GTY(()) callback_container {
    int id;
    callback_func GTY((skip)) handler;
    void * GTY((skip)) user_data;
};

/* Chain structure for testing chain_next/chain_prev */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chain_node {
    int value;
    struct chain_node *next;
    struct chain_node *prev;
};

/* Structure with length field */
struct GTY(()) variable_length_struct {
    int length;
    int * GTY((length("%0.length"))) data;
};

/* For TYPE_LANG_STRUCT - will be defined in C++ file */
#ifdef __cplusplus
class lang_class;
#endif

#endif /* TEST_TYPES_H */
