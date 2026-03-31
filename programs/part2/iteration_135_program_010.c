/* test-gty.h - Comprehensive GTY test types for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_SCALAR - basic scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef bool flag_t;

/* TYPE_STRING */
typedef const char *string_t;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_USER_STRUCT - user-defined structure with special handling */
typedef struct user_data {
    int id;
    string_t name;
} user_data_t;

/* TYPE_STRUCT - regular GTY-tagged structure */
struct GTY(()) linked_node {
    int value;
    struct linked_node *GTY((skip)) next;  /* TYPE_POINTER */
    user_data_t *GTY((tag("USER_DATA"))) user;  /* Another TYPE_POINTER */
};

/* TYPE_UNION */
union GTY(()) variant_data {
    int int_val;
    double double_val;
    string_t string_val;
    struct linked_node *GTY((skip)) node_ptr;
};

/* TYPE_ARRAY - within a struct */
struct GTY(()) array_container {
    int fixed_array[10];  /* Fixed-size array */
    int *GTY((length("len"))) dyn_array;  /* Variable-length array */
    size_t len;
    color_t colors[5];  /* Array of scalar enum */
};

/* TYPE_POINTER - standalone pointer typedef */
typedef struct linked_node *node_ptr_t;
typedef union variant_data *variant_ptr_t;

/* Complex nested structure for thorough testing */
struct GTY(()) complex_struct {
    /* TYPE_STRUCT embedding */
    struct array_container container;
    
    /* TYPE_UNION field */
    union variant_data variant;
    
    /* TYPE_POINTER fields */
    node_ptr_t first_node;
    variant_ptr_t variant_ptr;
    
    /* TYPE_ARRAY of pointers */
    node_ptr_t GTY((length("node_count"))) node_array[5];
    int node_count;
    
    /* TYPE_SCALAR fields */
    color_t color;
    flag_t active;
    size_t size;
    
    /* TYPE_STRING field */
    string_t description;
    
    /* TYPE_CALLBACK field */
    callback_func notify;
    
    /* Reference to TYPE_UNDEFINED */
    opaque_ptr_t opaque;
};

/* TYPE_LANG_STRUCT will be in separate C++ file */

#endif /* TEST_GTY_H */
