/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h if available in GCC build context */
#ifdef HAVE_GTYPE_DESC_H
#include "gtype-desc.h"
#endif

/* For GTY macro definition if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ==================== TYPE_SCALAR ==================== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */
typedef enum { RED, GREEN, BLUE } color_t; /* Enum is also scalar */

/* ==================== TYPE_STRING ==================== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string */

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

struct GTY(()) complex_user {           /* Another GTY-tagged struct */
    struct user_s *next;                /* Pointer to another GTY struct */
    struct user_s *prev;                /* Recursive pointer pattern */
    int id;
    char tag;
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct user_s *GTY((tag("0"))) usr_ptr;
    struct complex_user *GTY((tag("1"))) complex_ptr;
    int GTY((tag("2"))) value;
};

/* ==================== TYPE_POINTER ==================== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef struct complex_user **double_ptr_t; /* Double pointer */

struct GTY(()) pointer_container {      /* Struct with various pointers */
    struct user_s *single_ptr;          /* Single pointer */
    struct complex_user **double_ptr;   /* Double pointer */
    void *generic_ptr;                  /* Generic void pointer */
    struct pointer_container *self_ptr; /* Self-referential pointer */
};

/* ==================== TYPE_ARRAY ==================== */
struct GTY(()) array_container {
    int scalar_array[10];               /* Array of scalars */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char string_array[3][64];           /* 2D array (array of arrays) */
    int flexible_array[];               /* Flexible array member */
};

/* ==================== TYPE_CALLBACK ==================== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;              /* Another callback */
    void (*inline_callback)(struct user_s *); /* Inline callback type */
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structs using conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct_gen {
    int gen_specific_field;
    struct user_s *gen_ptr;
};
#else
struct GTY(()) lang_struct_normal {
    int normal_field;
    struct complex_user *normal_ptr;
};
#endif

/* Conditional based on LANG_TYPE macro */
#if defined(LANG_TYPE) && LANG_TYPE == 1
struct GTY(()) lang_specific_one {
    int lang1_field;
};
#elif defined(LANG_TYPE) && LANG_TYPE == 2
struct GTY(()) lang_specific_two {
    int lang2_field;
    struct user_s *lang2_ptr;
};
#endif

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations create undefined types initially */
struct forward_declared;                /* Forward declaration */
typedef struct forward_declared *fwd_ptr_t;

struct GTY(()) uses_forward {
    struct forward_declared *fwd_ptr;   /* Pointer to forward-declared type */
};

/* Later definition of the forward-declared struct */
struct forward_declared {
    int defined_now;
    struct uses_forward *back_ptr;
};

/* ==================== Complex Nested Example ==================== */
struct GTY(()) master_container {
    /* Mix of all types */
    my_int scalar_field;                /* TYPE_SCALAR */
    string_t string_field;              /* TYPE_STRING */
    struct plain_s plain_struct;        /* TYPE_STRUCT */
    struct user_s *user_struct_ptr;     /* TYPE_POINTER to TYPE_USER_STRUCT */
    union my_u union_field;             /* TYPE_UNION */
    callback_fn callback_field;         /* TYPE_CALLBACK */
    int array_field[8];                 /* TYPE_ARRAY */
    
    /* Nested struct with GTY */
    struct GTY(()) nested {
        int nested_id;
        struct master_container *parent;
    } nested_field;
    
    /* Union with GTY */
    union GTY(()) nested_union {
        int as_int;
        struct user_s *as_ptr;
    } union_choice;
    
    /* Array of callbacks */
    callback_fn callback_array[3];
    
    /* Pointer to array */
    int (*matrix_ptr)[4][4];
};

/* ==================== Root Structure ==================== */
/* This will be the main structure processed by gengtype */
struct GTY(()) root_struct {
    struct master_container *main;
    struct array_container *arrays;
    struct callback_container *callbacks;
    struct pointer_container *pointers;
    
    /* Chain of user structs */
    struct user_s *first_user;
    struct complex_user *first_complex;
    
    /* Language-specific based on compilation context */
#ifdef GENERATOR_FILE
    struct lang_struct_gen *lang_gen;
#else
    struct lang_struct_normal *lang_normal;
#endif
    
    /* Undefined type usage */
    fwd_ptr_t forward_pointer;
    
    /* End marker */
    int end_marker;
};

#endif /* TEST_GENGTYPE_TYPES_H */
