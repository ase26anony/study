/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

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
typedef char *mutable_string_t;         /* Mutable string */

/* ========== TYPE_STRUCT ========== */
struct plain_s {                        /* Untagged struct */
    int a;
    double b;
};

struct another_plain {                  /* Another untagged struct */
    struct plain_s *link;
    char name[32];
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *plain_ptr;          /* Pointer to plain struct */
    my_int scalar_field;                /* Scalar field */
    string_t str_field;                 /* String field */
};

struct GTY(()) recursive_s {            /* Recursive GTY struct */
    int value;
    struct recursive_s *GTY((skip)) next;  /* Recursive pointer */
};

struct GTY(()) container_s {            /* Container with arrays */
    struct user_s *GTY((tag("0"))) items[10];  /* Array of pointers */
    int counts[20];                     /* Array of scalars */
};

/* ========== TYPE_UNION ========== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct user_s *GTY((tag("1"))) usr_ptr;
    struct container_s *GTY((tag("2"))) cont_ptr;
    my_int scalar_val;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef void *generic_ptr_t;            /* Generic pointer */

struct GTY(()) pointer_heavy_s {        /* Struct with many pointers */
    struct user_s *direct_ptr;          /* Direct pointer */
    struct container_s **double_ptr;    /* Pointer to pointer */
    generic_ptr_t generic;              /* Generic pointer */
    struct recursive_s *recursive_ptr;  /* Another pointer */
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_s {                /* Struct with various arrays */
    int scalar_array[50];               /* Array of scalars */
    struct user_s *ptr_array[20];       /* Array of pointers */
    char string_array[10][100];         /* 2D array */
    double matrix[4][4];                /* Multi-dimensional */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int, void*);  /* Function pointer typedef */
typedef int (*comparator_fn)(const void*, const void*);

struct GTY(()) callback_s {             /* Struct with callback */
    callback_fn handler;                /* Function pointer field */
    comparator_fn compare;              /* Another function pointer */
    void *user_data;
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef GENERATOR_FILE
/* These structs are only processed when GENERATOR_FILE is defined */
struct GTY(()) lang_struct_s {
    int lang_specific_field;
    struct user_s *associated;
};

union GTY(()) lang_union_u {
    struct lang_struct_s *ls;
    struct callback_s *cb;
};
#endif

/* ========== Complex Nested Types ========== */
struct GTY(()) complex_nested_s {
    /* Nested struct definition */
    struct GTY(()) inner_s {
        int inner_val;
        struct complex_nested_s *parent;  /* Pointer to parent */
    } inner;
    
    /* Union field */
    union {
        struct user_s *usr;
        struct container_s *cont;
        callback_fn handler;
    } GTY((tag("3"))) choice;
    
    /* Array of structs */
    struct inner_s inner_array[5];
    
    /* Pointer to callback struct */
    struct callback_s *callback_ptr;
};

/* ========== Forward Declarations ========== */
struct GTY(()) forward_declared_s;      /* Forward declaration */

struct GTY(()) uses_forward_s {
    int id;
    struct forward_declared_s *GTY((skip)) fwd_ptr;  /* Pointer to forward declared */
};

struct GTY(()) forward_declared_s {     /* Actual definition */
    int value;
    struct uses_forward_s *back_ref;
};

/* ========== Mixed Type Example ========== */
struct GTY(()) mixed_types_s {
    /* All type kinds in one struct */
    my_int scalar;                      /* TYPE_SCALAR */
    string_t str;                       /* TYPE_STRING */
    struct plain_s plain;               /* TYPE_STRUCT */
    struct user_s *user_ptr;            /* TYPE_POINTER to TYPE_USER_STRUCT */
    union my_u data;                    /* TYPE_UNION */
    int numbers[10];                    /* TYPE_ARRAY */
    callback_fn func;                   /* TYPE_CALLBACK */
    #ifdef GENERATOR_FILE
    struct lang_struct_s *lang_ptr;     /* TYPE_LANG_STRUCT */
    #endif
};

#endif /* TEST_GENGTYPE_TYPES_H */
