/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque pointer */
struct opaque;

/* TYPE_SCALAR: Plain scalar type */
/* TYPE_STRING: String type */
/* TYPE_POINTER: Pointer type */
/* TYPE_ARRAY: Array type */
/* TYPE_STRUCT: Struct type */
struct GTY(()) test_struct {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct opaque *opaque_ptr;     /* TYPE_POINTER to undefined type */
    int array_field[10];           /* TYPE_ARRAY */
    struct test_struct *next;      /* TYPE_POINTER to defined struct */
};

/* TYPE_UNION: Union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void GTY((callback)) (*test_callback)(int, char *);

/* Another struct using the callback type */
struct GTY(()) struct_with_callback {
    test_callback cb;              /* TYPE_CALLBACK */
    union test_union data;         /* TYPE_UNION */
};

/* TYPE_USER_STRUCT: User-defined struct (via typedef) */
typedef struct GTY(()) {
    int id;
    char *name;
} user_struct_t;

/* Chain of structures for complex testing */
struct GTY(()) chain_node {
    int value;
    struct chain_node * GTY((skip)) next_skip;  /* Skipped pointer */
    struct chain_node *next;                    /* Regular pointer */
    union test_union data;
};

/* Array of pointers */
struct GTY(()) pointer_array {
    struct test_struct * GTY((length("count"))) ptrs[];
    int count;
};

/* Nested structures */
struct GTY(()) outer_struct {
    struct GTY((tag("inner_tag"))) {
        int x;
        int y;
    } inner;
    user_struct_t user;
};

#endif /* GTY_TEST_H */
