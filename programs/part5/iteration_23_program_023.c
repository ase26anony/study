/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;  /* TYPE_UNDEFINED */

/* Basic scalar types (TYPE_SCALAR) */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;
typedef float GTY(()) scalar_float;
typedef long GTY(()) scalar_long;
typedef char GTY(()) scalar_char;

/* String type (TYPE_STRING) */
typedef const char * GTY((length("strlen($1)"))) string_ptr;

/* Callback type (TYPE_CALLBACK) */
typedef void (*GTY(()) callback_func)(int, const char*);

/* Union type (TYPE_UNION) */
union GTY(()) data_union {
    int GTY((tag("0"))) int_val;
    double GTY((tag("1"))) double_val;
    const char* GTY((tag("2"))) string_val;
    void* GTY((tag("3"))) ptr_val;
};

/* Array type (TYPE_ARRAY) */
typedef int GTY(()) int_array[10];
typedef struct simple_struct* GTY(()) ptr_array[5];

/* Pointer types (TYPE_POINTER) */
typedef struct simple_struct* GTY(()) struct_ptr;
typedef union data_union* GTY(()) union_ptr;
typedef void (*GTY(()) func_ptr)(void);
typedef void* GTY(()) void_ptr;
typedef int** GTY(()) ptr_to_ptr;

/* Simple struct (TYPE_STRUCT) */
struct GTY(()) simple_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    double GTY(()) value;
    
    /* Anonymous union within struct */
    union {
        int GTY(()) x;
        float GTY(()) y;
    } GTY(()) coords;
    
    /* Bit fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) status : 2;
    
    /* Pointer member */
    struct simple_struct* GTY((chain_next)) next;
    
    /* Array member */
    int GTY(()) scores[5];
    
    /* String member */
    const char* GTY(()) description;
    
    /* Callback member */
    callback_func GTY(()) handler;
    
    /* Union member */
    union data_union GTY(()) data;
};

/* Complex nested struct with all features */
struct GTY(()) complex_struct {
    /* Scalar members */
    int GTY(()) count;
    enum color GTY(()) color;
    
    /* Pointer to another struct */
    struct simple_struct* GTY(()) child;
    
    /* Array of structs */
    struct simple_struct GTY(()) items[3];
    
    /* Pointer to array */
    int* GTY(()) dynamic_array;
    
    /* Union with tag */
    union data_union GTY(()) payload;
    int GTY((desc("payload.int_val"))) payload_type;
    
    /* String */
    const char* GTY(()) label;
    
    /* Nested anonymous struct */
    struct {
        int GTY(()) x;
        int GTY(()) y;
    } GTY(()) position;
    
    /* Chain pointers */
    struct complex_struct* GTY((chain_next)) next_complex;
    struct complex_struct* GTY((chain_prev)) prev_complex;
    
    /* Callback */
    callback_func GTY(()) notify;
};

/* User struct (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int GTY(()) user_id;
    const char* GTY(()) user_name;
    
    /* User-defined marking function would be defined elsewhere */
    void* GTY(()) user_data;
};

/* Forward declaration for pointer chain */
struct GTY(()) linked_node;

struct GTY(()) linked_node {
    int GTY(()) data;
    struct linked_node* GTY((chain_next)) next;
    struct linked_node* GTY((chain_prev)) prev;
};

/* Struct with variable-length array */
struct GTY(()) varray_struct {
    int GTY(()) length;
    int GTY((length("((varray_struct*)this)->length"))) items[1];
};

#endif /* TEST_TYPES_H */
