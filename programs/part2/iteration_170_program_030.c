/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
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

/* TYPE_USER_STRUCT: GTY-tagged structs */
struct GTY(()) user_struct {
    /* TYPE_POINTER: Pointer field */
    struct user_struct *next;
    
    /* TYPE_ARRAY: Array field */
    int values[10];
    
    /* Nested plain struct */
    struct plain_struct plain;
    
    /* String field */
    string_t name;
};

/* Another GTY-tagged struct for complex dependencies */
struct GTY(()) complex_struct {
    /* Pointer to another GTY-tagged struct */
    struct user_struct *user;
    
    /* Pointer to self (recursive) */
    struct complex_struct *self;
    
    /* Array of pointers */
    struct user_struct *users[5];
    
    /* Scalar field */
    my_int count;
};

/* TYPE_UNION: Union definitions */
union my_union {
    int int_val;
    double double_val;
    void *ptr_val;
    struct user_struct *user_ptr;
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    struct user_struct *us;
    struct complex_struct *cs;
    int tag;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct user_struct *, my_int);

/* GTY-tagged struct with callback field */
struct GTY(()) callback_container {
    simple_callback cb;
    complex_callback complex_cb;
    
    /* Union containing callback */
    union {
        simple_callback scb;
        complex_callback ccb;
    } callback_union;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *gen_user;
};
#endif

/* Conditional compilation for different contexts */
#if defined(GENERATOR_FILE) || defined(IN_GCC)
struct GTY(()) conditional_struct {
    int context_specific;
#ifdef GENERATOR_FILE
    string_t generator_string;
#else
    const char *compiler_string;
#endif
};
#endif

/* Array of structs */
struct GTY(()) array_container {
    struct user_struct users[3];
    struct complex_struct *ptr_array[8];
};

/* Nested struct with all type kinds */
struct GTY(()) master_struct {
    /* Scalar */
    my_int id;
    
    /* String */
    string_t description;
    
    /* Struct */
    struct plain_struct plain;
    
    /* User struct */
    struct user_struct *user;
    
    /* Union */
    union my_union data;
    
    /* Pointer */
    struct master_struct *parent;
    
    /* Array */
    int numbers[20];
    
    /* Callback */
    simple_callback notify;
    
    /* Array of callbacks */
    complex_callback callbacks[4];
    
    /* Union with GTY type */
    union tagged_union tagged;
    
#ifdef GENERATOR_FILE
    /* Lang struct */
    struct lang_specific_struct *lang;
#endif
    
    /* Self-referential array */
    struct master_struct *children[5];
};

/* TYPE_UNDEFINED: Forward declaration that might not be defined */
struct undefined_struct;

/* Struct with pointer to undefined type */
struct GTY(()) has_undefined {
    struct undefined_struct *undef_ptr;
    void *opaque;
};

/* Additional pointer typedefs for TYPE_POINTER coverage */
typedef struct user_struct *user_ptr_t;
typedef struct complex_struct **double_ptr_t;
typedef int *int_ptr;

/* Array typedef */
typedef int int_array[50];

/* Struct with variable length array (using pointer) */
struct GTY(()) vla_container {
    int length;
    int *variable_array;  /* Treated as pointer, not array with known bounds */
};

#endif /* TEST_GENGTYPE_TYPES_H */
