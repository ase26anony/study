/* gty-test.h - Test various GTY-annotated types for gengtype coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;

/* TYPE_SCALAR: Plain scalar type */
/* TYPE_STRING: String type */
/* TYPE_POINTER: Pointer type */
/* TYPE_ARRAY: Array type */
struct GTY(()) test_struct {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct opaque *opaque_ptr;     /* TYPE_POINTER to undefined type */
    int array_field[10];           /* TYPE_ARRAY */
    struct test_struct *next;      /* TYPE_POINTER to defined struct */
};

/* TYPE_STRUCT: Regular struct */
struct GTY(()) regular_struct {
    int id;
    char *name;
};

/* TYPE_USER_STRUCT: User-defined struct (via typedef) */
typedef struct GTY(()) {
    int x;
    int y;
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;
    struct regular_struct *struct_ptr;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY((callback)) callback_func)(int, char *);

/* Structure containing callback */
struct GTY(()) with_callback {
    callback_func handler;         /* TYPE_CALLBACK */
    int data;
};

/* Nested structure for complex testing */
struct GTY(()) container {
    struct regular_struct item1;   /* TYPE_STRUCT */
    user_struct_t item2;           /* TYPE_USER_STRUCT */
    union test_union item3;        /* TYPE_UNION */
    struct with_callback item4;    /* TYPE_STRUCT with callback */
    struct container *children[5]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* This typically requires special handling in GCC frontends */
struct GTY((desc("%0.lang_code"))) lang_struct {
    int lang_code;
    void * GTY((skip)) lang_data;
    struct lang_struct *next;
};

/* Additional pointer types for coverage */
typedef struct regular_struct* GTY((ptr)) regular_ptr_t;
typedef union test_union* GTY((ptr)) union_ptr_t;

/* Array of pointers */
typedef struct GTY(()) ptr_array {
    regular_ptr_t items[20];       /* TYPE_ARRAY of TYPE_POINTER */
    int count;
} ptr_array_t;

#endif /* GTY_TEST_H */
