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
    struct plain_s *link;
    char name[20];
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *plain_ptr;          /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) recursive_s {            /* Self-referential GTY struct */
    int value;
    struct recursive_s *GTY((skip)) next;  /* Recursive pointer */
};

struct GTY(()) complex_user_s {         /* Complex GTY struct with multiple fields */
    struct user_s *user_ptr;            /* Pointer to another GTY struct */
    struct plain_s plain_instance;      /* Embedded plain struct */
    string_t name;                      /* String field */
};

/* ========== TYPE_UNION ========== */
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

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef void *generic_ptr_t;            /* Generic void pointer */

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container_s {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char multi_dim[3][4][5];            /* Multi-dimensional array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container_s {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
    void (*inline_cb)(struct user_s *); /* Inline function pointer declaration */
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structs using conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) generator_specific_s {
    int generator_only_field;
    struct user_s *linked_data;
};
#endif

#ifdef LANG_SPECIFIC
struct GTY(()) lang_struct_s {
    int lang_marker;
    void *lang_data;
};
#endif

/* ========== Complex Nested Types ========== */
/* Struct containing union containing GTY struct pointer */
struct GTY(()) nested_container_s {
    union {
        struct user_s *user_ptr;
        struct complex_user_s *complex_ptr;
        callback_fn cb;
    } data;
    
    struct {
        int tag;
        union my_u value;
    } tagged_value;
    
    struct array_container_s arrays;
};

/* ========== Forward Declarations ========== */
struct GTY(()) forward_declared_s;      /* Forward declaration */

struct GTY(()) uses_forward_s {
    struct forward_declared_s *fwd_ptr; /* Pointer to forward-declared type */
    int current_value;
};

struct GTY(()) forward_declared_s {     /* Actual definition */
    struct uses_forward_s *back_ptr;    /* Mutual recursion */
    char data[50];
};

/* ========== Mixed Type Example ========== */
struct GTY(()) comprehensive_example_s {
    /* Scalar fields */
    my_int id;
    my_double weight;
    
    /* String field */
    string_t description;
    
    /* Struct fields */
    struct plain_s plain_data;
    struct user_s *user_link;
    
    /* Union field */
    union my_u variant;
    
    /* Array fields */
    int scores[20];
    struct user_s *team[4];
    
    /* Callback field */
    callback_fn notify;
    
    /* Pointer to array */
    int (*matrix)[10];
    
    /* Nested struct */
    struct {
        int x, y;
    } position;
};

/* ========== Edge Cases ========== */
/* Empty struct */
struct GTY(()) empty_s {
    /* No fields */
};

/* Struct with only arrays */
struct GTY(()) arrays_only_s {
    char buffer[256];
    int matrix[3][3];
    void *pointers[10];
};

/* Struct with function pointer array */
struct GTY(()) callback_array_s {
    callback_fn handlers[8];
    void (*operations[5])(void);
};

#endif /* TEST_GENGTYPE_TYPES_H */
