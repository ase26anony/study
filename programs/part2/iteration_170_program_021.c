/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include GCC's gtype header for GTY macro */
#ifdef GENERATOR_FILE
#include "gtype-desc.h"
#else
#define GTY(x) 
#endif

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    char c;
    float f;
};

/* TYPE_USER_STRUCT: GTY-tagged structs */
struct GTY(()) user_s {
    struct plain_s *p;  /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) complex_user {
    struct user_s *next;  /* Recursive pointer */
    string_t name;
    int values[5];  /* Array field */
};

/* TYPE_UNION: Union definitions */
union my_u {
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {
    struct user_s *us;
    struct complex_user *cu;
    my_int id;
};

/* TYPE_POINTER: Various pointer types */
typedef struct user_s *user_ptr_t;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types in structs */
struct GTY(()) array_container {
    int fixed_array[10];
    struct user_s *ptr_array[5];
    char string_array[3][50];
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int, const char*);
typedef int (*compare_fn)(const void*, const void*);

struct GTY(()) callback_container {
    callback_fn handler;
    compare_fn comparator;
    void (*simple_callback)(void);
};

/* TYPE_LANG_STRUCT: Language-specific structs */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific {
    int lang_data;
    void *lang_ptr;
};
#endif

/* More complex nested types for thorough testing */

/* Recursive GTY structure with multiple pointer types */
struct GTY(()) tree_node {
    int node_type;
    string_t name;
    struct tree_node *GTY((skip)) left;   /* Skip in GC tracing */
    struct tree_node *right;               /* Regular pointer */
    struct tree_node **children;           /* Pointer to pointer */
    int child_count;
};

/* Union containing GTY-tagged pointer */
union GTY(()) variant_data {
    struct user_s *user_data;
    struct array_container *array_data;
    callback_fn callback_data;
    string_t string_data;
};

/* Struct with all kinds of fields */
struct GTY(()) comprehensive {
    /* Scalar fields */
    my_int id;
    my_double value;
    
    /* String field */
    string_t description;
    
    /* Struct fields */
    struct plain_s plain;
    
    /* Pointer fields */
    struct user_s *owner;
    void *opaque;
    
    /* Array fields */
    int scores[20];
    struct user_s *users[8];
    
    /* Union field */
    union my_u data;
    
    /* Callback field */
    callback_fn notify;
    
    /* Nested GTY struct */
    struct tree_node *root;
    
    /* Variant union */
    union variant_data variant;
};

/* Forward declaration for mutual recursion */
struct GTY(()) list_node;

/* Mutual recursion example */
struct GTY(()) list_node {
    string_t data;
    struct list_node *next;
    struct GTY(()) list_head *head;
};

struct GTY(()) list_head {
    struct list_node *first;
    struct list_node *last;
    int count;
};

/* Typedef for pointer to callback */
typedef callback_fn* callback_ptr_t;

/* Another scalar typedef */
typedef long long big_int;

#endif /* TEST_GENGTYPE_TYPES_H */
