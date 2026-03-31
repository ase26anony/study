/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: string type */
typedef char *GTY(()) string_type;

/* TYPE_CALLBACK: function pointer type */
typedef void (*callback_type)(void *data) GTY((callback));

/* TYPE_STRUCT: regular struct */
struct GTY(()) test_struct {
    /* TYPE_SCALAR */
    int scalar_field;
    
    /* TYPE_STRING */
    char *string_field;
    
    /* TYPE_POINTER */
    struct test_struct *next;
    
    /* TYPE_ARRAY */
    int array_field[10];
    
    /* TYPE_CALLBACK */
    callback_type callback;
    
    /* TYPE_UNDEFINED (opaque pointer) */
    struct opaque *opaque_ptr;
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int x;
    int y;
} user_struct_type;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int int_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_LANG_STRUCT: language-specific struct */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_struct {
    int lang_specific;
};
#endif

/* Additional pointer types for TYPE_POINTER coverage */
typedef struct test_struct *GTY(()) struct_ptr_type;
typedef union test_union *GTY(()) union_ptr_type;
typedef int *GTY(()) int_ptr_type;

/* Array typedef for TYPE_ARRAY */
typedef int GTY(()) int_array_type[5];

/* Complex nested structure for comprehensive coverage */
struct GTY(()) container {
    /* Direct TYPE_STRUCT */
    struct test_struct nested_struct;
    
    /* TYPE_UNION */
    union test_union nested_union;
    
    /* TYPE_USER_STRUCT */
    user_struct_type user_struct;
    
    /* TYPE_POINTER to various types */
    struct test_struct *struct_ptr;
    union test_union *union_ptr;
    callback_type *callback_ptr;
    
    /* TYPE_ARRAY of various types */
    struct test_struct *struct_array[3];
    callback_type callback_array[2];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* Enumeration type (should be treated as scalar) */
enum GTY(()) test_enum {
    ENUM_VAL1,
    ENUM_VAL2,
    ENUM_VAL3
};

#endif /* GTY_TEST_H */
