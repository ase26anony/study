/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

#ifdef GENERATOR_FILE
/* This macro indicates we're being processed by gengtype */
#endif

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */
typedef char my_char;                   /* Character scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string pointer */

/* ========== TYPE_STRUCT ========== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

struct another_plain {                  /* Another plain struct */
    char c;
    struct plain_s *next;
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    int id;
    string_t name;
    struct user_s *GTY((skip)) next;    /* Pointer with skip attribute */
    struct plain_s *plain_ptr;          /* Pointer to plain struct */
};

struct GTY(()) complex_user {          /* Another GTY-tagged struct */
    my_int value;
    struct user_s *GTY((tag("0"))) child;  /* Tagged pointer */
    void *GTY((skip)) opaque;          /* Opaque pointer */
};

/* ========== TYPE_UNION ========== */
union my_u {                            /* Plain union */
    int i;
    double d;
    void *p;
};

union GTY(()) tagged_union {           /* GTY-tagged union */
    struct user_s *GTY((tag("0"))) usr;
    struct complex_user *GTY((tag("1"))) complex;
    int scalar_val;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef int *int_ptr_t;                 /* Another pointer typedef */

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char string_array[3][20];           /* 2D array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;              /* Another callback */
    void (*simple_cb)(void);           /* Direct function pointer */
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef GENERATOR_FILE
/* Language-specific struct - only seen by gengtype */
struct GTY(()) lang_specific {
    int lang_data;
    struct user_s *lang_ptr;
};
#endif

/* ========== NESTED/RECURSIVE PATTERNS ========== */

/* Recursive GTY struct */
struct GTY(()) tree_node {
    int type;
    string_t name;
    struct tree_node *GTY((tag("0"))) left;
    struct tree_node *GTY((tag("1"))) right;
    union {
        int int_val;
        double float_val;
        string_t str_val;
    } GTY((tag("2"))) value;
};

/* Struct with array of pointers to different types */
struct GTY(()) heterogeneous_container {
    struct user_s *users[4];
    struct complex_user *complex_users[2];
    struct tree_node *nodes[8];
    void *void_ptrs[3];
};

/* Union containing GTY-tagged pointer */
union GTY(()) mixed_union {
    struct user_s *GTY((tag("0"))) user;
    struct complex_user *GTY((tag("1"))) complex;
    callback_fn callback;
    int array[4];
};

/* Struct with callback that takes GTY types as parameters */
typedef void (*process_fn)(struct user_s *, struct complex_user *);

struct GTY(()) processor {
    process_fn process;
    struct user_s *GTY((tag("0"))) data;
};

/* ========== UNDEFINED TYPE REFERENCES ========== */
/* Forward declaration that creates TYPE_UNDEFINED initially */
struct GTY(()) forward_declared;

struct GTY(()) uses_forward {
    int id;
    struct forward_declared *GTY((tag("0"))) fwd_ptr;  /* Initially undefined */
};

/* Later definition */
struct GTY(()) forward_declared {
    int value;
    struct uses_forward *GTY((tag("0"))) back_ptr;
};

/* ========== COMPLEX NESTING ========== */
struct GTY(()) outer_container {
    struct {
        int x;
        struct user_s *GTY((tag("0"))) user;
    } inner;
    
    union {
        int i;
        struct {
            double a;
            double b;
        } point;
    } data;
    
    struct tree_node *GTY((tag("1"))) root;
    callback_fn callbacks[3];
};

/* ========== EDGE CASES ========== */

/* Empty struct */
struct GTY(()) empty_struct {
    /* No members */
};

/* Struct with only scalars */
struct GTY(()) scalar_only {
    int a;
    double b;
    char c;
    unsigned long d;
};

/* Struct with only pointers */
struct GTY(()) pointer_only {
    void *ptr1;
    int *ptr2;
    struct user_s *GTY((tag("0"))) ptr3;
    const char *ptr4;
};

#endif /* TEST_GENGTYPE_TYPES_H */
