/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;
typedef struct opaque *opaque_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int_t;
typedef long GTY(()) scalar_long_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    scalar_int_t id;
    scalar_char_t initial;
    scalar_float_t value;
    color_t color;
};

/* TYPE_STRUCT: Nested struct with bitfields */
struct GTY(()) complex_struct {
    struct basic_struct GTY((tag("0"))) base;
    
    /* Bitfields */
    unsigned int GTY((bitfield("1"))) flag1 : 1;
    unsigned int GTY((bitfield("1"))) flag2 : 1;
    unsigned int GTY((bitfield("6"))) bits : 6;
    
    /* Anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } point;
    
    /* TYPE_POINTER: Pointer to another struct */
    struct complex_struct *GTY((skip)) next;
    
    /* TYPE_POINTER: Pointer to union */
    union data_union *GTY(()) data_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with custom handling */
struct GTY((user)) user_struct {
    int custom_id;
    char *GTY((length("strlen(%h.name)+1"))) name;
};

/* TYPE_UNION: Basic union */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    scalar_double_t as_double;
    struct basic_struct GTY((tag("1"))) as_struct;
};

/* TYPE_UNION: Tagged union within struct */
struct GTY(()) tagged_container {
    enum { INT_TYPE, FLOAT_TYPE, STRUCT_TYPE } type_tag;
    
    union GTY((desc("%0.type_tag"))) {
        scalar_int_t GTY((tag("INT_TYPE"))) int_val;
        scalar_float_t GTY((tag("FLOAT_TYPE"))) float_val;
        struct basic_struct GTY((tag("STRUCT_TYPE"))) struct_val;
    } value;
};

/* TYPE_ARRAY: Various array types */
struct GTY(()) array_container {
    /* Fixed-size array of scalars */
    scalar_int_t GTY(()) numbers[10];
    
    /* Array of structs */
    struct basic_struct GTY(()) items[5];
    
    /* Multi-dimensional array */
    scalar_float_t GTY(()) matrix[3][3];
    
    /* Pointer to array (TYPE_POINTER + TYPE_ARRAY) */
    scalar_double_t (*GTY(()) array_ptr)[4];
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_container {
    /* Simple pointers */
    scalar_int_t *GTY(()) int_ptr;
    struct basic_struct *GTY(()) struct_ptr;
    union data_union *GTY(()) union_ptr;
    
    /* Pointer to pointer */
    scalar_int_t **GTY(()) int_ptr_ptr;
    
    /* Void pointer */
    void *GTY(()) generic_ptr;
    
    /* Pointer to opaque (TYPE_UNDEFINED) */
    opaque_ptr_t opaque_ptr;
    
    /* Self-referential pointer */
    struct pointer_container *GTY(()) self;
};

/* TYPE_STRING: String types */
struct GTY(()) string_container {
    /* String literal pointer */
    const char *GTY((length("strlen(%h.text)"))) text;
    
    /* Fixed string array */
    char GTY(()) fixed_string[256];
    
    /* Pointer with string marker */
    char *GTY((string)) dynamic_string;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_func_t)(int, const char*);

struct GTY(()) callback_container {
    /* Function pointer member */
    callback_func_t GTY(()) handler;
    
    /* Array of function pointers */
    callback_func_t GTY(()) handlers[3];
    
    /* Struct with callback */
    struct {
        callback_func_t GTY(()) notify;
        int GTY(()) context;
    } listener;
};

/* Chain-linked structure for chain_next/chain_prev */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chain_node {
    int id;
    struct chain_node *GTY((skip)) next;
    struct chain_node *GTY((skip)) prev;
};

/* Root structure containing all types */
struct GTY(()) type_root {
    struct basic_struct basic;
    struct complex_struct complex;
    struct user_struct user;
    union data_union data;
    struct tagged_container tagged;
    struct array_container arrays;
    struct pointer_container pointers;
    struct string_container strings;
    struct callback_container callbacks;
    struct chain_node *GTY((length("%h.node_count"))) node_list;
    int node_count;
};

/* Global variable declarations */
extern struct type_root GTY(()) global_root;
extern struct chain_node GTY(()) global_nodes[5];

#endif /* TEST_TYPES_H */
