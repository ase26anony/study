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
    /* TYPE_SCALAR */
    int scalar_member;
    
    /* TYPE_STRING */
    char *string_member;
    
    /* TYPE_POINTER */
    struct test_struct *next;
    
    /* TYPE_ARRAY */
    int array_member[10];
    
    /* TYPE_UNDEFINED (opaque pointer) */
    struct opaque *opaque_ptr;
};

/* TYPE_USER_STRUCT */
typedef struct test_struct GTY(()) user_struct_type;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_member;
    char *string_member;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_LANG_STRUCT - Typically used for language-specific structures */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
    int lang_data;
    struct lang_struct *next;
    struct lang_struct *prev;
};

/* Complex nested structure to ensure thorough processing */
struct GTY(()) container {
    /* TYPE_STRUCT */
    struct test_struct nested_struct;
    
    /* TYPE_UNION */
    union test_union nested_union;
    
    /* TYPE_POINTER to callback */
    callback_type callback_ptr;
    
    /* TYPE_ARRAY of pointers */
    struct test_struct *ptr_array[5];
    
    /* TYPE_ARRAY of scalars */
    int int_array[20];
    
    /* TYPE_STRING array */
    char *string_array[3];
};

/* Additional pointer types for coverage */
typedef struct test_struct *GTY(()) struct_ptr_type;
typedef union test_union *GTY(()) union_ptr_type;
typedef int *GTY(()) int_ptr_type;

#endif /* GTY_TEST_H */
