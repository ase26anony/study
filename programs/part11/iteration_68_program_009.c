/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;

/* TYPE_SCALAR: Plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: String type */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT: Regular struct */
struct GTY(()) test_struct {
    /* TYPE_SCALAR */
    int scalar_field;
    
    /* TYPE_STRING */
    char *string_field;
    
    /* TYPE_POINTER */
    struct opaque *opaque_ptr;
    
    /* TYPE_ARRAY */
    int array_field[10];
    
    /* Nested TYPE_POINTER within struct */
    struct test_struct *next;
};

/* TYPE_UNION: Union type */
union GTY(()) test_union {
    int int_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_USER_STRUCT: Typedef of struct */
typedef struct GTY(()) user_struct {
    int data;
    struct user_struct *link;
} user_struct_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
    int lang_specific;
    void *lang_data;
};

/* Additional pointer types for coverage */
typedef struct test_struct *GTY(()) struct_ptr_type;
typedef union test_union *GTY(()) union_ptr_type;
typedef int (*GTY(()) another_callback)(void);

/* Array of pointers */
struct GTY(()) array_container {
    /* TYPE_ARRAY of TYPE_POINTER */
    struct test_struct *ptr_array[5];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* Complex nested structure */
struct GTY(()) complex_struct {
    /* TYPE_CALLBACK field */
    callback_type handler;
    
    /* TYPE_UNION field */
    union test_union data_union;
    
    /* TYPE_ARRAY of TYPE_UNION */
    union test_union union_array[4];
    
    /* TYPE_POINTER to TYPE_CALLBACK */
    callback_type *callback_ptr;
};

#endif /* GTY_TEST_H */
