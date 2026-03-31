/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct GTY(()) opaque_struct;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_SCALAR: Fundamental scalar types */
enum color {
    RED,
    GREEN,
    BLUE
};

typedef enum color GTY(()) color_enum;

/* TYPE_STRING: String types */
typedef const char * GTY(()) const_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_func)(int, const char*);

/* TYPE_UNION: Union types */
union GTY(()) data_value {
    int int_val;
    float float_val;
    double double_val;
    const_string string_val;
    void * GTY((skip)) ptr_val;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef union data_value GTY(()) value_array[5];

/* TYPE_STRUCT: Basic struct type */
struct GTY(()) base_struct {
    int GTY(()) id;
    const_string GTY(()) name;
    color_enum GTY(()) color;
    
    /* Anonymous struct member */
    struct GTY(()) {
        int x;
        int y;
    } position;
    
    /* Bit fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) status : 2;
    
    /* Nested array */
    int_array GTY(()) scores;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_struct {
    /* Simple pointers */
    struct base_struct * GTY(()) base_ptr;
    union data_value * GTY(()) value_ptr;
    
    /* Pointer to pointer */
    struct base_struct ** GTY(()) base_pptr;
    
    /* Void pointer */
    void * GTY((skip)) generic_ptr;
    
    /* Function pointer */
    callback_func GTY(()) callback;
    
    /* Self-referential pointer */
    struct pointer_struct * GTY(()) next;
    
    /* Pointer to undefined type */
    opaque_ptr GTY(()) opaque;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) user_id;
    const_string GTY(()) user_name;
    
    /* Chain pointers for user struct */
    struct user_struct * GTY((chain_next)) next_user;
    struct user_struct * GTY((chain_prev)) prev_user;
};

/* Complex struct with all type kinds */
struct GTY(()) complex_struct {
    /* Scalar members */
    int GTY(()) count;
    long GTY(()) total;
    float GTY(()) average;
    double GTY(()) precision;
    color_enum GTY(()) primary_color;
    
    /* String member */
    const_string GTY(()) description;
    
    /* Struct member */
    struct base_struct GTY(()) base;
    
    /* Union member */
    union data_value GTY(()) data;
    
    /* Pointer members */
    struct pointer_struct * GTY(()) ptr_struct;
    struct user_struct * GTY(()) user;
    
    /* Array members */
    int_array GTY(()) numbers;
    value_array GTY(()) values;
    
    /* Callback member */
    callback_func GTY(()) handler;
    
    /* Anonymous union */
    union GTY(()) {
        int tag;
        float value;
    } tag_union;
    
    /* Length field for variable array */
    unsigned int GTY(()) array_len;
    
    /* Variable length array (requires length field) */
    struct base_struct * GTY((length ("array_len"))) var_array;
};

/* Union with struct inside */
union GTY(()) nested_union {
    struct GTY(()) {
        int type;
        union data_value data;
    } tagged;
    long raw_data;
};

/* Struct with chain_next/chain_prev for linked list */
struct GTY(()) linked_node {
    int GTY(()) value;
    struct linked_node * GTY((chain_next)) next;
    struct linked_node * GTY((chain_prev)) prev;
    
    /* Descendant tag for discriminated union */
    int GTY((desc ("%0.type"))) desc_tag;
    union nested_union GTY(()) payload;
};

#endif /* TEST_TYPES_H */
