/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype header for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ==================== TYPE_SCALAR ==================== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */

/* ==================== TYPE_STRING ==================== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string */

/* ==================== TYPE_STRUCT ==================== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

/* ==================== TYPE_USER_STRUCT ==================== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *plain_ptr;          /* Pointer to plain struct */
    int counter;
};

/* Nested GTY struct for complex traversal */
struct GTY(()) nested_user_s {
    struct user_s *parent;              /* Pointer to another GTY struct */
    struct nested_user_s *next;         /* Recursive pointer */
    int depth;
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

/* GTY-tagged union */
union GTY(()) tagged_u {
    struct user_s *GTY((tag("0"))) us;  /* Tagged pointer in union */
    int GTY((tag("1"))) value;
    void *GTY((tag("2"))) generic;
};

/* ==================== TYPE_POINTER ==================== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef void (*void_func_ptr)(void);    /* Function pointer typedef */

/* Struct with various pointer types */
struct GTY(()) pointer_heavy_s {
    struct user_s *direct;              /* Direct pointer */
    struct nested_user_s **double_ptr;  /* Pointer to pointer */
    void *generic_ptr;                  /* Generic void pointer */
    const char *const_string;           /* String pointer */
};

/* ==================== TYPE_ARRAY ==================== */
struct GTY(()) array_s {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char string_array[3][20];           /* 2D array */
};

/* Variable-length array in GTY struct (requires special handling) */
struct GTY(()) varray_s {
    int length;
    int items[1];                       /* Variable length array */
};

/* ==================== TYPE_CALLBACK ==================== */
typedef int (*compare_fn)(const void *, const void *);  /* Callback typedef */
typedef void (*traversal_fn)(struct user_s *);          /* Another callback */

/* Struct using callback */
struct GTY(()) callback_s {
    compare_fn GTY((skip)) comparator;  /* Skip callback in GC */
    traversal_fn GTY((skip)) traverser;
    void *user_data;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structs using conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) gen_s {
    int generator_specific;
};
#endif

#ifdef LANG_SPECIFIC
struct GTY(()) lang_s {
    void *language_data;
};
#endif

/* Simulate GCC's language-specific macro pattern */
#define DEFTREECODE(SYM, STRING, TYPE, NARGS) \
    struct GTY(()) SYM##_node { \
        int code; \
        void *operands[NARGS]; \
    };

/* Example tree codes that would generate lang structs */
DEFTREECODE(PLUS, "+", '2', 2)
DEFTREECODE(MINUS, "-", '2', 2)
DEFTREECODE(MULT, "*", '2', 2)

#undef DEFTREECODE

/* ==================== COMPLEX NESTED STRUCTURE ==================== */
/* This ensures deep traversal of the type graph */
struct GTY(()) root_s {
    struct user_s *user;                /* TYPE_USER_STRUCT */
    struct plain_s plain;               /* TYPE_STRUCT (embedded) */
    union tagged_u choice;              /* TYPE_UNION (GTY-tagged) */
    struct array_s arrays;              /* TYPE_ARRAY (embedded) */
    struct callback_s callbacks;        /* TYPE_CALLBACK fields */
    
    /* Pointer chain for traversal */
    struct root_s *next;
    struct root_s **prev_ptr;           /* Pointer to pointer */
    
    /* Mixed array of pointers */
    void *mixed_ptrs[8];                /* Array of void pointers */
    
    /* String handling */
    const char *name;                   /* TYPE_STRING */
    char buffer[256];                   /* Array of chars */
};

/* ==================== UNDEFINED TYPE HANDLING ==================== */
/* Forward declaration that might create TYPE_UNDEFINED during processing */
struct GTY(()) forward_declared_s;      /* Forward declaration */

struct GTY(()) uses_forward_s {
    struct forward_declared_s *fwd_ptr; /* Pointer to forward-declared type */
    int valid;
};

/* Later definition */
struct GTY(()) forward_declared_s {
    struct uses_forward_s *back_ptr;    /* Circular reference */
    int data;
};

/* ==================== FUNCTION POINTER TYPEDEFS ==================== */
/* More callback types for coverage */
typedef void (*void_callback)(void);
typedef int (*int_callback)(int, int);
typedef struct user_s* (*factory_fn)(int id);

#endif /* TEST_GENGTYPE_TYPES_H */
