/* gengtype_test_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef GT_TEST_TYPES_H
#define GT_TEST_TYPES_H

/* Include gtype.h for GTY macro if not in GCC tree context */
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
    int id;
    struct plain_struct *plain_ptr;  /* Pointer to non-GTY struct */
    struct user_struct *next;        /* Recursive pointer */
};

/* Another GTY-tagged struct for complex relationships */
struct GTY(()) complex_struct {
    struct user_struct *owner;
    int data[5];  /* TYPE_ARRAY */
};

/* TYPE_UNION: Union definitions */
union my_union {
    int int_val;
    double double_val;
    void *ptr_val;
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    struct user_struct *user_ptr;
    struct complex_struct *complex_ptr;
    string_t str;
};

/* TYPE_POINTER: Various pointer types */
typedef struct user_struct *user_ptr_t;
typedef struct complex_struct *complex_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_ARRAY: Array types within structs */
struct GTY(()) array_container {
    int fixed_array[10];           /* Fixed-size array */
    struct user_struct *ptr_array[5]; /* Array of pointers */
    int multi_dim[3][4];           /* Multi-dimensional array */
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct user_struct*, string_t);
typedef void (*void_callback)(void);

/* Callback within a GTY-tagged struct */
struct GTY(()) callback_container {
    simple_callback cb1;
    complex_callback cb2;
    void (*inline_cb)(double);  /* Inline function pointer declaration */
};

/* TYPE_LANG_STRUCT: Language-specific structs */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *gen_ptr;
};
#endif

/* Conditional for other language contexts */
#ifdef LANG_HOOKS
struct GTY(()) lang_hooks_struct {
    int lang_specific_data;
};
#endif

/* TYPE_UNDEFINED: Forward declarations that might be undefined */
struct forward_declared;  /* This will be TYPE_UNDEFINED initially */

/* Later definition to resolve */
struct GTY(()) forward_declared {
    int value;
    struct forward_declared *next;
};

/* Complex nested example with all types */
struct GTY(()) master_container {
    /* Scalar fields */
    my_int scalar1;
    my_double scalar2;
    
    /* String field */
    string_t name;
    
    /* Struct fields */
    struct plain_struct plain;
    struct user_struct *user;
    
    /* Union field */
    union my_union data_union;
    
    /* Pointer fields */
    user_ptr_t user_ptr;
    generic_ptr_t generic_ptr;
    
    /* Array fields */
    int scores[20];
    struct user_struct *users[10];
    
    /* Callback field */
    simple_callback notify;
    
    /* Nested struct with GTY */
    struct GTY(()) nested {
        int level;
        struct master_container *parent;
    } nested_data;
    
    /* Union with GTY */
    union GTY(()) choice {
        int as_int;
        string_t as_string;
        struct user_struct *as_user;
    } choice_data;
};

/* Additional pointer typedefs for coverage */
typedef struct master_container *master_ptr_t;
typedef union tagged_union *tagged_union_ptr_t;

/* Array of callbacks */
typedef simple_callback callback_array_t[5];

/* Struct containing array of callbacks */
struct GTY(()) callback_array_container {
    callback_array_t callbacks;
    int count;
};

/* Self-referential structure for deep traversal */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Mixed struct with conditional fields */
struct GTY(()) conditional_struct {
    int always_present;
#ifdef SPECIAL_FEATURE
    int special_feature_data;
    struct user_struct *special_ptr;
#endif
    string_t conditional_string;
};

#endif /* GT_TEST_TYPES_H */
