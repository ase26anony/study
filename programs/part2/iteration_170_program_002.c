/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h if needed for GTY macro */
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

/* TYPE_USER_STRUCT: GTY-tagged structs */
struct GTY(()) user_struct {
    int id;
    struct plain_struct *plain_ptr;  /* TYPE_POINTER */
    struct user_struct *next;        /* Recursive pointer */
};

/* TYPE_UNION: Union definitions */
union my_union {
    int i;
    void *p;
    double d;
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
    int fixed_array[10];             /* Fixed-size array */
    int *dynamic_array GTY((length("len"))); /* Variable-length array */
    int len;
};

/* TYPE_POINTER: Various pointer types */
typedef struct user_struct *user_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct user_struct *, string_t);

/* TYPE_LANG_STRUCT: Language-specific structs */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *data;
};
#endif

/* More complex nested structures to ensure deep traversal */

/* Union containing GTY-tagged pointer */
union GTY(()) tagged_union {
    struct user_struct * GTY((tag("0"))) user_ptr;
    struct array_container * GTY((tag("1"))) array_ptr;
    int type_tag;
};

/* Struct with callback field */
struct GTY(()) callback_container {
    simple_callback cb1;
    complex_callback cb2;
    void (*inline_cb)(struct callback_container *self);
};

/* Struct with array of pointers */
struct GTY(()) pointer_array {
    struct user_struct * GTY((length("count"))) *items;
    int count;
};

/* Nested struct definitions */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        struct outer_struct *parent;
    } inner;
    
    union my_union data;
    struct callback_container callbacks;
};

/* Linked list structure for recursive patterns */
struct GTY(()) linked_node {
    string_t name;
    struct linked_node * GTY((skip)) *children;  /* Skip in GC, but still a pointer */
    int child_count;
    struct linked_node *next;
};

/* Mixed struct with all kinds of fields */
struct GTY(()) kitchen_sink {
    /* SCALAR */
    my_int scalar1;
    my_double scalar2;
    
    /* STRING */
    string_t str_field;
    char *mutable_str;
    
    /* STRUCT */
    struct plain_struct plain;
    
    /* POINTER */
    struct user_struct *user_ptr;
    void *generic_ptr;
    
    /* ARRAY */
    int small_array[5];
    struct user_struct * GTY((length("dynamic_len"))) *ptr_array;
    int dynamic_len;
    
    /* UNION */
    union my_union u;
    
    /* CALLBACK */
    simple_callback cb;
    
    /* Nested GTY struct */
    struct outer_struct nested;
};

/* Undefined type forward declaration (will be TYPE_UNDEFINED initially) */
struct undefined_struct;

/* Later definition to resolve undefined type */
struct GTY(()) undefined_struct {
    int defined_now;
    struct undefined_struct *self_ptr;
};

#endif /* TEST_GENGTYPE_TYPES_H */
