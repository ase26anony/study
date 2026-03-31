/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
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
    string_t name;
    struct plain_struct *plain_ptr;  /* Pointer to non-GTY struct */
};

/* TYPE_UNION: Union definitions */
union my_union {
    int i;
    double d;
    void *p;
};

/* TYPE_POINTER: Pointer typedefs and pointer fields */
typedef struct user_struct *user_ptr_t;
typedef int *int_ptr_t;

/* TYPE_ARRAY: Array types within structs */
struct GTY(()) array_container {
    int fixed_array[10];
    int *dynamic_array GTY((length("dynamic_len")));
    int dynamic_len;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int, const char*);
typedef int (*compare_fn)(const void*, const void*);

/* TYPE_LANG_STRUCT: Language-specific structs */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    callback_fn generator_callback;
};
#endif

/* Complex nested/recursive type patterns */

/* Recursive GTY struct */
struct GTY(()) recursive_node {
    int value;
    struct recursive_node *next GTY((skip));
    struct recursive_node *prev;
};

/* Struct with multiple pointer types */
struct GTY(()) complex_struct {
    /* Scalar fields */
    my_int scalar1;
    my_double scalar2;
    
    /* String field */
    string_t description;
    
    /* Pointer fields */
    struct user_struct *user_ptr;
    int_ptr_t int_ptr;
    
    /* Array field */
    struct user_struct *ptr_array[5];
    
    /* Callback field */
    callback_fn notify;
    
    /* Union field */
    union my_union data;
    
    /* Nested struct */
    struct {
        int nested_id;
        char nested_name[20];
    } nested;
    
    /* Pointer to array */
    int (*matrix_ptr)[10];
};

/* Union containing GTY-tagged pointer */
union GTY(()) tagged_union {
    struct user_struct * GTY((tag("0"))) user;
    struct recursive_node * GTY((tag("1"))) node;
    int type_tag;
};

/* Typedef for callback used in GTY struct */
typedef void (*event_handler)(struct complex_struct*, int);

/* Another GTY struct using the callback typedef */
struct GTY(()) event_source {
    event_handler handler;
    struct complex_struct *context;
    callback_fn fallback;
};

/* Array of pointers in GTY struct */
struct GTY(()) pointer_array {
    struct user_struct *items[8];
    struct recursive_node **dynamic_items;
    int item_count;
};

/* Mixed struct with conditional compilation */
struct GTY(()) conditional_struct {
    int always_present;
#ifdef SPECIAL_FEATURE
    int special_feature_only;
    callback_fn special_callback;
#endif
    union my_union data;
};

/* Undefined type forward declaration (will be TYPE_UNDEFINED initially) */
struct undefined_struct;

/* Later definition to complete it */
struct undefined_struct {
    int defined_now;
    struct user_struct *link;
};

#endif /* TEST_GENGTYPE_TYPES_H */
