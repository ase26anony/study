/* gengtype_test_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
# define GTY(x) x
#endif

/* ==================== TYPE_SCALAR ==================== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;
typedef double my_float;

/* ==================== TYPE_STRING ==================== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;

/* ==================== TYPE_STRUCT ==================== */
struct plain_struct {                   /* Untagged struct */
    int field1;
    double field2;
};

/* ==================== TYPE_USER_STRUCT ==================== */
struct GTY(()) user_struct {            /* GTY-tagged struct */
    int GTY((tag("0"))) tag_field;
    struct plain_struct *plain_ptr;     /* Pointer to plain struct */
};

/* Another GTY-tagged struct for recursion */
struct GTY(()) recursive_struct {
    int value;
    struct recursive_struct *GTY((skip)) next;  /* Recursive pointer */
};

/* ==================== TYPE_UNION ==================== */
union my_union {                        /* Plain union */
    int int_val;
    double double_val;
    void *void_ptr;
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    int GTY((tag("0"))) int_field;
    struct user_struct *GTY((tag("1"))) struct_ptr;
};

/* ==================== TYPE_POINTER ==================== */
typedef struct user_struct *user_ptr_t; /* Pointer typedef */
typedef void *generic_ptr_t;

/* Struct with various pointers */
struct GTY(()) pointer_struct {
    int *int_ptr;                       /* Pointer to scalar */
    struct user_struct *user_ptr;       /* Pointer to GTY struct */
    struct plain_struct *plain_ptr;     /* Pointer to plain struct */
    void (*callback)(void);             /* Function pointer */
};

/* ==================== TYPE_ARRAY ==================== */
struct GTY(()) array_struct {
    int fixed_array[10];                /* Fixed-size array */
    struct user_struct *ptr_array[5];   /* Array of pointers */
    char string_array[3][20];           /* 2D array */
};

/* Variable-length array in GTY struct */
struct GTY(()) varray_struct {
    int length;
    int GTY((length("%0.length"))) elements[];
};

/* ==================== TYPE_CALLBACK ==================== */
typedef void (*simple_callback)(int);   /* Function pointer typedef */
typedef int (*complex_callback)(struct user_struct *, string_t);

struct GTY(()) callback_container {
    simple_callback cb1;
    complex_callback cb2;
    void (*inline_cb)(void);
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific struct - using GENERATOR_FILE macro as trigger */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *gen_ptr;
};
#endif

/* Another conditional for LTO */
#ifdef LTO_STREAMER_H
struct GTY(()) lto_struct {
    unsigned int lto_field;
};
#endif

/* ==================== COMPLEX NESTED STRUCTURES ==================== */

/* Struct containing union */
struct GTY(()) struct_with_union {
    int type;
    union {
        int int_val;
        struct user_struct *struct_ptr;
        string_t string_val;
    } GTY((desc("%0.type"))) data;
};

/* Struct with nested struct */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        struct outer_struct *parent;
    } *inner;
    
    struct array_struct arrays;
    struct callback_container callbacks;
};

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration that might not be defined */
struct undefined_struct;

/* Struct with pointer to undefined type */
struct GTY(()) uses_undefined {
    int valid_field;
    struct undefined_struct *GTY((skip)) undefined_ptr; /* Skip undefined */
};

/* ==================== ROOT STRUCTURE ==================== */
/* Main structure that references everything */
struct GTY(()) root_container {
    /* Scalars */
    my_int scalar1;
    my_float scalar2;
    
    /* Strings */
    string_t message;
    mutable_string_t buffer;
    
    /* Structs */
    struct plain_struct plain;
    struct user_struct *user;
    struct recursive_struct *recursive;
    
    /* Unions */
    union my_union plain_union;
    union tagged_union *tagged_union_ptr;
    
    /* Pointers */
    user_ptr_t user_pointer;
    generic_ptr_t generic_pointer;
    
    /* Arrays */
    struct array_struct arrays;
    struct varray_struct *varray;
    
    /* Callbacks */
    simple_callback simple_cb;
    struct callback_container *cb_container;
    
    /* Language-specific */
#ifdef GENERATOR_FILE
    struct lang_specific_struct *lang_specific;
#endif
    
    /* Complex types */
    struct struct_with_union *complex;
    struct outer_struct *nested;
    
    /* Undefined reference */
    struct uses_undefined *with_undefined;
    
    /* Self-reference for completeness */
    struct root_container *next;
};

#endif /* GTY_TEST_TYPES_H */
