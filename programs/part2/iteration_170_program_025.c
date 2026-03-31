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
typedef _Bool my_bool;                  /* Boolean scalar */

/* ==================== TYPE_STRING ==================== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string type */

/* ==================== TYPE_STRUCT ==================== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

struct another_plain {                  /* Another plain struct */
    struct plain_s *link;
    char name[20];
};

/* ==================== TYPE_USER_STRUCT ==================== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *p;                  /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) complex_user {          /* Another GTY-tagged struct */
    struct user_s *next;               /* Pointer to another GTY struct */
    string_t name;                     /* String field */
    int values[10];                    /* Array field */
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {           /* GTY-tagged union */
    struct user_s *user_ptr;
    struct complex_user *complex_ptr;
    int int_val;
};

/* ==================== TYPE_POINTER ==================== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef void *generic_ptr_t;            /* Generic pointer typedef */

/* ==================== TYPE_ARRAY ==================== */
/* Arrays will be defined within structs below */

/* ==================== TYPE_CALLBACK ==================== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

/* ==================== COMPLEX NESTED STRUCTURES ==================== */

/* Struct with array of pointers */
struct GTY(()) array_container {
    struct user_s *items[20];           /* Array of pointers */
    callback_fn handler;                /* Callback field */
};

/* Struct with nested array */
struct GTY(()) nested_array_struct {
    int matrix[5][5];                   /* 2D array */
    char strings[10][50];               /* Array of strings */
};

/* Recursive struct definition */
struct GTY(()) recursive_node {
    int value;
    struct recursive_node *left;        /* Recursive pointer */
    struct recursive_node *right;       /* Another recursive pointer */
    union my_u data;                    /* Union field */
};

/* Struct with all type kinds */
struct GTY(()) kitchen_sink {
    /* Scalar fields */
    my_int id;
    my_double weight;
    
    /* String field */
    string_t description;
    
    /* Pointer fields */
    struct user_s *owner;
    struct complex_user *complex;
    
    /* Array field */
    int scores[100];
    
    /* Union field */
    union tagged_union variant;
    
    /* Callback field */
    callback_fn notify;
    
    /* Nested struct */
    struct plain_s base;
};

/* ==================== LANGUAGE-SPECIFIC STRUCT ==================== */
/* TYPE_LANG_STRUCT requires conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_s *gen_ptr;
};
#endif

/* For non-generator contexts */
#ifndef GENERATOR_FILE
struct GTY(()) regular_struct {
    int regular_field;
};
#endif

/* ==================== MORE COMPLEX PATTERNS ==================== */

/* Linked list of mixed types */
struct GTY(()) list_node {
    enum { INT_NODE, USER_NODE, STRING_NODE } node_type;
    union {
        my_int int_value;
        struct user_s *user_value;
        string_t string_value;
    } data;
    struct list_node *next;
};

/* Tree structure with callbacks */
struct GTY(()) tree_node {
    int key;
    string_t data;
    struct tree_node *children[4];      /* Array of pointers */
    compare_fn comparator;              /* Callback for comparison */
};

/* Container with multiple array types */
struct GTY(()) multi_array {
    int int_array[50];                  /* Simple array */
    struct user_s *ptr_array[30];       /* Array of pointers */
    callback_fn func_array[10];         /* Array of callbacks */
    char char_matrix[20][40];           /* 2D character array */
};

#endif /* TEST_GENGTYPE_TYPES_H */
