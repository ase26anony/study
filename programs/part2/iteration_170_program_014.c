/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro if needed */
#ifdef GTY
#undef GTY
#endif
#define GTY(x) x

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

/* TYPE_UNION: Plain unions */
union plain_union {
    int i;
    float f;
    void *p;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char *, int);

/* TYPE_ARRAY: Array type (in context) */
typedef int int_array[10];
typedef char char_matrix[5][20];

/* TYPE_POINTER: Pointer typedefs */
typedef struct plain_struct *plain_struct_ptr;
typedef union plain_union *union_ptr_t;

/* ========== GTY-TAGGED TYPES BELOW ========== */

/* TYPE_USER_STRUCT: GTY-tagged struct */
struct GTY(()) user_struct {
    /* TYPE_POINTER field */
    struct user_struct *next;
    
    /* TYPE_SCALAR field */
    int id;
    
    /* TYPE_STRING field */
    const char *name;
    
    /* TYPE_ARRAY field */
    int values[5];
    
    /* TYPE_POINTER to plain struct */
    struct plain_struct *plain;
    
    /* TYPE_CALLBACK field */
    simple_callback cb;
};

/* Another GTY-tagged struct for complex relationships */
struct GTY(()) complex_struct {
    /* Nested GTY-tagged struct pointer */
    struct user_struct *user;
    
    /* Array of pointers */
    struct GTY((length = "count")) user_struct **items;
    int count;
    
    /* Union containing GTY pointer */
    union {
        struct user_struct *gt_ptr;
        void *raw_ptr;
    } GTY((desc ("%0.raw_ptr ? 1 : 0"))) u;
};

/* TYPE_USER_STRUCT with union */
struct GTY(()) struct_with_union {
    int type;
    union {
        int int_val;
        double double_val;
        struct user_struct *GTY((tag ("1"))) user_ptr;
    } GTY((desc ("%0.type"))) value;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *data;
};
#endif

/* Conditional compilation for different contexts */
#if defined(GENERATOR_FILE) || defined(IN_GCC)
struct GTY(()) conditional_struct {
    int context_specific;
    void *ptr;
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

/* Union with GTY-tagged members */
union GTY(()) tagged_union {
    struct user_struct *GTY((tag ("0"))) as_user;
    struct complex_struct *GTY((tag ("1"))) as_complex;
    int GTY((default)) as_int;
};

/* Array of structs with GTY */
struct GTY(()) array_container {
    struct user_struct GTY((length = "size")) items[10];
    int size;
};

/* Struct with callback in GTY context */
struct GTY(()) callback_container {
    complex_callback handler;
    void *user_data;
};

#endif /* TEST_GENGTYPE_TYPES_H */
