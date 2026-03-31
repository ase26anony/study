/* test-gtypes.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t GTY((tag("OPAQUE")));

/* TYPE_SCALAR - Basic scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef bool flag_t;

/* TYPE_STRING */
typedef const char *string_t;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_func)(void *data) GTY((callback));

/* TYPE_USER_STRUCT - User-defined structure with custom handling */
typedef struct user_data {
    int id;
    string_t name;
} user_data_t;
#define USER_DATA_GTY(X)  GTY((user)) X

/* TYPE_STRUCT - Regular structure */
struct regular_struct GTY(()) {
    int scalar_field;          /* TYPE_SCALAR */
    color_t enum_field;        /* TYPE_SCALAR (enum) */
    string_t string_field;     /* TYPE_STRING */
    struct regular_struct *next;  /* TYPE_POINTER */
    callback_func callback;    /* TYPE_CALLBACK */
    user_data_t *user_data;    /* TYPE_POINTER to TYPE_USER_STRUCT */
};

/* TYPE_UNION */
union variant_union GTY(()) {
    int int_val;
    double double_val;
    string_t string_val;
    struct regular_struct *struct_ptr;
};

/* TYPE_ARRAY - Fixed size array */
struct array_container GTY(()) {
    int fixed_array[10];       /* TYPE_ARRAY of TYPE_SCALAR */
    struct regular_struct *ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
};

/* Variable length array with length specifier */
struct vl_array_container GTY(()) {
    size_t count;
    int *variable_array GTY((length("%0.count")));
};

/* TYPE_POINTER - Chain of pointers */
typedef struct regular_struct *struct_ptr_t;
typedef struct_ptr_t *double_ptr_t;

/* Self-referential structure for deep traversal */
struct tree_node GTY(()) {
    int value;
    struct tree_node *left;    /* TYPE_POINTER */
    struct tree_node *right;   /* TYPE_POINTER */
    union variant_union data;  /* TYPE_UNION */
};

/* Container with nested structures */
struct master_container GTY(()) {
    struct regular_struct regular;
    union variant_union variant;
    struct array_container arrays;
    struct tree_node *tree_root;
    opaque_ptr_t opaque;       /* TYPE_POINTER to TYPE_UNDEFINED */
};

#endif /* TEST_GTYPES_H */
