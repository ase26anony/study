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
typedef char *mutable_string_t;         /* Mutable string pointer */

/* ==================== TYPE_STRUCT ==================== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

struct another_plain {                  /* Another plain struct */
    struct plain_s *link;
    char name[32];
};

/* ==================== TYPE_USER_STRUCT ==================== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *p;                  /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) recursive_s {            /* Self-referential GTY struct */
    int value;
    struct recursive_s *GTY((skip)) next;  /* Recursive pointer */
};

struct GTY(()) complex_user_s {         /* Complex GTY struct with multiple fields */
    struct user_s *user;                /* Pointer to another GTY struct */
    struct plain_s plain;               /* Embedded plain struct */
    string_t name;                      /* String field */
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct user_s *GTY((tag("0"))) usr;
    struct complex_user_s *GTY((tag("1"))) complex;
    int GTY((tag("2"))) value;
};

/* ==================== TYPE_POINTER ==================== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef void *generic_ptr_t;            /* Generic void pointer */

/* ==================== TYPE_ARRAY ==================== */
struct GTY(()) array_container {
    int fixed_arr[10];                  /* Fixed-size array */
    struct user_s *ptr_arr[5];          /* Array of pointers */
    char str_arr[3][32];                /* 2D array */
};

/* ==================== TYPE_CALLBACK ==================== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
    void (*inline_cb)(struct user_s *); /* Inline function pointer declaration */
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structs - using GCC's generator file detection */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct_gen {
    int generator_specific;
    struct user_s *linked;
};
#endif

/* Alternative: Use a macro that expands differently based on context */
#define MAYBE_LANG_STRUCT(name) struct GTY(()) name

MAYBE_LANG_STRUCT(lang_struct_variant) {
    int variant_field;
    struct array_container *arr_ptr;
};

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations create undefined types initially */
struct forward_declared;                /* Forward declaration */
typedef struct forward_declared *fwd_ptr_t;

struct GTY(()) uses_forward {
    fwd_ptr_t fwd;                      /* Pointer to forward-declared type */
};

/* Later definition of forward-declared type */
struct forward_declared {
    int defined_now;
    struct uses_forward *back_ref;
};

/* ==================== Nested and Complex Types ==================== */
struct GTY(()) outer_container {
    union tagged_union data;            /* Nested union */
    struct {                            /* Anonymous struct */
        int x;
        int y;
    } point;
    struct GTY(()) inner_s {            /* Nested struct definition */
        int depth;
        struct outer_container *parent; /* Recursive pointer */
    } inner;
    struct inner_s *sibling;            /* Pointer to nested type */
};

/* Array of unions */
union mixed_array {
    int ints[4];
    void *ptrs[4];
};

struct GTY(()) last_struct {
    union mixed_array arr_union;        /* Union containing arrays */
    callback_fn callbacks[2];           /* Array of callbacks */
    struct outer_container *outer;      /* Complex pointer chain */
};

#endif /* TEST_GENGTYPE_TYPES_H */
