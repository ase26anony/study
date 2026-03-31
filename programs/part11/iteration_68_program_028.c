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
    
    /* Pointer to undefined type */
    struct opaque *undefined_ptr;
};

/* TYPE_USER_STRUCT */
typedef struct test_struct GTY(()) user_struct_type;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_POINTER (standalone) */
typedef struct test_struct *GTY(()) struct_pointer_type;

/* TYPE_ARRAY (standalone) */
typedef int GTY(()) int_array_type[5];

/* Nested structure for additional coverage */
struct GTY(()) outer_struct {
    /* TYPE_STRUCT within TYPE_STRUCT */
    struct GTY(()) inner_struct {
        int inner_field;
        char *inner_string;
    } inner;
    
    /* TYPE_UNION within TYPE_STRUCT */
    union GTY(()) inner_union {
        int a;
        float b;
    } u;
    
    /* TYPE_CALLBACK within TYPE_STRUCT */
    callback_type callback_field;
    
    /* TYPE_ARRAY of pointers */
    struct test_struct *GTY(()) ptr_array[8];
};

/* Language-specific structure (TYPE_LANG_STRUCT) */
struct GTY((user)) lang_specific_struct {
    int lang_field;
    void *lang_data;
};

/* Another callback with different signature */
typedef int (*GTY(()) compare_func)(const void *, const void *);

#endif /* GTY_TEST_H */
