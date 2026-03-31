/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque type */
struct opaque;

/* TYPE_SCALAR: plain int */
/* TYPE_STRING: char* */
/* TYPE_POINTER: pointer to struct */
/* TYPE_ARRAY: fixed-size array */
struct GTY(()) test_struct {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct test_struct *next;      /* TYPE_POINTER */
    int array_field[10];           /* TYPE_ARRAY */
    struct opaque *opaque_ptr;     /* TYPE_POINTER to undefined type */
};

/* TYPE_STRUCT: regular struct */
struct GTY(()) another_struct {
    int x;
    struct test_struct *link;
};

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int as_int;
    char *as_string;
    struct test_struct *as_struct;
};

/* TYPE_CALLBACK: function pointer typedef */
typedef void GTY((callback)) (*callback_func)(int, char*);

/* Structure containing callback */
struct GTY(()) with_callback {
    callback_func handler;
    int data;
};

/* TYPE_USER_STRUCT: Use GTY marker with user-defined behavior */
struct user_base {
    int id;
};

struct GTY((user)) user_derived {
    struct user_base base;
    char *name;
};

/* Chain of structures for complex testing */
struct GTY(()) node {
    int value;
    struct node *GTY((skip)) children[5];  /* TYPE_ARRAY of pointers */
    struct node *parent;                   /* TYPE_POINTER */
};

/* Language-specific structure (simulating TYPE_LANG_STRUCT) */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_struct {
    int lang_specific;
};
#endif

/* Multiple levels of indirection */
struct GTY(()) complex_type {
    union test_union *union_ptr;          /* TYPE_POINTER to union */
    struct with_callback **cb_array[3];   /* TYPE_ARRAY of pointer-to-pointer */
};

#endif /* GTY_TEST_H */
