/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR: plain integer */
/* TYPE_STRING: char pointer */
/* TYPE_POINTER: pointer to struct */
/* TYPE_ARRAY: fixed-size array */
struct GTY(()) test_struct {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct opaque *opaque_ptr;     /* TYPE_POINTER to undefined type */
    int array_field[10];           /* TYPE_ARRAY */
    struct test_struct *next;      /* TYPE_POINTER to defined struct */
};

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    char *str_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK: function pointer typedef */
typedef void (*GTY((callback)) test_callback)(int, char *);

/* Another struct using the callback type */
struct GTY(()) callback_container {
    test_callback cb;              /* TYPE_CALLBACK */
    int id;
};

/* TYPE_USER_STRUCT: struct without GTY but referenced by GTY struct */
struct user_struct {
    int x;
    double y;
};

/* Struct containing a user_struct */
struct GTY(()) container {
    struct user_struct *user;      /* TYPE_USER_STRUCT pointer */
    int count;
};

/* TYPE_LANG_STRUCT: language-specific structure */
/* This requires special handling - typically marked with GTY((user)) */
struct GTY((user)) lang_specific {
    void *data;
    int lang_tag;
};

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int value;
        char *name;
    } inner;
    
    union GTY(()) inner_union {
        int a;
        float b;
    } u;
};

#endif /* GTY_TEST_H */
