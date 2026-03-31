/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h for GTY macro if needed */
#ifdef GTY
#undef GTY
#endif
#define GTY(x) x

/* For language-specific struct test */
#ifdef GENERATOR_FILE
#define LANG_SPECIFIC 1
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;
typedef _Bool my_bool;

/* ========== TYPE_STRING ========== */
typedef const char *string_t;
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT (plain, non-GTY) ========== */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    struct plain_s *link;
    int count;
};

/* ========== TYPE_USER_STRUCT (GTY-tagged) ========== */
struct GTY(()) user_s {
    struct plain_s *plain_ptr;      /* TYPE_POINTER to TYPE_STRUCT */
    int scalar_field;               /* TYPE_SCALAR */
    char name[32];                  /* TYPE_ARRAY of TYPE_SCALAR */
};

struct GTY(()) recursive_s {
    struct recursive_s *next;       /* Recursive pointer */
    struct user_s *user;            /* Pointer to another GTY struct */
    int data;
};

/* ========== TYPE_UNION ========== */
union my_u {
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {
    struct user_s *user_ptr;
    struct recursive_s *rec_ptr;
    int int_val;
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int numbers[10];                /* TYPE_ARRAY of TYPE_SCALAR */
    struct user_s *ptr_array[5];    /* TYPE_ARRAY of TYPE_POINTER */
    char string_array[3][64];       /* Multi-dimensional array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int, void*);
typedef int (*compare_fn)(const void*, const void*);

struct GTY(()) callback_container {
    callback_fn handler;            /* TYPE_CALLBACK */
    compare_fn comparator;          /* TYPE_CALLBACK */
    void *user_data;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;
typedef struct recursive_s **double_ptr_t;

struct GTY(()) pointer_heavy {
    user_ptr_t single_ptr;          /* TYPE_POINTER via typedef */
    struct recursive_s **dbl_ptr;   /* Pointer to pointer */
    void *generic_ptr;              /* Generic pointer */
    const char *const_string;       /* TYPE_STRING pointer */
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef LANG_SPECIFIC
struct GTY(()) lang_specific_s {
    int lang_feature;
    void *lang_data;
};
#endif

/* ========== Complex nested example ========== */
struct GTY(()) complex_node {
    struct complex_node *left;      /* Recursive pointer */
    struct complex_node *right;     /* Recursive pointer */
    union tagged_union data;        /* TYPE_UNION */
    callback_fn on_event;           /* TYPE_CALLBACK */
    char buffer[256];               /* TYPE_ARRAY */
    struct array_container arrays;  /* Nested struct */
};

/* ========== Mixed container ========== */
struct GTY(()) mixed_container {
    /* All type kinds in one struct */
    my_int scalar;                  /* TYPE_SCALAR (via typedef) */
    string_t str;                   /* TYPE_STRING */
    struct plain_s plain;           /* TYPE_STRUCT */
    union my_u uni;                 /* TYPE_UNION */
    struct user_s *user_ptr;        /* TYPE_POINTER */
    int numbers[8];                 /* TYPE_ARRAY */
    callback_fn cb;                 /* TYPE_CALLBACK */
    #ifdef LANG_SPECIFIC
    struct lang_specific_s *lang;   /* TYPE_LANG_STRUCT pointer */
    #endif
};

/* ========== Linked list example ========== */
struct GTY(()) list_node {
    int value;
    struct list_node *GTY((skip)) next;  /* Skip pointer for GC */
    struct list_node *prev;
};

/* ========== Function pointer typedefs ========== */
typedef void (*void_func)(void);
typedef int (*int_func)(int, int);
typedef struct user_s* (*factory_func)(int);

#endif /* TEST_GENGTYPE_TYPES_H */
