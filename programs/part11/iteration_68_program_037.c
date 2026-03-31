/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque pointer */
struct opaque;

/* TYPE_SCALAR: plain int */
/* TYPE_STRING: char * */
/* TYPE_POINTER: struct opaque * (also helps with undefined) */
/* TYPE_ARRAY: int array[10] */
struct GTY(()) test_struct {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct opaque *opaque_ptr;     /* TYPE_POINTER + potentially TYPE_UNDEFINED */
    int array_field[10];           /* TYPE_ARRAY */
};

/* TYPE_STRUCT: regular struct */
struct GTY(()) my_struct {
    int x;
    struct test_struct *next;      /* TYPE_POINTER within struct */
};

/* TYPE_UNION: union type */
union GTY(()) my_union {
    int i;
    char *str;                     /* TYPE_STRING within union */
    struct my_struct *s;           /* TYPE_POINTER within union */
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int id;
    char name[50];
} user_struct_t;

/* TYPE_LANG_STRUCT: language-specific structure */
/* This typically requires special handling in gengtype */
struct GTY((user)) lang_specific {
    void *data;
};

/* TYPE_CALLBACK: function pointer typedef */
typedef void (*GTY((callback)) callback_func)(int, void*);

/* Another struct using the callback type */
struct GTY(()) has_callback {
    callback_func cb;              /* TYPE_CALLBACK */
    user_struct_t user;            /* TYPE_USER_STRUCT */
};

/* Nested structures for comprehensive testing */
struct GTY(()) container {
    struct my_struct nested_struct;    /* TYPE_STRUCT */
    union my_union nested_union;       /* TYPE_UNION */
    struct has_callback *cb_container; /* TYPE_POINTER */
};

/* Array of pointers */
struct GTY(()) pointer_array {
    struct my_struct * GTY((length("count"))) ptrs[];
    int count;
};

/* Union with struct */
union GTY(()) mixed_union {
    struct my_struct s;
    user_struct_t u;
    int arr[5];
};

#endif /* GTY_TEST_H */
