/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque type */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: string type */
typedef const char * GTY(()) string_type;

/* TYPE_STRUCT: basic struct */
struct GTY(()) basic_struct {
    int id;                    /* TYPE_SCALAR */
    const char *name;          /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int value;
    struct basic_struct *link; /* TYPE_POINTER inside struct */
} user_struct;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int as_int;
    float as_float;
    void *as_ptr;              /* TYPE_POINTER inside union */
};

/* TYPE_CALLBACK: function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);

/* TYPE_ARRAY: array type */
struct GTY(()) array_container {
    int numbers[10];           /* TYPE_ARRAY */
    struct basic_struct *items[5]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_POINTER: pointer types in various contexts */
struct GTY(()) pointer_test {
    struct basic_struct *direct;      /* TYPE_POINTER to TYPE_STRUCT */
    struct opaque *opaque_ptr;        /* TYPE_POINTER to TYPE_UNDEFINED */
    union test_union *union_ptr;      /* TYPE_POINTER to TYPE_UNION */
    callback_func handler;            /* TYPE_CALLBACK */
    void (*other_handler)(void);      /* Another TYPE_CALLBACK */
};

/* Nested structure for complex testing */
struct GTY(()) outer_struct {
    struct pointer_test *ptrs;        /* TYPE_POINTER */
    union test_union data;            /* TYPE_UNION */
    callback_func callbacks[3];       /* TYPE_ARRAY of TYPE_CALLBACK */
    struct {
        int nested_scalar;
        char *nested_string;          /* TYPE_STRING */
    } GTY(()) nested;                 /* TYPE_STRUCT (anonymous) */
};

/* TYPE_LANG_STRUCT: Typically used for language-specific structures */
/* In GCC, these are often marked with GTY(()) and special handling */
struct GTY(()) lang_specific {
    int lang_tag;
    void *lang_data;                  /* TYPE_POINTER */
};

/* Global variables with GTY annotations */
extern struct basic_struct GTY(()) *global_ptr;
extern user_struct GTY(()) global_user_struct;
extern callback_func GTY(()) global_callback;

#endif /* GTY_TEST_H */
