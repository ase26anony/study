/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro */
#ifdef GTY
#undef GTY
#endif
#define GTY(x) x

/* For TYPE_SCALAR */
typedef int my_int;
typedef unsigned long my_ulong;
typedef double my_double;

/* For TYPE_STRING */
typedef const char *string_t;
typedef char *mutable_string_t;

/* For TYPE_STRUCT (plain, untagged) */
struct plain_struct {
    int field1;
    double field2;
};

/* For TYPE_USER_STRUCT (GTY-tagged) */
struct GTY(()) user_struct {
    struct plain_struct *plain_ptr;  /* TYPE_POINTER */
    int scalar_field;                /* TYPE_SCALAR */
};

/* Another GTY-tagged struct for complex relationships */
struct GTY(()) complex_struct {
    struct user_struct *user_ptr;    /* TYPE_POINTER to GTY struct */
    struct complex_struct *self_ptr; /* Recursive pointer */
    int data;
};

/* For TYPE_UNION */
union my_union {
    int int_val;
    void *void_ptr;
    struct user_struct *user_ptr;
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    int tag;
    struct user_struct *user;
    struct complex_struct *complex;
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];             /* Fixed-size array */
    struct user_struct *ptr_array[5]; /* Array of pointers */
    int multi_dim[3][4];             /* Multi-dimensional */
};

/* For TYPE_CALLBACK */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct user_struct *, int);

/* GTY struct with callback field */
struct GTY(()) callback_container {
    simple_callback cb1;
    complex_callback cb2;
    void (*inline_cb)(void);         /* Inline function pointer */
};

/* For TYPE_LANG_STRUCT - using generator file conditionals */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *gen_ptr;
};
#endif

/* Conditional for non-generator context */
#ifndef GENERATOR_FILE
struct GTY(()) non_generator_struct {
    int runtime_field;
};
#endif

/* Nested type example */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_data;
        struct outer_struct *parent; /* Pointer to containing struct */
    } *inner;
    
    union GTY(()) {
        int as_int;
        struct inner_struct *as_inner;
    } variant;
    
    /* Array of function pointers */
    simple_callback callbacks[8];
};

/* String-specific struct */
struct GTY(()) string_container {
    const char *constant_string;     /* TYPE_STRING */
    char *mutable_string;            /* Also TYPE_STRING */
    string_t typedef_string;         /* TYPE_STRING via typedef */
};

/* Pointer chain for deep traversal */
struct GTY(()) pointer_chain {
    struct pointer_chain *next;      /* Linked list */
    void *data;                      /* Generic pointer */
    struct user_struct *user_data;
};

/* Mixed container with all types */
struct GTY(()) mega_container {
    /* Scalars */
    my_int typedef_scalar;
    int direct_scalar;
    
    /* Strings */
    string_t typedef_string;
    const char *direct_string;
    
    /* Structs */
    struct plain_struct plain;
    struct user_struct *user_ptr;
    
    /* Unions */
    union my_union any_type;
    
    /* Arrays */
    int numbers[20];
    struct user_struct *objects[10];
    
    /* Callbacks */
    simple_callback handler;
    
    /* Nested */
    struct outer_struct nested;
};

/* Undefined type forward declaration (will be TYPE_UNDEFINED initially) */
struct undefined_struct;

/* Later definition to complete it */
struct GTY(()) undefined_struct {
    int defined_now;
    struct undefined_struct *self;  /* Self-reference */
};

/* Additional pointer types */
typedef struct user_struct *user_ptr_t;
typedef struct complex_struct **double_ptr_t;

/* Array typedef */
typedef int int_array_t[100];

/* Function pointer with struct parameter */
typedef void (*struct_callback)(struct user_struct *);

#endif /* TEST_GENGTYPE_TYPES_H */
