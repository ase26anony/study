/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;
typedef double my_double;
typedef char my_char;

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String type */
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT ========== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

struct another_plain {
    struct plain_s *next;
    int count;
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *plain_ptr;          /* Pointer to plain struct */
    my_int value;
};

struct GTY(()) recursive_s {            /* Recursive GTY struct */
    int id;
    struct recursive_s *GTY((skip)) next;  /* Skip for recursion */
    struct user_s *user;
};

/* ========== TYPE_UNION ========== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct user_s *GTY((tag("0"))) as_user;
    struct recursive_s *GTY((tag("1"))) as_recursive;
    int GTY((tag("2"))) as_int;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef struct recursive_s **double_ptr_t;

struct GTY(()) pointer_container {
    void *GTY((skip)) opaque_ptr;       /* Untraced pointer */
    struct user_s *direct_ptr;          /* Direct pointer to GTY struct */
    struct recursive_s **indirect_ptr;  /* Pointer to pointer */
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    int *int_ptr_array[3];
};

/* Variable length array (requires length field) */
struct GTY(()) varray_container {
    int length;
    struct user_s *GTY((length("%h.length"))) items[];
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Function pointer field */
    compare_fn comparator;
    void (*direct_callback)(struct user_s *);
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structs - these are conditionally compiled */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct_s {
    int generator_specific;
    struct user_s *data;
};
#endif

#ifdef LANG_TYPE
struct GTY(()) another_lang_struct {
    void *lang_data;
    callback_fn lang_callback;
};
#endif

/* ========== Complex nested example ========== */
struct GTY(()) complex_nested {
    /* Scalar fields */
    my_int id;
    my_double weight;
    
    /* String field */
    string_t name;
    
    /* Struct fields */
    struct plain_s plain;
    
    /* User struct fields */
    struct user_s *user;
    struct recursive_s recursive;
    
    /* Union field */
    union my_u variant;
    
    /* Tagged union */
    union tagged_union tagged;
    
    /* Pointer fields */
    user_ptr_t user_ptr;
    void **void_dbl_ptr;
    
    /* Array fields */
    int scores[20];
    struct user_s *friends[10];
    
    /* Callback field */
    callback_fn notify;
    
    /* Nested struct with array of pointers to callbacks */
    struct GTY(()) nested_handler {
        callback_fn handlers[5];
        struct user_s *target;
    } handler;
    
    /* Pointer to array */
    int (*matrix)[10];
};

/* ========== Forward declarations ========== */
struct GTY(()) forward_declared_s;      /* TYPE_UNDEFINED until defined */

struct GTY(()) uses_forward {
    struct forward_declared_s *fwd_ptr;
};

/* Now define it to resolve TYPE_UNDEFINED */
struct GTY(()) forward_declared_s {
    int defined_now;
    struct uses_forward *back_ref;
};

/* ========== Chain of types ========== */
struct GTY(()) chain_link {
    int value;
    struct chain_link *next;
    struct chain_link *prev;
};

/* ========== Container with all types ========== */
struct GTY(()) master_container {
    /* Scalar */
    my_int scalar_field;
    
    /* String */
    string_t string_field;
    
    /* Plain struct */
    struct plain_s plain_field;
    
    /* User struct */
    struct user_s user_field;
    
    /* Union */
    union my_u union_field;
    
    /* Pointer */
    struct recursive_s *pointer_field;
    
    /* Array */
    struct user_s *array_field[8];
    
    /* Callback */
    callback_fn callback_field;
    
    /* Nested container */
    struct array_container nested_array;
    
    /* Language struct (conditional) */
#ifdef SPECIAL_BUILD
    struct GTY(()) special_s {
        int special_data;
    } special_field;
#endif
};

#endif /* TEST_GENGTYPE_TYPES_H */
