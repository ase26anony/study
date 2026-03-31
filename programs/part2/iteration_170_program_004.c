/* test_gengtype_types.h - Comprehensive type definitions for gengtype testing */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro */
#ifdef GTY
#undef GTY
#endif
#define GTY(x) x

/* For language-specific structs */
#ifdef GENERATOR_FILE
#define LANG_STRUCT_MARKER
#else
#define LANG_STRUCT_MARKER
#endif

/* ==================== TYPE_SCALAR ==================== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */

/* ==================== TYPE_STRING ==================== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string */

/* ==================== TYPE_CALLBACK ==================== */
typedef void (*callback_fn)(int);       /* Function pointer type */
typedef int (*compare_fn)(const void *, const void *);  /* Another callback */

/* ==================== TYPE_STRUCT ==================== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

struct another_plain {                  /* Another plain struct */
    char c;
    long d;
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union data_union {                      /* Another union */
    long l;
    char bytes[8];
};

/* ==================== TYPE_POINTER ==================== */
typedef struct plain_s *plain_ptr_t;    /* Pointer typedef */
typedef void *generic_ptr_t;            /* Generic pointer */

/* ==================== TYPE_ARRAY ==================== */
/* Arrays will be defined within structs below */

/* ==================== TYPE_USER_STRUCT ==================== */
/* GTY-tagged structs for garbage collection */

struct GTY(()) user_s {                 /* Basic user struct */
    int id;
    string_t name;                      /* String type */
    struct plain_s *plain;              /* Pointer to plain struct */
};

struct GTY(()) tree_node {              /* Tree-like structure */
    int value;
    struct GTY(()) tree_node *left;     /* Recursive pointer */
    struct GTY(()) tree_node *right;    /* Recursive pointer */
    struct GTY(()) tree_node *parent;   /* Another pointer */
};

struct GTY(()) complex_struct {
    my_int scalar_field;                /* Scalar type */
    string_t description;               /* String type */
    
    /* Array types */
    int fixed_array[10];                /* Fixed-size array */
    struct plain_s *ptr_array[5];       /* Array of pointers */
    
    /* Pointer types */
    struct GTY(()) user_s *user_ptr;    /* Pointer to GTY struct */
    union my_u *union_ptr;              /* Pointer to union */
    
    /* Callback type */
    callback_fn handler;                /* Function pointer */
    
    /* Nested struct */
    struct {
        int nested_a;
        double nested_b;
    } nested;
};

struct GTY(()) container {
    /* Multiple array types */
    struct GTY(()) complex_struct *objects[20];  /* Array of pointers to GTY structs */
    int matrix[3][4];                           /* Multi-dimensional array */
    
    /* Union field */
    union data_union data;
    
    /* Pointer to array */
    int (*dynamic_array)[];
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structs (conditional compilation) */

#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_s {
    int lang_field;
    struct GTY(()) user_s *lang_ptr;
};
#endif

/* Another language-specific struct variant */
#if defined(GENERATOR_FILE) || defined(IN_GCC)
struct GTY(()) gcc_specific_s {
    int magic;
    void *gcc_data;
};
#endif

/* ==================== More Complex Patterns ==================== */

/* Struct with array of callbacks */
struct GTY(()) callback_container {
    callback_fn handlers[8];            /* Array of function pointers */
    compare_fn comparators[4];          /* Another array of callbacks */
};

/* Union containing GTY pointers */
union GTY(()) gty_union {
    struct GTY(()) user_s *user;
    struct GTY(()) complex_struct *complex;
    generic_ptr_t generic;
};

/* Struct with self-referential pointer array */
struct GTY(()) graph_node {
    int id;
    struct GTY(()) graph_node **neighbors;  /* Pointer to array of pointers */
    int neighbor_count;
};

/* Mixed struct with all type kinds */
struct GTY(()) kitchen_sink {
    /* Scalar */
    my_int count;
    my_double value;
    
    /* String */
    string_t title;
    mutable_string_t buffer;
    
    /* Struct/Union */
    struct plain_s plain;               /* Embedded plain struct */
    union my_u data_union;              /* Embedded union */
    
    /* Pointers */
    struct GTY(()) user_s *owner;
    struct GTY(()) kitchen_sink *next;  /* Linked list */
    
    /* Arrays */
    callback_fn callbacks[3];
    struct GTY(()) complex_struct *items[5];
    
    /* Special: pointer to array */
    int (*matrix_ptr)[4][4];
    
    /* Language-specific (conditional) */
#ifdef GENERATOR_FILE
    struct GTY(()) lang_specific_s *lang_data;
#endif
};

/* ==================== Undefined Type Forward Declarations ==================== */
struct undefined_struct;                /* Forward declaration */
typedef struct undefined_struct *undefined_ptr_t;

/* This will create TYPE_UNDEFINED when referenced */
struct GTY(()) uses_undefined {
    int valid;
    undefined_ptr_t undefined_field;    /* Pointer to undefined type */
};

#endif /* TEST_GENGTYPE_TYPES_H */
