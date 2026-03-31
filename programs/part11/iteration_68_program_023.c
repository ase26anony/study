/* gty-test.h - Test file for gengtype type classification coverage */

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
    /* TYPE_SCALAR */
    int scalar_field;
    
    /* TYPE_STRING */
    char *string_field;
    
    /* TYPE_POINTER */
    struct test_struct *next;
    
    /* TYPE_ARRAY */
    int array_field[10];
    
    /* TYPE_UNDEFINED (opaque pointer) */
    struct opaque *opaque_ptr;
};

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_USER_STRUCT */
typedef struct test_struct GTY(()) user_struct_type;

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, char *);

/* Another struct with callback */
struct GTY(()) struct_with_callback {
    callback_type callback;
    int data;
};

/* TYPE_ARRAY in typedef */
typedef int GTY(()) int_array[20];

/* Nested structures for comprehensive testing */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int value;
        char *name;
    } inner;
    
    union GTY(()) inner_union {
        int ival;
        float fval;
    } u;
    
    /* Array of pointers */
    struct inner_struct *GTY(()) ptr_array[5];
    
    /* Pointer to array */
    int (*GTY(()) array_ptr)[10];
};

/* For TYPE_LANG_STRUCT simulation */
#ifdef LANGUAGE_HOOKS
/* This would normally be defined in language-specific headers */
struct GTY(()) lang_type_struct {
    int lang_specific;
};
#endif

#endif /* GTY_TEST_H */
