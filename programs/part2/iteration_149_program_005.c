/* test-coverage.h - Header to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations to create TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* Never defined - TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned int my_unsigned_scalar;

/* Enum type for scalar coverage */
enum gty_test_enum {
    GTY_TEST_ZERO,
    GTY_TEST_ONE,
    GTY_TEST_TWO
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
    int a;                     /* Scalar field */
    tree GTY((skip)) node;     /* Pointer with skip attribute */
    my_scalar count;           /* Typedef scalar */
    enum gty_test_enum status; /* Enum scalar */
};

/* TYPE_USER_STRUCT: User-defined type handling */
struct GTY((user)) user_defined {
    void *private_data;        /* Opaque pointer */
    int user_tag;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
    int i;
    const char *s;
    tree t;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
    tree value;
    struct tree_list *GTY((skip)) next;  /* Self-referential pointer with skip */
    struct plain_struct *nested;         /* Pointer to another GTY struct */
};

/* More complex pointer structure */
struct GTY(()) pointer_network {
    struct tree_list *GTY((chain_next("next"))) head;
    struct user_defined *user_ptr;       /* Pointer to user struct */
    struct opaque_type *opaque_ptr;      /* Pointer to undefined type */
    void *GTY((skip)) raw_ptr;           /* Skip-marked void pointer */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
    int fixed[5];                        /* Fixed-size array */
    tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
    int dynamic_count;
    const char * GTY((length("str_len"))) string_array[10]; /* String array */
    size_t str_len;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimicking tree structure with lang-specific data */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
    tree type;
    enum tree_code code;
    union value_union GTY((desc("code"))) value;
};

/* Another lang struct pattern */
struct GTY((tag("TS_BLOCK"))) lang_block {
    tree vars;
    tree subblocks;
    tree supercontext;
    tree chain;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
    const char * GTY((tag("STRING"))) name;  /* String field */
    const char * GTY((tag("STRING"))) filename;
    int id;
};

/* Container with multiple string types */
struct GTY(()) string_container {
    const char * GTY((tag("STRING"))) static_string;
    char * GTY((tag("STRING"))) dynamic_string;
    const char * const * GTY((length("string_count"))) string_array;
    int string_count;
};

/* TYPE_CALLBACK: Structure with callback function pointer */
struct GTY(()) tree_walker {
    walk_fn GTY((skip)) pre_order_callback;
    walk_fn GTY((skip)) post_order_callback;
    transform_fn GTY((skip)) transform_callback;
    void * GTY((skip)) user_data;
};

/* Complex structure combining multiple types */
struct GTY(()) comprehensive_test {
    /* Scalar fields */
    my_scalar scalar_field;
    my_long_scalar long_field;
    my_unsigned_scalar unsigned_field;
    
    /* Struct fields */
    struct plain_struct plain;
    struct user_defined *user_struct;
    
    /* Union field */
    union value_union data;
    
    /* Pointer fields */
    struct tree_list *list;
    struct pointer_network *network;
    
    /* Array fields */
    struct array_container arrays;
    
    /* Lang struct fields */
    struct lang_specific_tree_node *lang_node;
    struct lang_block *block;
    
    /* String fields */
    struct named_object named;
    struct string_container strings;
    
    /* Callback fields */
    struct tree_walker walker;
    
    /* Reference to undefined type */
    struct opaque_type *opaque_ref;
    
    /* Nested anonymous union for additional coverage */
    union {
        int as_int;
        tree as_tree;
        const char * GTY((tag("STRING"))) as_string;
    } GTY((desc("data.i != 0"))) anonymous_union;
};

/* Root structure for gengtype to process */
extern struct GTY(()) root_struct {
    struct comprehensive_test *main_test;
    struct tree_list *global_list;
    struct named_object *global_names;
    struct tree_walker global_walker;
} test_root;

#endif /* TEST_COVERAGE_H */
