/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;
typedef double my_float;
typedef char my_char;

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT (untagged) ========== */
struct plain_struct {
    int field1;
    double field2;
};

/* ========== TYPE_USER_STRUCT (GTY-tagged) ========== */
struct GTY(()) user_struct {
    int id;
    struct plain_struct *plain_ptr;     /* Pointer to untagged struct */
    string_t name;                      /* String type */
};

/* ========== TYPE_UNION ========== */
union my_union {
    int int_val;
    double float_val;
    void *ptr_val;
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    struct user_struct *user_ptr;
    int tag;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_struct *user_ptr_t; /* Pointer typedef */
typedef int *int_ptr_t;

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct user_struct *ptr_array[5];   /* Array of pointers */
    int multi_dim[3][4];                /* Multi-dimensional array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int, const char*);  /* Function pointer typedef */
typedef int (*compare_fn)(const void*, const void*);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field in GTY struct */
    compare_fn comparator;
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific struct using conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *linked;
};
#else
struct GTY(()) lang_specific_struct {
    int normal_field;
    struct user_struct *linked;
};
#endif

/* ========== Complex Nested/Recursive Types ========== */

/* Recursive GTY struct */
struct GTY(()) tree_node {
    int node_type;
    struct tree_node *GTY((skip)) left;    /* Skip this for GC */
    struct tree_node *right;                /* Regular pointer */
    struct tree_node **children;            /* Pointer to pointer */
    int child_count;
};

/* Container with multiple type kinds */
struct GTY(()) complex_container {
    /* Scalar fields */
    my_int scalar1;
    my_float scalar2;
    
    /* String field */
    string_t description;
    
    /* Struct fields */
    struct plain_struct plain;
    struct user_struct *user;
    
    /* Union field */
    union my_union data;
    
    /* Array fields */
    int scores[20];
    struct user_struct *users[8];
    
    /* Pointer fields */
    void *opaque_ptr;
    int *int_array_ptr;
    
    /* Callback field */
    callback_fn notify;
    
    /* Nested struct */
    struct GTY(()) nested {
        int depth;
        struct complex_container *parent;
    } inner;
    
    /* For language-specific testing */
#ifdef GENERATOR_FILE
    struct lang_specific_struct *lang_data;
#endif
};

/* Union containing GTY pointer */
union GTY(()) gty_union_container {
    struct user_struct *user_data;
    struct array_container *array_data;
    int tag;
};

/* ========== Additional Edge Cases ========== */

/* Struct with anonymous union */
struct GTY(()) with_anonymous_union {
    int type;
    union {
        int int_val;
        double float_val;
        struct user_struct *user_ptr;
    } data;
};

/* Typedef for struct (should still be TYPE_USER_STRUCT) */
typedef struct GTY(()) {
    int x;
    int y;
} point_t;

/* Array of callbacks */
typedef callback_fn callback_array_t[5];

/* Struct with array of callbacks */
struct GTY(()) has_callback_array {
    callback_array_t handlers;
    int handler_count;
};

/* Forward declaration that creates TYPE_UNDEFINED initially */
struct GTY(()) forward_declared_struct;

struct GTY(()) uses_forward_decl {
    int id;
    struct forward_declared_struct *next;  /* TYPE_UNDEFINED until defined */
};

struct GTY(()) forward_declared_struct {
    int value;
    struct uses_forward_decl *owner;
};

#endif /* TEST_GENGTYPE_TYPES_H */
