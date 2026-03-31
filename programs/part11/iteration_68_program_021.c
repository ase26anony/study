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

/* TYPE_POINTER (standalone typedef) */
typedef struct test_struct *GTY(()) struct_ptr_type;

/* TYPE_ARRAY (standalone typedef) */
typedef int GTY(()) int_array_type[20];

/* Nested structure for additional coverage */
struct GTY(()) outer_struct {
    /* TYPE_STRUCT nested */
    struct GTY(()) inner_struct {
        int x;
        char *name;
    } inner;
    
    /* TYPE_UNION nested */
    union GTY(()) inner_union {
        int a;
        float b;
    } data;
    
    /* TYPE_CALLBACK field */
    callback_type callback;
    
    /* TYPE_ARRAY of pointers */
    struct test_struct *GTY(()) ptr_array[5];
};

/* TYPE_LANG_STRUCT simulation */
#ifdef LANGUAGE_HOOKS
/* This would normally be in language-specific headers */
struct GTY(()) lang_type_struct {
    int lang_specific;
    void *lang_data;
};
#endif

#endif /* GTY_TEST_H */
