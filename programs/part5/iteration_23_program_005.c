/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;  /* TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    float value;
    enum color color;
};

/* TYPE_STRUCT: Nested struct with anonymous struct */
struct GTY(()) complex_struct {
    struct basic_struct base;
    
    /* Anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } position;
    
    /* Bit-fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
};

/* TYPE_STRUCT: Struct with chain_next/chain_prev for linked list */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
    int data;
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int int_val;
    float float_val;
    char *string_val;
    struct basic_struct *struct_ptr;
};

/* TYPE_UNION: Tagged union within a struct */
struct GTY(()) tagged_union_container {
    int tag;
    union GTY((desc("%0.tag"))) {
        int int_member;
        float float_member;
        struct basic_struct *struct_member;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct * GTY(()) struct_pointer;
typedef union basic_union * GTY(()) union_pointer;
typedef int * GTY(()) int_pointer;
typedef void * GTY(()) void_pointer;

/* TYPE_POINTER: Function pointer (callback type component) */
typedef int (* GTY(()) compare_func)(const void *, const void *);

/* TYPE_ARRAY: Fixed-size arrays */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[3];

/* TYPE_ARRAY: Multi-dimensional array */
typedef int GTY(()) matrix[3][3];

/* TYPE_STRING: String types */
typedef char * GTY((length("%h.length"))) string_ptr;

struct GTY(()) string_container {
    int length;
    char * GTY((length("%h.length"))) data;
};

/* TYPE_CALLBACK: Struct containing function pointer */
struct GTY(()) callback_container {
    compare_func cmp;
    void *user_data;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int user_id;
    char *user_name;
    struct user_struct *next;
};

/* For TYPE_LANG_STRUCT - defined in C++ file */

#endif /* TEST_TYPES_H */
