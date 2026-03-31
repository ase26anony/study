/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
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
    
    /* Scalar fields */
    my_int count;
    my_double weight;
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
    int tag;
    struct user_struct *user;
    struct complex_struct *complex;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct user_struct *, my_int);

/* GTY-tagged struct with callback field */
struct GTY(()) struct_with_callback {
    simple_callback cb;
    complex_callback complex_cb;
    
    /* Union field */
    union my_union data;
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
    struct lang_specific_struct *lang_struct;
#endif
};
#endif

/* Array of structs */
struct GTY(()) array_container {
    struct user_struct users[3];
    struct complex_struct *ptr_array[8];
};

/* Nested anonymous struct/unions */
struct GTY(()) nested_anonymous {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } data;
};

/* Forward declarations to test TYPE_UNDEFINED handling */
struct forward_declared;
union another_forward;

/* Complete the forward declarations */
struct forward_declared {
    int completed;
};

union another_forward {
    int a;
    char b;
};

/* Typedef struct */
typedef struct GTY(()) {
    int anonymous;
} anonymous_struct_t;

/* Multiple indirection pointers */
struct GTY(()) multi_ptr {
    struct user_struct **double_ptr;
    struct complex_struct ***triple_ptr;
};

/* Mixed declarations in single struct */
struct GTY(()) mixed_types {
    /* Scalars */
    my_int id;
    my_double score;
    
    /* Strings */
    string_t description;
    char buffer[256];
    
    /* Pointers */
    struct mixed_types *next;
    void *generic_ptr;
    
    /* Arrays */
    int matrix[3][3];
    struct user_struct *obj_array[4];
    
    /* Union */
    union my_union choice;
    
    /* Callback */
    simple_callback handler;
    
    /* Bitfields */
    unsigned int flags : 4;
    unsigned int status : 2;
};

#endif /* TEST_GENGTYPE_TYPES_H */
