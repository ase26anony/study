/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED test */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: string type */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT: basic struct */
struct GTY(()) basic_struct {
    int GTY(()) scalar_field;      /* TYPE_SCALAR */
    char *GTY(()) string_field;    /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int GTY(()) data;
} user_struct_type;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int GTY(()) int_val;
    char *GTY(()) str_val;
    struct basic_struct *GTY(()) struct_ptr;
};

/* TYPE_CALLBACK: function pointer */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_POINTER: pointer type within struct */
struct GTY(()) pointer_container {
    struct basic_struct *GTY(()) ptr_field;    /* TYPE_POINTER */
    union test_union *GTY(()) union_ptr;       /* TYPE_POINTER */
};

/* TYPE_ARRAY: array type */
struct GTY(()) array_container {
    int GTY(()) int_array[10];                 /* TYPE_ARRAY */
    struct basic_struct GTY(()) struct_array[5]; /* TYPE_ARRAY */
    char *GTY(()) string_array[3];             /* TYPE_ARRAY (of TYPE_STRING) */
};

/* TYPE_LANG_STRUCT: language-specific struct */
struct GTY((tag("LANG"))) lang_struct {
    int GTY(()) lang_data;
    callback_type GTY(()) handler;             /* TYPE_CALLBACK */
};

/* Complex nested structure to test multiple classifications */
struct GTY(()) complex_type {
    /* TYPE_UNDEFINED: opaque pointer */
    struct opaque *GTY(()) opaque_ptr;
    
    /* TYPE_POINTER: various pointers */
    struct basic_struct *GTY(()) struct_ptr;
    union test_union *GTY(()) union_ptr;
    callback_type *GTY(()) callback_ptr;
    
    /* TYPE_ARRAY: arrays */
    int GTY(()) matrix[3][3];
    callback_type GTY(()) handlers[5];
    
    /* TYPE_STRING */
    char *GTY(()) name;
    
    /* TYPE_SCALAR */
    int GTY(()) count;
    float GTY(()) value;
    
    /* Nested TYPE_STRUCT */
    struct basic_struct GTY(()) nested;
    
    /* Nested TYPE_UNION */
    union test_union GTY(()) choice;
};

/* Additional test for TYPE_CALLBACK in different contexts */
typedef struct GTY(()) callback_container {
    callback_type GTY(()) cb1;
    void (*GTY(()) cb2)(struct basic_struct *);
    int (*GTY(()) cb3)(union test_union *, char **);
} callback_container;

#endif /* GTY_TEST_H */
