/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* TYPE_SCALAR: Basic scalar typedefs */
typedef int my_int;
typedef unsigned long my_ulong;
typedef double my_double;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Plain unions */
union plain_union {
    int i;
    float f;
    void *p;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char *, void *);

/* TYPE_USER_STRUCT: GTY-tagged structs for garbage collection */
struct GTY(()) user_struct {
    /* TYPE_POINTER: Pointer field */
    struct user_struct *next;
    
    /* TYPE_ARRAY: Array field */
    int values[10];
    
    /* Mixed pointer types */
    string_t name;
    my_int id;
    
    /* Nested plain struct */
    struct plain_struct plain;
    
    /* Callback field */
    simple_callback cb;
};

/* Another GTY-tagged struct with more complex relationships */
struct GTY(()) complex_struct {
    /* Pointer to another GTY-tagged struct */
    struct user_struct *user;
    
    /* Array of pointers */
    struct user_struct *users[5];
    
    /* Pointer array with variable length marker */
    struct user_struct **user_list GTY((length("user_count")));
    int user_count;
    
    /* Union containing GTY-tagged pointer */
    union {
        struct user_struct *uptr;
        void *vptr;
    } data;
    
    /* String array */
    const char *tags[3];
};

/* TYPE_LANG_STRUCT: Language-specific struct with conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *gen_ptr;
};
#endif

/* More complex type relationships for deep traversal */

/* Recursive struct definition */
struct GTY(()) recursive_struct {
    int value;
    struct recursive_struct *left;
    struct recursive_struct *right;
};

/* Struct with callback pointer */
struct GTY(()) struct_with_callback {
    int (*compare)(const void *, const void *);
    void *data;
};

/* Union with GTY-tagged members */
union GTY(()) tagged_union {
    struct user_struct *us;
    struct complex_struct *cs;
    int tag;
};

/* Array-only struct */
struct GTY(()) array_struct {
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Array of strings */
    const char *messages[5];
};

/* Struct with nested anonymous struct */
struct GTY(()) nested_anon_struct {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } value;
};

/* Forward declaration to test TYPE_UNDEFINED handling */
struct forward_declared_struct;

/* Struct using forward declared type */
struct GTY(()) uses_forward_decl {
    struct forward_declared_struct *fwd_ptr;
    int valid;
};

/* Now define the forward declared struct */
struct GTY(()) forward_declared_struct {
    int data;
    struct uses_forward_decl *back_ref;
};

/* Template-like macro usage (common in GCC) */
#define DEFINE_VEC_T(T) \
    struct GTY(()) vec_##T { \
        T *data; \
        int length; \
        int capacity; \
    }

/* Instantiate template-like macros */
DEFINE_VEC_T(int);
DEFINE_VEC_T(struct user_struct*);

/* Enumeration (should be treated as scalar) */
typedef enum {
    RED,
    GREEN,
    BLUE
} color_t;

/* Struct with enum */
struct GTY(()) struct_with_enum {
    color_t color;
    int intensity;
};

/* Function pointer with complex signature */
typedef void (*error_handler_t)(const char *file, int line, const char *msg);

/* Struct containing function pointer table */
struct GTY(()) handler_table {
    error_handler_t on_error;
    error_handler_t on_warning;
    simple_callback on_success;
};

#endif /* TEST_GENGTYPE_TYPES_H */
