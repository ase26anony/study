/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro if needed */
#ifdef GTY
/* Already defined */
#else
/* Define a simple GTY macro for testing */
#define GTY(x) 
#endif

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;
typedef long my_long;

/* ==================== TYPE_STRING ==================== */
/* String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* ==================== TYPE_STRUCT ==================== */
/* Plain C structs (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
};

struct another_plain {
    char c;
    int i;
    float f;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* GTY-tagged structs for garbage collection */
struct GTY(()) user_struct {
    int GTY((skip)) id;          /* scalar field with skip marker */
    struct plain_struct *plain;  /* pointer to plain struct */
    string_t name;               /* string type */
};

struct GTY(()) complex_user_struct {
    struct user_struct *next;    /* pointer to another GTY struct */
    struct user_struct *prev;    /* forming a linked list */
    int data;
};

/* Nested GTY struct */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_data;
        struct user_struct *user;
    } inner;
    int outer_data;
};

/* ==================== TYPE_UNION ==================== */
/* Union definitions */
union my_union {
    int i;
    double d;
    void *p;
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    int as_int;
    double as_double;
    struct user_struct *GTY((tag("0"))) as_struct;
};

/* ==================== TYPE_POINTER ==================== */
/* Pointer typedefs */
typedef struct user_struct *user_ptr_t;
typedef int *int_ptr_t;
typedef void (*void_func_ptr)(void);

/* Pointer fields in structs */
struct GTY(()) pointer_container {
    int *int_ptr;               /* pointer to scalar */
    struct user_struct **double_ptr; /* pointer to pointer */
    void *generic_ptr;
};

/* ==================== TYPE_ARRAY ==================== */
/* Array types within structs */
struct GTY(()) array_struct {
    int fixed_array[10];        /* fixed-size array */
    struct user_struct *ptr_array[5]; /* array of pointers */
    char string_array[3][20];   /* 2D array */
};

/* Variable-length array marker */
struct GTY(()) varray_struct {
    int length;
    int data[1];                /* Variable length array */
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types (callbacks) */
typedef int (*compare_fn)(const void *, const void *);
typedef void (*callback_fn)(int, void *);
typedef struct user_struct *(*allocator_fn)(void);

/* Struct with callback field */
struct GTY(()) callback_container {
    compare_fn comparator;
    callback_fn handler;
    void *user_data;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structs using conditional compilation */
#ifdef GENERATOR_FILE
/* This struct should only be processed when GENERATOR_FILE is defined */
struct GTY(()) generator_specific_struct {
    int generator_field;
    struct user_struct *gen_ptr;
};
#endif

/* Another language-specific pattern */
#ifdef LANG_SPECIFIC
struct GTY(()) lang_struct {
    int lang_id;
    void *lang_data;
};
#else
/* Alternative definition */
struct GTY(()) lang_struct {
    int default_id;
    int default_data;
};
#endif

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Recursive structure for deep traversal */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    union {
        int int_val;
        double double_val;
        string_t string_val;
    } data;
};

/* Container with multiple type kinds */
struct GTY(()) type_container {
    /* Scalar */
    my_int scalar_field;
    
    /* String */
    string_t string_field;
    
    /* Struct */
    struct plain_struct plain_field;
    
    /* User struct */
    struct user_struct *user_field;
    
    /* Union */
    union my_union union_field;
    
    /* Pointer */
    void *pointer_field;
    
    /* Array */
    int array_field[5];
    
    /* Callback */
    callback_fn callback_field;
    
    /* Nested container */
    struct type_container *next;
};

/* ==================== UNDEFINED TYPE HANDLING ==================== */
/* Forward declaration that might be undefined */
struct undefined_struct;

/* Struct with pointer to potentially undefined type */
struct GTY(()) uses_undefined {
    struct undefined_struct *undef_ptr;  /* TYPE_UNDEFINED */
    int defined_field;
};

/* ==================== EDGE CASES ==================== */
/* Empty struct */
struct GTY(()) empty_struct {
    /* No fields */
};

/* Struct with only arrays */
struct GTY(()) array_only {
    int matrix[3][3];
    char buffer[256];
};

/* Struct with function pointers array */
struct GTY(()) callback_array {
    callback_fn handlers[10];
    int handler_count;
};

#endif /* TEST_GENGTYPE_TYPES_H */
