/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro */
#ifdef GTY
#undef GTY
#endif
#define GTY(x) x

/* For language-specific structs */
#ifdef GENERATOR_FILE
#define IN_GENERATOR
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String type */
typedef char *mutable_string_t;         /* Mutable string */

/* ========== TYPE_STRUCT ========== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *plain_ptr;          /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) recursive_s {            /* Recursive GTY struct */
    int value;
    struct recursive_s *GTY((skip)) next;  /* Recursive pointer */
};

/* ========== TYPE_UNION ========== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    int GTY((tag("0"))) as_int;
    struct user_s *GTY((tag("1"))) as_user;
    string_t GTY((tag("2"))) as_string;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef void *generic_ptr_t;            /* Generic pointer */

struct GTY(()) pointer_container {
    struct user_s *direct_ptr;          /* Direct pointer */
    struct user_s **double_ptr;         /* Pointer to pointer */
    void *GTY((skip)) opaque_ptr;       /* Skip-marked pointer */
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    int *int_ptr_array[3];              /* Array of int pointers */
    
    /* Variable length array (requires length field) */
    int vl_length;
    int variable_array[1];              /* VLA placeholder */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
    
    /* Nested callback in struct */
    struct {
        void (*nested_cb)(void);
    } callback_struct;
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef GENERATOR_FILE
/* This struct should only be seen by generator */
struct GTY(()) generator_only_s {
    int generator_field;
    void *generator_data;
};
#else
/* Alternative for non-generator context */
struct GTY(()) normal_struct_s {
    int normal_field;
};
#endif

/* ========== Complex Nested Types ========== */
struct GTY(()) complex_nested {
    /* Nested struct definition */
    struct GTY(()) inner_s {
        int inner_value;
        struct complex_nested *parent;  /* Pointer to parent */
    } inner;
    
    /* Union within struct */
    union {
        int as_int;
        struct inner_s *as_inner;
    } data;
    
    /* Array of structs */
    struct inner_s inner_array[3];
    
    /* Pointer to array */
    int (*matrix_ptr)[4][4];
    
    /* Callback array */
    callback_fn callbacks[2];
};

/* ========== Chain of Types ========== */
struct GTY(()) chain_a {
    int id;
    struct GTY(()) chain_b *next_b;
};

struct GTY(()) chain_b {
    int id;
    struct GTY(()) chain_c *next_c;
};

struct GTY(()) chain_c {
    int id;
    struct chain_a *next_a;  /* Completes the cycle */
};

/* ========== Mixed Container ========== */
struct GTY(()) mixed_container {
    /* All type kinds in one struct */
    my_int scalar_field;                /* TYPE_SCALAR */
    string_t string_field;              /* TYPE_STRING */
    struct plain_s plain_struct;        /* TYPE_STRUCT */
    union my_u union_field;             /* TYPE_UNION */
    user_ptr_t pointer_field;           /* TYPE_POINTER */
    int array_field[5];                 /* TYPE_ARRAY */
    callback_fn callback_field;         /* TYPE_CALLBACK */
    
    /* Self-reference */
    struct mixed_container *self;
};

/* ========== Template-like Pattern ========== */
#define DECLARE_CONTAINER(TYPE, NAME) \
    struct GTY(()) NAME { \
        TYPE *items; \
        int count; \
        int capacity; \
    }

DECLARE_CONTAINER(struct user_s, user_container);
DECLARE_CONTAINER(int, int_container);
DECLARE_CONTAINER(string_t, string_container);

/* ========== Edge Cases ========== */
/* Struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int : 4;  /* Unnamed bitfield */
    unsigned int value : 8;
};

/* Anonymous struct/union */
struct GTY(()) anonymous_container {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } data;
};

#endif /* TEST_GENGTYPE_TYPES_H */
