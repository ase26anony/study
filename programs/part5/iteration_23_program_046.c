/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
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

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%h) + 1"))) string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int ival;
    float fval;
    char* GTY((length("strlen(%h) + 1"))) sval;
};

union GTY(()) tagged_union {
    int tag;
    struct {
        int x;
        int y;
    } GTY(()) point;
    struct {
        float radius;
        color_t color;
    } GTY(()) circle;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef union GTY(()) basic_union union_array[5];

/* TYPE_STRUCT: Basic struct types */
struct GTY(()) simple_struct {
    int id;
    char name[32];
    float value;
};

struct GTY(()) complex_struct {
    int GTY(()) count;
    int_array GTY(()) numbers;
    union GTY(()) basic_union data;
    struct GTY(()) simple_struct* GTY(()) next;
    struct GTY(()) complex_struct* GTY(()) prev;
    
    /* Anonymous struct */
    struct {
        int x;
        int y;
    } GTY(()) position;
    
    /* Bit fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_id;
    char* GTY((length("strlen(%h) + 1"))) user_name;
};

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) simple_struct* simple_ptr;
typedef union GTY(()) basic_union* union_ptr;
typedef void* GTY(()) void_ptr;
typedef int* GTY(()) int_ptr;
typedef callback_func GTY(()) callback_ptr;

/* Nested pointer types */
typedef struct GTY(()) complex_struct** double_ptr;
typedef int*** GTY(()) triple_ptr;

/* TYPE_STRUCT with TYPE_POINTER members */
struct GTY(()) pointer_rich_struct {
    simple_ptr GTY(()) sp;
    union_ptr GTY(()) up;
    void_ptr GTY(()) vp;
    int_ptr GTY(()) ip;
    callback_ptr GTY(()) cp;
    struct opaque_struct* GTY(()) opaque_ptr;
    double_ptr GTY(()) dbl_ptr;
    triple_ptr GTY(()) trpl_ptr;
    
    /* Array of pointers */
    simple_ptr GTY(()) ptr_array[5];
    
    /* Pointer to array */
    int (*GTY(()) array_ptr)[10];
};

/* Self-referential struct */
struct GTY(()) linked_node {
    int data;
    struct GTY(()) linked_node* GTY((chain_next("%h.next"))) next;
    struct GTY(()) linked_node* GTY((chain_prev("%h.prev"))) prev;
};

/* Union within struct */
struct GTY(()) struct_with_union {
    int type;
    union {
        int int_val;
        float float_val;
        char* GTY((length("strlen(%h) + 1"))) str_val;
    } GTY(()) value;
};

/* Array within struct */
struct GTY(()) struct_with_arrays {
    int GTY(()) matrix[3][3];
    struct GTY(()) simple_struct objects[5];
    string_ptr GTY(()) strings[4];
};

/* Callback in struct */
struct GTY(()) struct_with_callback {
    int id;
    callback_func GTY(()) handler;
    void (*GTY(()) another_handler)(struct GTY(()) struct_with_callback*);
};

#endif /* TEST_TYPES_H */
