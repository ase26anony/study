#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Various scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen($1) + 1"))) string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, void*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Tagged union with discriminant */
struct GTY(()) tagged_union_container {
    int tag;
    union GTY((desc("%0.tag"))) {
        int int_val;
        float float_val;
        char* string_val;
    } value;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct GTY(()) my_struct* GTY(()) ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef void* GTY(()) void_ptr;
typedef int* GTY(()) int_ptr;
typedef struct GTY(()) my_struct* GTY(()) struct_ptr;
typedef union GTY(()) basic_union* GTY(()) union_ptr;
typedef callback_func GTY(()) callback_ptr;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) my_struct {
    /* Scalar members */
    int id;
    scalar_int count;
    color_t color;
    
    /* Pointer members */
    struct_ptr next;
    void_ptr data;
    
    /* Array member */
    int_array values;
    
    /* String member */
    string_ptr name;
    
    /* Callback member */
    callback_func handler;
    
    /* Union member */
    union GTY(()) {
        int x;
        float y;
    } anonymous_union;
    
    /* Bit fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* Padding */
    
    /* Nested struct */
    struct GTY(()) {
        int nested_id;
        float nested_value;
    } nested;
};

/* TYPE_STRUCT with chain_next option for linked list */
struct GTY((chain_next("%h.next"))) linked_node {
    int value;
    struct linked_node* GTY((skip)) next;
};

/* TYPE_STRUCT with length option for variable array */
struct GTY(()) var_array_struct {
    int length;
    int GTY((length("%h.length"))) data[];
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_defined_struct {
    int user_id;
    char* user_name;
};

/* Complex struct with all type dependencies */
struct GTY(()) master_struct {
    /* Direct scalar */
    int master_id;
    
    /* Pointer to another struct */
    struct my_struct* GTY(()) child;
    
    /* Array of pointers */
    struct_ptr GTY(()) children[3];
    
    /* Pointer to union */
    union_ptr union_data;
    
    /* String */
    string_ptr description;
    
    /* Callback */
    callback_ptr notify;
    
    /* Scalar array */
    int_array scores;
    
    /* Nested anonymous struct */
    struct GTY(()) {
        int nested_count;
        float nested_ratio;
    } stats;
    
    /* Reference to undefined type */
    struct opaque_struct* GTY(()) opaque_ref;
    
    /* User struct */
    struct user_defined_struct* GTY(()) user_data;
};

/* Global variable declarations */
extern struct my_struct GTY(()) global_struct;
extern struct master_struct GTY(()) global_master;
extern union basic_union GTY(()) global_union;

#endif /* TEST_TYPES_H */
