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
#define LANG_STRUCT_MARKER
#else
#define LANG_STRUCT_MARKER
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
struct plain_s {                        /* Plain C struct (not GTY) */
    int a;
    double b;
};

struct another_plain {                  /* Another plain struct */
    struct plain_s *link;
    char name[32];
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *plain_ptr;          /* Pointer to plain struct */
    int counter;
};

struct GTY(()) complex_user {           /* Another GTY-tagged struct */
    struct user_s *user_link;           /* Pointer to another GTY struct */
    struct GTY(()) complex_user *self;  /* Recursive pointer */
    int data;
};

/* ========== TYPE_UNION ========== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct user_s *user_ptr;
    struct complex_user *complex_ptr;
    int value;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef void *generic_ptr_t;            /* Generic pointer */

struct GTY(()) pointer_container {      /* Struct with various pointers */
    struct user_s *direct_ptr;          /* Direct pointer */
    struct GTY(()) pointer_container *next;  /* Linked list pointer */
    void *generic_ptr;                  /* Generic void pointer */
    const char *const_string;           /* Const string pointer */
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char string_array[3][32];           /* 2D array */
};

/* Variable length array in another struct */
struct GTY(()) vla_container {
    int count;
    int data[];                         /* Flexible array member */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
    void (*inline_cb)(struct user_s *); /* Inline function pointer type */
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {   /* Language-specific struct */
    int generator_only_field;
    struct user_s *linked_user;
};
#else
struct GTY(()) lang_specific_struct {   /* Different definition for non-generator */
    int runtime_field;
    struct user_s *linked_user;
};
#endif

/* ========== Complex Nested Types ========== */
struct GTY(()) nested_example {
    /* Nested struct definition */
    struct GTY(()) inner_struct {
        int inner_data;
        struct nested_example *parent;
    } inner;
    
    union GTY(()) inner_union {
        int as_int;
        struct inner_struct *as_struct;
    } u;
    
    /* Array of structs */
    struct inner_struct children[4];
    
    /* Pointer to array */
    int (*matrix_ptr)[3][3];
    
    /* Callback that takes this type */
    void (*notify)(struct nested_example *);
};

/* ========== Root Structure ========== */
struct GTY(()) root_container {
    struct user_s *user_instance;
    struct complex_user *complex_instance;
    union tagged_union variant;
    struct array_container arrays;
    struct callback_container callbacks;
    struct nested_example nested;
    struct lang_specific_struct *lang_struct;
    struct pointer_container *ptr_chain;
    
    /* Mixed array */
    void *mixed_ptrs[8];
    
    /* End marker */
    int final_count;
};

/* ========== Undefined Type Reference ========== */
struct GTY(()) forward_ref_user {
    struct not_yet_defined *future_ptr;  /* Forward reference - TYPE_UNDEFINED */
    int current_data;
};

/* Later definition to resolve forward reference */
struct GTY(()) not_yet_defined {
    struct forward_ref_user *back_ref;
    char name[64];
};

#endif /* TEST_GENGTYPE_TYPES_H */
