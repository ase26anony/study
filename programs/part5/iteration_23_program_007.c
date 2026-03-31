#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"

/* TYPE_UNDEFINED - forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR - various scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;

/* TYPE_STRUCT - regular struct with various members */
struct GTY(()) base_struct {
    int GTY((skip)) x;
    double y;
    color_t color;
};

/* Nested struct with bitfields */
struct GTY(()) complex_struct {
    struct base_struct base;
    unsigned int flags : 4;
    unsigned int count : 28;
    
    /* Anonymous struct */
    struct GTY(()) {
        float f;
        char c;
    } anonymous;
    
    /* Chain pointers for GTY options */
    struct complex_struct *GTY((chain_next ("%h.next"))) next;
    struct complex_struct *GTY((chain_prev ("%h.prev"))) prev;
};

/* TYPE_USER_STRUCT - user-defined struct */
struct GTY((user)) user_struct {
    int id;
    char *GTY((tag("0"))) name;
};

/* TYPE_UNION - various union types */
union GTY(()) simple_union {
    int i;
    double d;
    void *p;
};

/* Tagged union with desc */
union GTY((desc ("%1.type"))) tagged_union {
    int type;
    struct {
        int type;
        int value;
    } GTY((tag ("0"))) int_val;
    struct {
        int type;
        double value;
    } GTY((tag ("1"))) double_val;
};

/* TYPE_POINTER - various pointer types */
typedef struct base_struct *GTY(()) base_ptr;
typedef union simple_union *GTY(()) union_ptr;
typedef void (*GTY(()) func_ptr)(int, double);
typedef int *GTY(()) int_ptr;
typedef void *GTY(()) void_ptr;

/* TYPE_ARRAY - various array types */
typedef int GTY(()) int_array[10];
typedef struct base_struct GTY(()) struct_array[5];
typedef int GTY(()) multi_dim_array[3][4][5];

/* TYPE_STRING - string types */
typedef char *GTY((length ("strlen(%h)"))) string_ptr;
typedef const char *GTY(()) const_string;

/* TYPE_CALLBACK - callback function pointer in struct */
struct GTY(()) callback_container {
    int (*GTY((callback)) compare)(const void *, const void *);
    void (*GTY((callback)) handler)(int, void *);
};

/* Struct containing all types */
struct GTY(()) master_struct {
    /* SCALAR */
    int scalar_int;
    enum color scalar_enum;
    
    /* STRUCT */
    struct base_struct nested_struct;
    
    /* UNION */
    union tagged_union data;
    
    /* POINTER */
    struct complex_struct *struct_ptr;
    int *int_ptr;
    void *void_ptr;
    
    /* ARRAY */
    int int_arr[10];
    struct base_struct struct_arr[3];
    
    /* STRING */
    char *GTY((length ("strlen(%h.str)"))) str;
    const char *const_str;
    
    /* CALLBACK */
    int (*sorter)(int *, size_t);
    
    /* USER STRUCT */
    struct user_struct user;
    
    /* Chain for traversal */
    struct master_struct *GTY((chain_next)) next;
};

/* Global variable declarations */
extern struct master_struct GTY(()) global_master;
extern struct complex_struct *GTY(()) global_list;
extern union simple_union GTY(()) global_union;

#endif /* TEST_TYPES_H */
