/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if available */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Forward declarations for recursive structures */
struct GTY(()) user_struct_t;
struct GTY(()) lang_struct_t;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned long my_ulong;
typedef double my_double;
typedef char my_char;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Plain unions */
union plain_union {
    int i;
    double d;
    void *p;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char *, int);
typedef void (*struct_callback)(struct plain_struct *);

/* TYPE_POINTER: Pointer typedefs */
typedef struct plain_struct *plain_struct_ptr;
typedef int *int_ptr;
typedef void (*callback_ptr)(void);

/* TYPE_ARRAY: Array typedef */
typedef int int_array[10];
typedef struct plain_struct struct_array[5];

/* TYPE_USER_STRUCT: GTY-tagged structs */
struct GTY(()) user_struct_t {
    /* Scalar field */
    int scalar_field;
    
    /* String field */
    const char * GTY((skip)) string_field;
    
    /* Pointer to another GTY struct (recursive) */
    struct user_struct_t * GTY((tag("0"))) next;
    
    /* Pointer to plain struct */
    struct plain_struct *plain_ptr;
    
    /* Array field */
    int int_array_field[20];
    
    /* Array of pointers */
    struct user_struct_t * GTY((length("array_len"))) ptr_array[10];
    int array_len;
    
    /* Callback field */
    simple_callback callback_field;
    
    /* Nested struct */
    struct {
        int nested_scalar;
        double nested_double;
    } GTY((skip)) nested;
    
    /* Union field */
    union {
        int as_int;
        void *as_ptr;
    } GTY((skip)) union_field;
};

/* Another GTY struct with different patterns */
struct GTY(()) complex_user_struct {
    /* Pointer chain */
    struct complex_user_struct * GTY((tag("1"))) prev;
    struct complex_user_struct * GTY((tag("2"))) next;
    
    /* Array of structs */
    struct plain_struct structs[5];
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Function pointer with complex signature */
    int (*compare_fn)(const struct complex_user_struct *,
                      const struct complex_user_struct *);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct_t {
    int lang_specific_field;
    struct lang_struct_t * GTY((skip)) lang_next;
    
    /* Language-specific union */
    union lang_union {
        int lang_int;
        struct user_struct_t *lang_struct_ptr;
    } GTY((skip)) lang_data;
};
#endif

/* Union containing GTY-tagged pointer */
union GTY(()) tagged_union {
    int as_int;
    struct user_struct_t * GTY((tag("3"))) as_user_struct;
    struct complex_user_struct * GTY((tag("4"))) as_complex;
};

/* Struct with all possible field types */
struct GTY(()) comprehensive_struct {
    /* TYPE_SCALAR */
    int count;
    double value;
    
    /* TYPE_STRING */
    const char * GTY((skip)) name;
    char * GTY((skip)) buffer;
    
    /* TYPE_POINTER */
    void *generic_ptr;
    int *int_ptr;
    struct comprehensive_struct * GTY((tag("5"))) self_ptr;
    
    /* TYPE_ARRAY */
    int scores[100];
    struct user_struct_t * GTY((length("item_count"))) items[50];
    int item_count;
    
    /* TYPE_CALLBACK */
    simple_callback on_start;
    complex_callback on_data;
    
    /* TYPE_UNION */
    union {
        int option;
        void *data;
    } GTY((skip)) config;
    
    /* Nested anonymous struct */
    struct {
        int x, y;
    } GTY((skip)) position;
    
    /* Fixed-size string array */
    char path[256];
};

/* Chain of structures for deep traversal */
struct GTY(()) chain_link {
    int id;
    struct chain_link * GTY((tag("6"))) next;
    struct chain_link * GTY((tag("7"))) prev;
    struct chain_link * GTY((tag("8"))) children[5];
    int child_count;
};

/* Tree structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node * GTY((tag("9"))) left;
    struct tree_node * GTY((tag("10"))) right;
    struct tree_node * GTY((tag("11"))) parent;
};

/* Graph node with multiple edges */
struct GTY(()) graph_node {
    int id;
    struct graph_node * GTY((length("edge_count"))) edges[20];
    int edge_count;
    int visited;
};

#endif /* TEST_GENGTYPE_TYPES_H */
