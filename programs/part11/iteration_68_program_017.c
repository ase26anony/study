/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR */
typedef int GTY(()) scalar_type;

/* TYPE_STRING */
typedef const char * GTY(()) string_type;

/* TYPE_STRUCT */
struct GTY(()) my_struct {
    int scalar_field;          /* TYPE_SCALAR */
    char *string_field;        /* TYPE_STRING */
    struct opaque *opaque_ptr; /* May influence TYPE_UNDEFINED */
};

/* TYPE_USER_STRUCT - A typedef'd struct */
typedef struct GTY(()) {
    int x;
    double y;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
    int int_val;
    double double_val;
    void *ptr_val;
};

/* TYPE_POINTER - Explicit pointer type */
typedef struct my_struct * GTY(()) struct_pointer;

/* TYPE_ARRAY - Array type */
typedef int GTY(()) int_array[10];

/* TYPE_CALLBACK - Function pointer */
typedef void (* GTY(()) callback_func)(int, const char*);

/* TYPE_LANG_STRUCT - Typically used for language-specific structures */
struct GTY((tag("LANG"))) lang_struct {
    int lang_specific;
    void *lang_data;
};

/* Nested structures to ensure thorough processing */
struct GTY(()) container {
    /* TYPE_POINTER within struct */
    struct my_struct *nested_ptr;
    
    /* TYPE_ARRAY within struct */
    int GTY(()) nested_array[5];
    
    /* TYPE_CALLBACK within struct */
    callback_func handler;
    
    /* TYPE_UNION within struct */
    union my_union data;
    
    /* TYPE_USER_STRUCT within struct */
    user_struct_t user;
    
    /* TYPE_LANG_STRUCT within struct */
    struct lang_struct lang;
};

/* Another structure with all types */
struct GTY(()) all_types {
    /* Basic types */
    int scalar;
    char *string;
    
    /* Composite types */
    struct my_struct struct_field;
    union my_union union_field;
    
    /* Pointer types */
    int *int_ptr;
    struct container *container_ptr;
    
    /* Array types */
    double GTY(()) double_array[20];
    
    /* Callback */
    callback_func callbacks[3];
};

/* Template for generating multiple instances */
#define DECLARE_STRUCT(name) \
    struct GTY(()) name##_struct { \
        int id; \
        struct name##_struct *next; \
    };

DECLARE_STRUCT(list_node)
DECLARE_STRUCT(tree_node)

/* Chain of structures for pointer chasing */
struct GTY(()) chain {
    struct chain * GTY((skip)) next;  /* Skip annotation for testing */
    struct chain *prev;
    int data;
};

#endif /* GTY_TEST_H */
