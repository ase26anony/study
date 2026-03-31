/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque pointer */
struct opaque;

/* TYPE_SCALAR: plain int */
/* TYPE_STRING: char* */
/* TYPE_POINTER: struct opaque* pointer */
/* TYPE_ARRAY: int array */
struct GTY(()) test_struct {
    int scalar_field;           /* TYPE_SCALAR */
    char *string_field;         /* TYPE_STRING */
    struct opaque *opaque_ptr;  /* TYPE_POINTER (to undefined type) */
    int array_field[10];        /* TYPE_ARRAY */
};

/* TYPE_STRUCT: regular struct */
struct GTY(()) regular_struct {
    int x;
    struct test_struct *next;   /* Nested TYPE_POINTER */
};

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;              /* TYPE_STRING within union */
    struct regular_struct *s;   /* TYPE_POINTER within union */
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int id;
    char name[50];              /* TYPE_ARRAY within typedef struct */
} user_struct_t;

/* TYPE_CALLBACK: function pointer typedef */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Struct containing callback */
struct GTY(()) callback_container {
    callback_func handler;      /* TYPE_CALLBACK */
    int priority;
};

/* Nested structures for complex testing */
struct GTY(()) outer_struct {
    struct regular_struct inner;    /* TYPE_STRUCT nested */
    union test_union data;          /* TYPE_UNION nested */
    user_struct_t users[5];         /* TYPE_ARRAY of user structs */
};

/* TYPE_LANG_STRUCT: Marked with special tag */
struct GTY((tag("LANG"))) lang_struct {
    int lang_specific;
    void *data;                     /* TYPE_POINTER (void*) */
};

/* Additional pointer types for coverage */
typedef struct regular_struct* GTY((ptr)) regular_ptr;
typedef union test_union* GTY((ptr)) union_ptr;

/* Array of pointers */
struct GTY(()) pointer_array_container {
    regular_ptr ptrs[8];            /* TYPE_ARRAY of TYPE_POINTER */
    union_ptr union_ptrs[4];        /* TYPE_ARRAY of TYPE_POINTER */
};

#endif /* GTY_TEST_H */
