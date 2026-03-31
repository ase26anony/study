/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
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
    struct opaque *opaque_ptr;
};

/* TYPE_USER_STRUCT */
typedef struct GTY(()) {
    int x;
    int y;
} user_struct_type;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_POINTER in typedef */
typedef struct test_struct *GTY(()) struct_ptr_type;

/* TYPE_ARRAY in typedef */
typedef int GTY(()) int_array_type[5];

/* Nested structures for complex testing */
struct GTY(()) outer_struct {
    /* TYPE_STRUCT */
    struct GTY(()) inner_struct {
        int data;
        struct inner_struct *GTY((skip)) sibling;
    } inner;
    
    /* TYPE_UNION */
    union GTY(()) inner_union {
        int i;
        float f;
    } u;
    
    /* TYPE_CALLBACK field */
    callback_type callback_field;
    
    /* TYPE_ARRAY of pointers */
    struct test_struct *GTY(()) ptr_array[8];
};

/* For TYPE_LANG_STRUCT simulation (GCC internal) */
#ifdef IN_GCC
struct GTY(()) lang_type {
    int lang_specific;
};
#endif

/* Template for GCC's internal use */
struct GTY((user)) user_defined {
    int tag;
    union GTY((desc ("%0.tag"))) {
        int GTY((tag ("0"))) i;
        char *GTY((tag ("1"))) s;
    } u;
};

#endif /* GTY_TEST_H */
