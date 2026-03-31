/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned long my_ulong;
typedef double my_double;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_s {
    int a;
    double b;
};

/* TYPE_UNION: Union definitions */
union my_u {
    int i;
    void *p;
    double d;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_USER_STRUCT: GTY-tagged structs for garbage collection */
struct GTY(()) user_s {
    /* TYPE_POINTER: Pointer field */
    struct plain_s *plain_ptr;
    
    /* TYPE_ARRAY: Array field */
    int arr[10];
    
    /* Another TYPE_POINTER */
    struct user_s *next;
    
    /* TYPE_CALLBACK used in struct */
    callback_fn handler;
    
    /* TYPE_STRING field */
    const char *name;
};

/* More complex GTY-tagged struct with nested structures */
struct GTY(()) complex_s {
    /* Self-referential pointer (recursive type) */
    struct complex_s *GTY((skip)) self;
    
    /* Pointer to another GTY-tagged struct */
    struct user_s *user;
    
    /* Array of pointers */
    struct plain_s *GTY((length("count"))) *ptr_array;
    int count;
    
    /* Union containing GTY-tagged pointer */
    union {
        struct user_s *GTY((tag("0"))) uptr;
        int value;
    } data;
    
    /* Two-dimensional array */
    double matrix[3][3];
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_s {
    int generator_only_field;
    struct user_s *linked;
};
#endif

/* Another GTY-tagged union */
union GTY(()) tagged_u {
    struct user_s *GTY((tag("0"))) us;
    struct complex_s *GTY((tag("1"))) cs;
    int GTY((tag("2"))) ival;
};

/* Struct with callback pointer */
struct GTY(()) with_callback {
    callback_fn start;
    callback_fn finish;
    int state;
};

/* Chain of structures for deep traversal */
struct GTY(()) node_s {
    int value;
    struct node_s *GTY((skip)) left;
    struct node_s *GTY((skip)) right;
    struct node_s *parent;
};

/* Mixed struct with various field types */
struct GTY(()) mixed_s {
    /* Scalar fields */
    my_int id;
    my_double weight;
    
    /* String field */
    string_t description;
    
    /* Pointer to plain struct */
    struct plain_s *plain;
    
    /* Array of strings */
    const char *names[5];
    
    /* Function pointer */
    compare_fn comparator;
    
    /* Nested union */
    union {
        struct user_s *u;
        struct complex_s *c;
    } container;
    
    /* Flexible array member (zero-length array) */
    int scores[];
};

/* Forward declaration that will be TYPE_UNDEFINED initially */
struct GTY(()) forward_declared_s;

/* Complete definition later */
struct GTY(()) forward_declared_s {
    int x;
    struct forward_declared_s *next;
};

/* Template-like structure using macros */
#define DECLARE_GTY_STRUCT(name, type) \
    struct GTY(()) name##_s { \
        type value; \
        struct name##_s *next; \
    }

DECLARE_GTY_STRUCT(int_node, int);
DECLARE_GTY_STRUCT(double_node, double);

#endif /* TEST_GENGTYPE_TYPES_H */
