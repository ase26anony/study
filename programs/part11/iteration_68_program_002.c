/* gty-test.h - Test file for gengtype type categorization */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR */
typedef int GTY(()) scalar_type;

/* TYPE_STRING */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT */
struct GTY(()) test_struct {
    int scalar_field;          /* TYPE_SCALAR */
    char *string_field;        /* TYPE_STRING */
    struct opaque *opaque_ptr; /* May influence TYPE_UNDEFINED */
};

/* TYPE_USER_STRUCT - A typedef'd struct */
typedef struct GTY(()) user_struct {
    int data;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    char *str_val;
    void *ptr_val;
};

/* TYPE_POINTER */
typedef struct GTY(()) pointer_container {
    struct test_struct *struct_ptr;  /* TYPE_POINTER */
    union test_union *union_ptr;     /* TYPE_POINTER */
} pointer_container_t;

/* TYPE_ARRAY */
struct GTY(()) array_container {
    int scalar_array[10];            /* TYPE_ARRAY of TYPE_SCALAR */
    struct test_struct *ptr_array[5]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_LANG_STRUCT - Typically used for language-specific structures */
struct GTY(()) lang_struct {
    int lang_specific;
    callback_type callback;          /* TYPE_CALLBACK */
};

/* Nested structures to ensure comprehensive processing */
struct GTY(()) outer_struct {
    struct test_struct inner;        /* TYPE_STRUCT */
    union test_union inner_union;    /* TYPE_UNION */
    callback_type handlers[3];       /* TYPE_ARRAY of TYPE_CALLBACK */
    struct array_container *container; /* TYPE_POINTER */
};

/* Additional pointer types */
typedef struct test_struct *GTY(()) struct_ptr_type;
typedef union test_union *GTY(()) union_ptr_type;
typedef void (*GTY(()) another_callback)(struct opaque *);

#endif /* GTY_TEST_H */
