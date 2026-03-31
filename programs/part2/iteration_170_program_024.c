/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype.h for GTY macro definition */
#ifdef GENERATOR_FILE
#include "gtype.h"
#else
/* Simplified GTY macro for testing */
#define GTY(x) 
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                    /* Simple scalar typedef */
typedef unsigned long my_ulong;        /* Another scalar */
typedef double my_double;              /* Floating point scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;          /* String pointer type */
typedef char *mutable_string_t;        /* Mutable string pointer */

/* ========== TYPE_STRUCT ========== */
struct plain_struct {                  /* Untagged struct */
    int field1;
    double field2;
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_struct {           /* GTY-tagged struct */
    int GTY((skip)) id;                /* Skip this field for GC */
    string_t name;                     /* String field */
    struct user_struct *next;          /* Pointer to same type */
};

/* Another GTY-tagged struct for complex relationships */
struct GTY(()) complex_struct {
    my_int count;
    struct user_struct *users;         /* Pointer to another GTY struct */
    void *opaque;                      /* Untyped pointer */
};

/* ========== TYPE_UNION ========== */
union data_union {
    int int_val;
    double double_val;
    void *ptr_val;
    struct user_struct *user_ptr;      /* GTY pointer in union */
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    int tag;
    struct user_struct *GTY((tag("0"))) as_user;
    struct complex_struct *GTY((tag("1"))) as_complex;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_struct *user_ptr_t;  /* Pointer typedef */
typedef int *int_ptr_t;                  /* Pointer to scalar */

/* Struct with various pointer types */
struct GTY(()) pointer_struct {
    int *scalar_ptr;                    /* Pointer to scalar */
    struct plain_struct *plain_ptr;     /* Pointer to non-GTY struct */
    struct user_struct *user_ptr;       /* Pointer to GTY struct */
    void (*callback)(int);              /* Function pointer */
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_struct {
    int fixed_array[10];                /* Fixed-size array */
    struct user_struct *ptr_array[5];   /* Array of pointers */
    int variable_array[0];              /* Zero-length array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*simple_callback)(int);   /* Simple callback typedef */
typedef int (*complex_callback)(struct user_struct *, string_t);  /* Complex callback */

/* Struct with callback fields */
struct GTY(()) callback_struct {
    simple_callback on_event;
    complex_callback process;
    void (*inline_cb)(void);            /* Inline function pointer */
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific struct - only processed in generator context */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int lang_specific_field;
    struct user_struct *lang_ptr;
};
#endif

/* ========== Complex Nested Types ========== */

/* Recursive structure */
struct GTY(()) tree_node {
    int node_type;
    union {
        struct tree_node *left;
        struct user_struct *user_data;
    } GTY((desc("node_type"))) u;
    struct tree_node *right;
};

/* Container structure with multiple type kinds */
struct GTY(()) container {
    /* Scalar fields */
    my_int id;
    my_double value;
    
    /* String field */
    string_t description;
    
    /* Struct fields */
    struct plain_struct plain;
    
    /* Pointer fields */
    struct user_struct *owner;
    struct complex_struct *complex_data;
    
    /* Array field */
    struct user_struct *members[8];
    
    /* Union field */
    union data_union data;
    
    /* Callback field */
    simple_callback notify;
    
    /* Nested struct */
    struct {
        int nested_id;
        struct user_struct *nested_ptr;
    } GTY(()) nested;
    
    /* Pointer to array */
    int (*matrix)[4][4];
};

/* ========== Forward Declarations ========== */
struct GTY(()) forward_declared;  /* TYPE_UNDEFINED until defined */

struct GTY(()) forward_declared {
    int defined_now;
    struct container *container_ptr;
};

/* ========== Edge Cases ========== */

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int : 5;  /* Unnamed bitfield */
    unsigned int value : 8;
};

/* Const-qualified pointers */
struct GTY(()) const_struct {
    const struct user_struct *readonly_ptr;
    struct user_struct * const const_ptr;
    const struct user_struct * const const_ptr_to_const;
};

#endif /* TEST_GENGTYPE_TYPES_H */
