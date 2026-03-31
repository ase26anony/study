/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro */
#ifdef GENERATOR_FILE
#include "gtype-desc.h"
#else
#define GTY(x) 
#endif

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef long my_long;
typedef double my_double;

/* TYPE_STRING: String typedefs */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int);
typedef int (*compare_fn)(const void *, const void *);
typedef void (*cleanup_fn)(void *);

/* TYPE_POINTER: Pointer typedefs */
typedef my_int *int_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    char c;
    struct plain_s *next;
};

/* TYPE_UNION: Union definitions */
union my_u {
    int i;
    double d;
    void *p;
};

union tagged_union {
    int type;
    struct {
        int x, y;
    } coord;
    char *name;
};

/* TYPE_ARRAY: Array type (as part of structs) */
struct with_array {
    int count;
    int values[10];
    char name[32];
};

/* TYPE_USER_STRUCT: GTY-tagged structs for garbage collection */
struct GTY(()) user_s {
    int id;
    string_t name;
    struct user_s *GTY((skip)) next;  /* Pointer to same type */
    struct plain_s *plain_ptr;        /* Pointer to non-GTY struct */
    callback_fn handler;              /* Callback function pointer */
};

/* More complex GTY-tagged struct with nested structures */
struct GTY(()) complex_node {
    int value;
    struct complex_node *GTY((tag("0"))) left;
    struct complex_node *GTY((tag("1"))) right;
    union my_u data;
    int array_field[5];
};

/* GTY-tagged struct containing arrays of pointers */
struct GTY(()) container {
    struct user_s *GTY((length("count"))) items[10];
    int count;
    callback_fn callbacks[3];
};

/* GTY-tagged union */
union GTY(()) gty_union {
    int as_int;
    struct user_s *GTY((tag("1"))) as_user;
    string_t as_string;
};

/* Recursive type pattern */
struct GTY(()) tree_node {
    int type;
    string_t name;
    struct tree_node *GTY((skip)) children;
    struct tree_node *GTY((skip)) next;
};

/* Struct with function pointer field */
struct GTY(()) event_handler {
    int event_id;
    callback_fn handler;
    void *GTY((skip)) user_data;
};

/* Nested struct definitions */
struct GTY(()) outer_struct {
    struct GTY(()) inner {
        int x;
        struct outer_struct *parent;
    } inner_obj;
    int outer_value;
};

/* Array of structs */
struct GTY(()) array_container {
    struct user_s users[5];
    int active[5];
};

/* Conditional compilation for language-specific types */
#ifdef GENERATOR_FILE
/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY(()) lang_struct {
    int lang_specific;
    void *lang_data;
};
#endif

/* Mixed pointer types in GTY struct */
struct GTY(()) mixed_pointers {
    int *scalar_ptr;          /* Pointer to scalar */
    struct plain_s *struct_ptr; /* Pointer to non-GTY struct */
    struct user_s *gty_ptr;   /* Pointer to GTY struct */
    void (*func_ptr)(void);   /* Function pointer */
    const char *const_string; /* String pointer */
};

/* Template for testing parameterized types (simulated) */
#define DEFINE_CONTAINER(TYPE, NAME) \
struct GTY(()) NAME { \
    TYPE *GTY((skip)) items; \
    int count; \
    int capacity; \
}

/* Use the macro to create specific instances */
DEFINE_CONTAINER(struct user_s, user_container);
DEFINE_CONTAINER(int, int_container);

/* Forward declarations that might be TYPE_UNDEFINED initially */
struct GTY(()) forward_declared;
struct not_gty_forward;

/* Now define them */
struct GTY(()) forward_declared {
    int defined_now;
    struct forward_declared *next;
};

struct not_gty_forward {
    int regular_field;
};

#endif /* TEST_GENGTYPE_TYPES_H */
