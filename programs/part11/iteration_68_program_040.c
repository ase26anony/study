/* gty-test.h - Test file for gengtype type categorization */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED test */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
typedef int GTY(()) scalar_t;

/* TYPE_STRING: string type */
typedef char *GTY(()) string_t;

/* TYPE_CALLBACK: function pointer type */
typedef void (*GTY(()) callback_t)(int, void*);

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
    callback_t callback_field;
    
    /* TYPE_UNDEFINED (opaque pointer) */
    struct opaque *opaque_ptr;
};

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int id;
    char *name;
} user_struct_t;

/* Another struct with nested pointer for additional coverage */
struct GTY(()) container {
    /* TYPE_POINTER to struct */
    struct test_struct *struct_ptr;
    
    /* TYPE_POINTER to union */
    union test_union *union_ptr;
    
    /* TYPE_ARRAY of pointers */
    struct test_struct *ptr_array[5];
    
    /* TYPE_ARRAY of scalars */
    int int_array[20];
    
    /* TYPE_STRING array */
    char *string_array[3];
};

/* TYPE_LANG_STRUCT: Simulating GCC language-specific structure */
#ifdef IN_GCC
struct GTY(()) lang_type {
    int lang_specific;
    void *lang_data;
};
#endif

/* Additional pointer types for coverage */
typedef struct test_struct *GTY(()) struct_ptr_t;
typedef union test_union *GTY(()) union_ptr_t;
typedef int (*GTY(()) compare_func_t)(const void*, const void*);

#endif /* GTY_TEST_H */
