/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR - plain integer */
/* TYPE_STRING - char pointer */
/* TYPE_POINTER - pointer to another struct */
/* TYPE_ARRAY - fixed-size array */
struct GTY(()) test_struct {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct test_struct *next;      /* TYPE_POINTER */
    int array_field[10];           /* TYPE_ARRAY */
    struct opaque *opaque_ptr;     /* TYPE_POINTER to undefined type */
};

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    char *str_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK - function pointer typedef */
typedef void (*GTY((callback)) test_callback)(int, char *);

/* Another struct using the callback type */
struct GTY(()) callback_container {
    test_callback cb;              /* TYPE_CALLBACK */
    int id;
};

/* TYPE_USER_STRUCT - struct with user-defined GC markers */
struct GTY((user)) user_struct {
    int data;
    void *user_data;
};

/* Nested structures for comprehensive testing */
struct GTY(()) outer_struct {
    struct test_struct inner;      /* TYPE_STRUCT */
    union test_union choice;       /* TYPE_UNION */
    struct callback_container *containers;  /* TYPE_POINTER to array */
    int count;
};

/* Array of pointers */
typedef struct test_struct *GTY(()) ptr_array[5];

/* Self-referential structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;   /* TYPE_POINTER with skip */
    struct tree_node *GTY((skip)) right;  /* TYPE_POINTER with skip */
};

#endif /* GTY_TEST_H */
