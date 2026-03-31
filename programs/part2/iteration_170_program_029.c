/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */
typedef char my_char;                   /* Character scalar */
typedef _Bool my_bool;                  /* Boolean scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string pointer */

/* ========== TYPE_STRUCT ========== */
/* Plain C structs (not GTY-tagged) */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    struct plain_s *link;
    char name[32];
};

/* ========== TYPE_USER_STRUCT ========== */
/* GTY-tagged structs for garbage collection */
struct GTY(()) user_s {
    struct plain_s *p;                  /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) complex_user {
    struct user_s *next;                /* Pointer to another GTY struct */
    struct user_s *prev;                /* Another pointer */
    string_t name;                      /* String type */
    int values[10];                     /* Array type */
};

/* Nested GTY struct */
struct GTY(()) outer_struct {
    struct GTY(()) inner {
        int x;
        struct outer_struct *parent;    /* Recursive pointer */
    } *inner_ptr;
    
    struct complex_user *users;         /* Pointer to another GTY struct */
    int id;
};

/* ========== TYPE_UNION ========== */
/* Plain union */
union my_u {
    int i;
    void *p;
    double d;
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    struct user_s *us;                  /* Pointer to GTY struct */
    string_t str;                       /* String */
    int num;
};

/* ========== TYPE_POINTER ========== */
/* Pointer typedefs */
typedef struct user_s *user_ptr_t;
typedef struct complex_user **double_ptr_t;

/* Pointer fields in GTY structs */
struct GTY(()) pointer_container {
    void *generic_ptr;                  /* Generic pointer */
    struct user_s *specific_ptr;        /* Specific pointer */
    struct pointer_container *self_ptr; /* Self-referential pointer */
    user_ptr_t typedef_ptr;             /* Pointer via typedef */
};

/* ========== TYPE_ARRAY ========== */
/* Arrays in structs */
struct GTY(()) array_container {
    int simple_array[20];               /* Simple array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char string_array[3][64];           /* 2D array */
    double *pointer_to_array;           /* Pointer to array (not array type itself) */
};

/* Array typedef */
typedef int int_array_t[100];

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef void (*callback_fn)(int, void*);
typedef int (*compare_fn)(const void*, const void*);
typedef struct user_s* (*factory_fn)(string_t);

/* GTY struct with callback */
struct GTY(()) callback_container {
    callback_fn handler;                /* Function pointer field */
    compare_fn comparator;              /* Another function pointer */
    void *user_data;
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structs */
#ifdef GENERATOR_FILE
/* This struct should only be seen by gengtype/generator */
struct GTY(()) lang_specific_gen {
    int generator_only_field;
    struct user_s *linked;
};
#endif

#ifdef GCC
/* GCC-specific struct */
struct GTY(()) lang_specific_gcc {
    int gcc_only_field;
    double special_value;
};
#endif

/* Generic fallback if neither macro is defined */
#ifndef GENERATOR_FILE
#ifndef GCC
struct GTY(()) lang_specific_fallback {
    int fallback_field;
    string_t description;
};
#endif
#endif

/* ========== TYPE_UNDEFINED ========== */
/* Forward declarations create undefined types initially */
struct GTY(()) forward_declared;        /* Forward declaration */

struct GTY(()) uses_forward {
    struct forward_declared *fd;        /* Pointer to forward-declared type */
    int ready;
};

/* Later definition of forward-declared type */
struct GTY(()) forward_declared {
    struct uses_forward *uf;            /* Mutual reference */
    string_t name;
    int value;
};

/* ========== Complex Nested Example ========== */
/* This ensures deep traversal of type graph */
struct GTY(()) root_struct {
    struct GTY(()) {
        int level;
        struct root_struct *root;
        union GTY(()) {
            struct user_s *us;
            struct complex_user *cu;
            callback_fn handler;
        } data;
    } *nested;
    
    struct array_container arrays;
    struct callback_container callbacks;
    union tagged_union variant;
    int_array_t large_buffer;
};

/* ========== Additional Edge Cases ========== */
/* Struct with all type kinds */
struct GTY(()) kitchen_sink {
    /* SCALAR */
    my_int scalar_field;
    
    /* STRING */
    string_t string_field;
    
    /* STRUCT (plain) */
    struct plain_s plain_struct;
    
    /* USER_STRUCT (via pointer) */
    struct user_s *user_struct_ptr;
    
    /* UNION */
    union my_u union_field;
    
    /* POINTER */
    void **double_ptr_field;
    
    /* ARRAY */
    struct user_s *ptr_array_field[8];
    
    /* CALLBACK */
    factory_fn factory_field;
    
    /* LANG_STRUCT (conditional) */
#ifdef GENERATOR_FILE
    struct lang_specific_gen *lang_struct_ptr;
#endif
};

#endif /* TEST_GENGTYPE_TYPES_H */
