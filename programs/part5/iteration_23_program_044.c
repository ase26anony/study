/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"  /* For GTY macro */

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Various scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;

/* TYPE_STRING: String type with explicit GTY marker */
typedef char* GTY((length("strlen($1)"))) string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, void*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int int_val;
    double double_val;
    void* GTY((tag("0"))) ptr_val;
};

/* Tagged union with desc option */
union GTY((desc("$1.type"))) tagged_union {
    struct {
        int type;
        union {
            int i;
            double d;
            char* GTY((length("strlen($1)"))) s;
        } GTY((desc("$1.type"))) data;
    } GTY(()) header;
    long long raw_data;
};

/* TYPE_STRUCT: Basic struct */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
    enum color color;
};

/* Struct with nested anonymous struct */
struct GTY(()) nested_struct {
    struct {
        int x;
        int y;
    } GTY(()) point;
    struct {
        int width;
        int height;
    } GTY(()) size;
};

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Struct with chain_next option for linked list */
struct GTY((chain_next("$1.next"))) linked_node {
    int data;
    struct linked_node* GTY((skip)) next;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_data;
    void* user_pointer;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr;
typedef union basic_union* GTY(()) union_ptr;
typedef int* GTY(()) int_ptr;
typedef void (*GTY(()) func_ptr)(void);
typedef void (**GTY(())) func_ptr_ptr;

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[3][3];

/* Complex struct with all type dependencies */
struct GTY(()) master_struct {
    /* Scalar types */
    int scalar_int;
    double scalar_double;
    enum color color_enum;
    
    /* String */
    char* GTY((length("strlen($1.master_string)"))) master_string;
    
    /* Pointer types */
    struct basic_struct* GTY(()) struct_pointer;
    union tagged_union* GTY(()) union_pointer;
    void* GTY((skip)) opaque_pointer;
    int* GTY(()) int_pointer;
    
    /* Array types */
    int int_array[20];
    struct basic_struct struct_array[3];
    union basic_union union_array[2][2];
    
    /* Nested struct */
    struct nested_struct nested;
    
    /* Bitfield struct */
    struct bitfield_struct flags;
    
    /* Callback */
    callback_func callback;
    
    /* Chain pointer */
    struct linked_node* GTY((chain_next("$1.node_next"))) node_next;
    
    /* Reference to undefined type */
    struct opaque_struct* GTY(()) opaque_ref;
    
    /* User struct */
    struct user_struct user;
    
    /* Anonymous union inside struct */
    union {
        int anon_int;
        double anon_double;
    } GTY(()) anon_union;
};

/* Global variable declarations */
extern struct master_struct GTY(()) global_master;
extern struct basic_struct GTY(()) global_struct_array[];
extern union tagged_union GTY(()) global_union;

#endif /* TEST_TYPES_H */
