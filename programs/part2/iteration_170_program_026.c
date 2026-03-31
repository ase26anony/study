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
typedef char *mutable_string_t;         /* Mutable string pointer */

/* ========== TYPE_STRUCT ========== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

struct another_plain {                  /* Another plain struct */
    char c;
    struct plain_s *ps;
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *p;                  /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) complex_user {           /* Another GTY-tagged struct */
    struct user_s *next;                /* Pointer to another GTY struct */
    struct user_s *prev;                /* Another pointer */
    int data;
};

/* ========== TYPE_UNION ========== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct user_s *us;                  /* Pointer to GTY struct */
    union my_u *plain_union;            /* Pointer to plain union */
    long value;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef struct plain_s **double_ptr_t;  /* Double pointer typedef */

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_arr[10];                  /* Fixed-size array */
    struct user_s *ptr_arr[5];          /* Array of pointers */
    double multi_dim[3][4];             /* Multi-dimensional array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;              /* Another callback */
    void (*simple_cb)(void);            /* Direct function pointer */
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef GENERATOR_FILE
/* This struct should only be seen by gengtype */
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_s *gen_ptr;
};
#endif

/* Conditional for other contexts */
#ifndef GENERATOR_FILE
struct GTY(()) regular_struct {
    int regular_field;
};
#endif

/* ========== Complex Nested Types ========== */
struct GTY(()) outer_container {
    struct complex_user *inner;         /* Pointer to GTY struct */
    struct array_container arrays;      /* Embedded struct with arrays */
    union tagged_union variant;         /* Embedded union */
    callback_fn callbacks[3];           /* Array of callbacks */
    
    /* Self-referential pointer for recursion */
    struct outer_container *GTY((skip)) next;
    
    /* Pointer chain */
    struct user_s *user_chain;
};

/* ========== Mixed Type Container ========== */
struct GTY(()) mixed_types {
    /* Scalars */
    my_int id;
    my_double weight;
    
    /* Strings */
    string_t name;
    mutable_string_t buffer;
    
    /* Structs */
    struct plain_s plain;
    struct user_s *user;
    
    /* Unions */
    union my_u choice;
    union tagged_union *tagged_choice;
    
    /* Arrays */
    int scores[20];
    struct user_s *friends[10];
    
    /* Callbacks */
    callback_fn on_event;
    
    /* Nested container */
    struct outer_container *container;
};

/* ========== Forward Declarations ========== */
struct GTY(()) forward_declared;        /* TYPE_UNDEFINED until defined */

struct GTY(()) uses_forward {
    struct forward_declared *fd;        /* Pointer to forward-declared type */
};

struct GTY(()) forward_declared {       /* Now defined - becomes TYPE_USER_STRUCT */
    int value;
    struct uses_forward *uf;
};

/* ========== Template-like Pattern ========== */
#define DECLARE_CONTAINER(TYPE, NAME) \
    struct GTY(()) NAME { \
        TYPE *items; \
        int count; \
    }

DECLARE_CONTAINER(struct user_s, user_container);
DECLARE_CONTAINER(int, int_container);

/* ========== Edge Cases ========== */
/* Empty struct */
struct GTY(()) empty_struct {
    /* No fields */
};

/* Struct with only arrays */
struct GTY(()) array_only {
    int matrix[5][5];
    char strings[10][50];
};

/* Struct with only callbacks */
struct GTY(()) callbacks_only {
    void (*start)(void);
    int (*process)(int);
    void (*finish)(int);
};

#endif /* TEST_GENGTYPE_TYPES_H */
