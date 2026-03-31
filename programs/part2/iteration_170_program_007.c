/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ==================== TYPE_SCALAR ==================== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */
typedef char my_char;                   /* Character scalar */

/* ==================== TYPE_STRING ==================== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string */

/* ==================== TYPE_STRUCT ==================== */
struct plain_s {                        /* Untagged struct */
    int a;
    double b;
};

struct another_plain {                  /* Another untagged struct */
    char c;
    long d;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* GTY-tagged structs for garbage collection */
struct GTY(()) user_s {
    struct plain_s *p;                  /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) tree_node {              /* Simulating GCC's tree_node */
    int code;
    struct GTY(()) tree_node *left;     /* Recursive pointer */
    struct GTY(()) tree_node *right;    /* Recursive pointer */
};

struct GTY(()) complex_struct {
    struct GTY(()) user_s *user;        /* Pointer to another GTY struct */
    string_t name;                      /* String field */
    int flags;
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct GTY(()) user_s *us;
    struct plain_s *ps;
    int value;
};

/* ==================== TYPE_POINTER ==================== */
typedef struct plain_s *plain_ptr_t;    /* Pointer typedef */
typedef struct GTY(()) user_s *user_ptr_t; /* GTY pointer typedef */

struct GTY(()) pointer_container {
    void *generic_ptr;                  /* Generic pointer */
    int *int_ptr;                       /* Pointer to scalar */
    struct plain_s **double_ptr;        /* Pointer to pointer */
    struct GTY(()) complex_struct *gty_ptr; /* GTY pointer */
};

/* ==================== TYPE_ARRAY ==================== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct plain_s *ptr_array[5];       /* Array of pointers */
    double multi_dim[3][4];             /* Multi-dimensional array */
};

/* Variable length array simulation */
struct GTY(()) vla_like {
    int length;
    int data[1];                        /* Flexible array member style */
};

/* ==================== TYPE_CALLBACK ==================== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
    void (*simple_cb)(void);           /* Direct function pointer */
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structs using conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct_gen {
    int generator_specific;
};
#else
struct GTY(()) lang_struct_normal {
    int normal_code;
};
#endif

/* Conditional based on LANG specific macro */
#ifdef LANG_is_cplusplus
struct GTY(()) cplusplus_struct {
    void *vtable_ptr;
};
#elif defined(LANG_is_c)
struct GTY(()) c_struct {
    int c_specific;
};
#endif

/* ==================== NESTED/RECURSIVE PATTERNS ==================== */
/* Complex nested type to ensure deep traversal */
struct GTY(()) outer_container {
    struct GTY(()) inner_container {
        struct GTY(()) user_s *user;
        struct GTY(()) inner_container *next; /* Recursive */
        callback_fn cb_array[3];              /* Array of callbacks */
    } *inner;
    
    union GTY(()) choice {
        int as_int;
        struct GTY(()) user_s *as_user;
        callback_fn as_callback;
    } variant;
    
    struct GTY(()) array_container arrays;    /* Embedded struct */
};

/* Linked list structure */
struct GTY(()) linked_node {
    int data;
    struct GTY(()) linked_node *next;         /* Recursive pointer */
    struct GTY(()) linked_node *prev;         /* Recursive pointer */
};

/* Tree structure with multiple pointer types */
struct GTY(()) tree_structure {
    string_t name;
    struct GTY(()) tree_structure *children[8]; /* Array of pointers */
    struct GTY(()) tree_structure *parent;      /* Single pointer */
    union GTY(()) {
        int int_val;
        double double_val;
        string_t str_val;
    } value;
};

/* ==================== UNDEFINED TYPE REFERENCE ==================== */
/* Forward declaration that might be undefined */
struct undefined_struct;                /* TYPE_UNDEFINED when first seen */

struct GTY(()) uses_undefined {
    struct undefined_struct *ptr;       /* Pointer to undefined type */
    int valid;
};

/* Later definition (if processed in same compilation unit) */
struct undefined_struct {
    int defined_now;
};

#endif /* TEST_GENGTYPE_TYPES_H */
