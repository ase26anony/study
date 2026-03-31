/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h if available in GCC build context */
#ifdef HAVE_GTYPE_DESC_H
#include "gtype-desc.h"
#endif

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned long my_ulong;
typedef double my_double;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Union definitions */
union plain_union {
    int i;
    void *p;
    double d;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int, void*);
typedef int (*compare_fn)(const void*, const void*);

/* TYPE_USER_STRUCT: GTY-tagged structs for garbage collection */
struct GTY(()) user_struct {
    /* TYPE_POINTER: Pointer field */
    struct user_struct *next;
    
    /* TYPE_SCALAR field */
    int id;
    
    /* TYPE_STRING field */
    const char *name;
    
    /* TYPE_ARRAY: Fixed-size array */
    int values[10];
    
    /* TYPE_CALLBACK field */
    callback_fn handler;
};

/* Another GTY-tagged struct with nested types */
struct GTY(()) complex_struct {
    /* Pointer to another GTY-tagged struct */
    struct user_struct *user;
    
    /* Pointer to plain struct */
    struct plain_struct *plain;
    
    /* Array of pointers */
    void * GTY((length("count"))) *items;
    int count;
    
    /* Union containing GTY pointer */
    union {
        struct user_struct * GTY((tag("0"))) uptr;
        int ival;
    } data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    void *generator_data;
};
#endif

/* Conditional compilation for different contexts */
#if defined(GCC) || defined(GENERATOR_FILE)
struct GTY(()) conditional_struct {
    int context_specific;
    #ifdef GENERATOR_FILE
    void *generator_ptr;
    #endif
};
#endif

/* Recursive type pattern */
struct GTY(()) tree_node;
struct GTY(()) tree_node {
    int node_type;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Union with GTY markers */
union GTY((desc ("%0.type"))) tagged_union {
    int type;
    struct GTY((tag ("1"))) user_struct *us;
    struct GTY((tag ("2"))) complex_struct *cs;
};

/* Struct containing array of structs */
struct GTY(()) array_container {
    /* Variable-length array of GTY structs */
    struct user_struct GTY((length("num_users"))) *users;
    int num_users;
    
    /* Fixed array of pointers */
    void *pointers[5];
};

/* Callback type used in GTY struct */
typedef void (*traverse_fn)(struct tree_node *);
struct GTY(()) traversable {
    struct tree_node *root;
    traverse_fn traverse;
};

/* More pointer types */
typedef struct user_struct *user_ptr_t;
typedef void (*void_callback)(void);

/* Nested struct definitions */
struct outer {
    struct GTY(()) inner {
        int inner_field;
        struct outer *outer_ptr;
    } *inner_ptr;
    
    union {
        int x;
        struct inner *in;
    } data;
};

/* Additional scalar types for coverage */
typedef short my_short;
typedef long long my_longlong;
typedef unsigned char byte;

#endif /* TEST_GENGTYPE_TYPES_H */
