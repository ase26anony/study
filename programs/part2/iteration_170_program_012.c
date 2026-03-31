/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* TYPE_SCALAR: Basic scalar typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_s {
    int a;
    double b;
};

/* TYPE_USER_STRUCT: GTY-tagged structs for garbage collection */
struct GTY(()) user_s {
    struct plain_s *p;  /* Pointer to plain struct */
    my_int count;
};

/* Another GTY-tagged struct for complex relationships */
struct GTY(()) complex_s {
    struct user_s *next;  /* Recursive pointer */
    string_t name;        /* String type */
};

/* TYPE_UNION: Union definitions */
union my_u {
    int i;
    void *p;
    double d;
};

/* GTY-tagged union */
union GTY(()) tagged_u {
    struct user_s *us;
    struct complex_s *cs;
    my_int *int_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef struct user_s *user_ptr_t;
typedef void (*void_func_ptr)(void);
typedef int *int_ptr_array[10];

/* TYPE_ARRAY: Array types within structs */
struct GTY(()) array_container {
    int fixed_array[20];
    struct user_s *pointer_array[15];
    char string_array[10][50];
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int, const char*);
typedef int (*compare_fn)(const void*, const void*);

/* GTY-tagged struct with callback */
struct GTY(()) callback_container {
    callback_fn handler;
    compare_fn comparator;
    void *user_data;
};

/* Nested complex type with multiple type kinds */
struct GTY(()) nested_s {
    /* Scalar fields */
    my_int id;
    my_double value;
    
    /* String field */
    string_t description;
    
    /* Pointer fields */
    struct nested_s *parent;
    struct nested_s *children[5];  /* Array of pointers */
    
    /* Union field */
    union my_u data;
    
    /* Callback field */
    callback_fn notify;
    
    /* Embedded struct */
    struct {
        int x;
        int y;
    } position;
};

/* Recursive type pattern */
struct GTY(()) tree_node {
    string_t data;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Array of structs */
struct GTY(()) item {
    int id;
    string_t name;
};

struct GTY(()) collection {
    int count;
    struct item items[100];  /* Array of GTY structs */
};

/* Language-specific struct (TYPE_LANG_STRUCT) */
#ifdef GENERATOR_FILE
/* This struct should only be processed by gengtype/generator */
struct GTY(()) lang_specific_s {
    int generator_only_field;
    void *generator_data;
};
#endif

/* Conditional compilation for different contexts */
#if defined(GENERATOR_FILE) || defined(IN_GCC)
struct GTY(()) conditional_s {
    int context_specific;
    union tagged_u data;
};
#endif

/* Forward declarations to test TYPE_UNDEFINED handling */
struct forward_declared_s;
union forward_declared_u;

/* GTY struct with forward declared pointer */
struct GTY(()) uses_forward {
    struct forward_declared_s *fwd_ptr;
    union forward_declared_u *fwd_union_ptr;
};

/* Now define the forward declared types */
struct forward_declared_s {
    int defined_later;
};

union forward_declared_u {
    int i;
    struct uses_forward *uf;
};

/* Template-like macro usage (common in GCC) */
#define DEFINE_VEC_T(T) \
struct GTY(()) vec_##T { \
    T *data; \
    int length; \
    int capacity; \
}

DEFINE_VEC_T(struct user_s);
DEFINE_VEC_T(string_t);

/* Variadic struct with function pointer array */
struct GTY(()) dispatcher {
    const char *name;
    void (*handlers[10])(void);  /* Array of function pointers */
    int priorities[10];
};

#endif /* TEST_GENGTYPE_TYPES_H */
