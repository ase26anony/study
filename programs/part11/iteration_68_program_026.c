/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
/* TYPE_STRING: string type */
/* TYPE_POINTER: pointer type */
/* TYPE_ARRAY: array type */
struct GTY(()) test_struct {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct opaque *opaque_ptr;     /* TYPE_POINTER to undefined type */
    int array_field[10];           /* TYPE_ARRAY */
    struct test_struct *next;      /* TYPE_POINTER to defined struct */
};

/* TYPE_STRUCT: regular struct */
struct GTY(()) another_struct {
    int id;
    struct test_struct *data;
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int x;
    int y;
} point_t;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK: function pointer type */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Structure containing callback */
struct GTY(()) with_callback {
    callback_func handler;
    int state;
};

/* TYPE_LANG_STRUCT: language-specific structure */
/* This requires special handling - typically for frontend structures */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct {
    int lang_specific;
};
#endif

/* Nested structures for complex testing */
struct GTY(()) container {
    struct test_struct items[5];   /* TYPE_ARRAY of structs */
    union test_union variant;      /* TYPE_UNION */
    point_t points[3];             /* TYPE_ARRAY of user structs */
};

/* For TYPE_NONE - this should not be reachable in normal operation */
/* gengtype should handle this with gcc_unreachable() */

#endif /* GTY_TEST_H */
