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
    struct test_struct *link;
};

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;
    struct test_struct *struct_ptr;
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int user_id;
    char *user_name;
} user_struct_t;

/* TYPE_CALLBACK: function pointer type */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* TYPE_LANG_STRUCT: language-specific structure */
/* This requires special handling - often marked with GTY for language frontends */
struct GTY(()) lang_struct {
    int lang_specific;
    void *lang_data;
};

/* Additional test for nested structures */
struct GTY(()) container {
    struct test_struct nested_struct;      /* TYPE_STRUCT */
    union test_union nested_union;         /* TYPE_UNION */
    user_struct_t nested_user_struct;      /* TYPE_USER_STRUCT */
    callback_func callback_field;          /* TYPE_CALLBACK */
    struct lang_struct *lang_ptr;          /* TYPE_POINTER to lang struct */
    
    /* Test array of pointers */
    struct test_struct *ptr_array[5];      /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Test pointer to array */
    int (*array_ptr)[10];                  /* TYPE_POINTER to TYPE_ARRAY */
};

/* Test for multiple levels of indirection */
struct GTY(()) complex_types {
    /* Pointer to pointer */
    struct test_struct **double_ptr;
    
    /* Array of pointers to function pointers */
    callback_func (*func_ptr_array[3])(int);
    
    /* Flexible array member */
    int flexible_array[];
};

#endif /* GTY_TEST_H */
