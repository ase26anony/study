/* gty-test.h - Test various GTY-annotated types for gengtype coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED test */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
/* TYPE_STRING: string type */
/* TYPE_POINTER: pointer type */
/* TYPE_ARRAY: array type */
struct GTY(()) test_struct {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct test_struct *next;      /* TYPE_POINTER */
    int array_field[10];           /* TYPE_ARRAY */
    struct opaque *opaque_ptr;     /* TYPE_UNDEFINED (forward declared) */
};

/* TYPE_STRUCT: regular struct */
struct GTY(()) another_struct {
    int id;
    struct test_struct *link;
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) user_struct_type {
    int data;
    struct user_struct_type *next;
} user_struct_t;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK: function pointer */
typedef void GTY((callback)) (*callback_func)(int, char *);

/* Struct using callback type */
struct GTY(()) callback_container {
    callback_func handler;
    int state;
};

/* TYPE_LANG_STRUCT: language-specific structure */
/* This typically requires special handling in gengtype */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct {
    int lang_specific;
};
#endif

/* Nested structures for complex testing */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_data;
    } inner;
    
    union GTY(()) inner_union {
        int a;
        double b;
    } u;
    
    struct outer_struct *self_ptr;
};

/* Array of pointers */
typedef struct GTY(()) node {
    int value;
    struct node *GTY((length("%h.count"))) children[];
} node_t;

/* Test case for parameterized types */
struct GTY(()) param_struct {
    int count;
    node_t *root;  /* Variable length array in node_t */
};

#endif /* GTY_TEST_H */
