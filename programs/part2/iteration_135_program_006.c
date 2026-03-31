/* test-gtypes.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

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
typedef void (*callback_fn)(void *data);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_USER_STRUCT - with user-defined options */
struct user_base {
    int id;
    string_t name;
};

typedef struct user_base * GTY((user)) user_struct_t;

/* TYPE_STRUCT - regular struct */
struct GTY(()) linked_list {
    int value;
    struct linked_list * GTY((skip(""))) next;
    color_t color;
};

/* TYPE_UNION */
union GTY(()) variant_data {
    int int_val;
    double double_val;
    string_t string_val;
    struct linked_list *list_ptr;
};

/* TYPE_ARRAY - within a struct */
struct GTY(()) array_container {
    int fixed_array[10];
    int * GTY((length("len"))) variable_array;
    size_t len;
};

/* TYPE_POINTER - pointer types in a struct */
struct GTY(()) pointer_holder {
    struct linked_list *list;
    union variant_data *variant;
    callback_fn callback;
    opaque_ptr_t opaque;
};

/* Self-referential structure for deep processing */
struct GTY(()) tree_node {
    int id;
    string_t label;
    struct tree_node * GTY((skip(""))) left;
    struct tree_node * GTY((skip(""))) right;
    union variant_data data;
};

/* Complex nested structure */
struct GTY(()) complex_type {
    struct tree_node *root;
    struct array_container arrays[5];
    struct pointer_holder *holders;
    callback_fn handlers[3];
    flag_t flags[8];
};

#endif /* TEST_GTYPES_H */
