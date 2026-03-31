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
typedef char *mutable_string_t;         /* Mutable string type */

/* ==================== TYPE_STRUCT ==================== */
struct plain_s {                        /* Untagged struct */
    int a;
    double b;
};

struct another_plain {                  /* Another untagged struct */
    struct plain_s *link;
    char name[32];
};

/* ==================== TYPE_USER_STRUCT ==================== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *plain_ptr;          /* Pointer to plain struct */
    int counter;
};

struct GTY(()) complex_user {          /* Another GTY-tagged struct */
    struct user_s *next;               /* Pointer to another GTY struct */
    struct user_s *prev;               /* Another pointer */
    my_int id;
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Simple union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {           /* GTY-tagged union */
    struct user_s *user_ptr;
    struct complex_user *complex_ptr;
    int tag;
};

/* ==================== TYPE_POINTER ==================== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef struct complex_user **double_ptr_t; /* Double pointer typedef */

/* ==================== TYPE_ARRAY ==================== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char string_array[3][32];           /* 2D array */
};

/* ==================== TYPE_CALLBACK ==================== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
    void (*void_callback)(void);
};

/* ==================== TYPE_LANG_STRUCT ==================== */
#ifdef GENERATOR_FILE
/* This struct should only be processed in generator context */
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_s *link;
};
#endif

/* Conditional for different language front-ends */
#ifdef LANG_HOOKS
struct GTY(()) lang_hook_struct {
    int hook_id;
    void *hook_data;
};
#endif

/* ==================== COMPLEX NESTED STRUCTURES ==================== */
/* Recursive structure */
struct GTY(()) tree_node {
    int node_type;
    struct tree_node *GTY((skip)) left;  /* Skip this for GC */
    struct tree_node *right;             /* Regular pointer */
    struct tree_node **children;         /* Pointer to pointer array */
    int child_count;
};

/* Structure with multiple array types */
struct GTY(()) multi_array {
    int matrix[4][4];                   /* 2D array */
    struct user_s *object_grid[3][3];   /* 2D array of pointers */
    char *string_list[8];               /* Array of string pointers */
};

/* Union containing GTY-tagged pointer */
union GTY(()) variant_data {
    struct user_s *as_user;
    struct complex_user *as_complex;
    struct array_container *as_array;
    callback_fn as_callback;
};

/* Structure with callback in union */
struct GTY(()) callback_union_container {
    union {
        callback_fn func_ptr;
        struct user_s *data_ptr;
    } GTY((tag("0"))) u;
    int tag_value;
};

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations create TYPE_UNDEFINED initially */
struct GTY(()) forward_declared;        /* Forward declaration */

struct GTY(()) uses_forward {
    struct forward_declared *fd_ptr;    /* Pointer to forward-declared type */
    int status;
};

/* Later definition of forward-declared type */
struct GTY(()) forward_declared {
    struct uses_forward *back_ptr;      /* Mutual reference */
    char data[64];
};

/* ==================== EDGE CASES ==================== */
/* Empty struct */
struct GTY(()) empty_struct {};

/* Struct with only scalar fields */
struct GTY(()) scalar_only {
    int a, b, c;
    double x, y, z;
    char flag;
};

/* Struct with mixed GTY and non-GTY fields */
struct mixed_fields {
    int regular_field;                  /* Non-GTY field */
    struct GTY(()) user_s *gty_field;   /* GTY field */
    void *regular_pointer;              /* Another non-GTY field */
};

#endif /* TEST_GENGTYPE_TYPES_H */
