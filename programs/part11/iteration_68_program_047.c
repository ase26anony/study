/* gty-test.h - Header with diverse GTY-annotated types for gengtype coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque type */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
/* TYPE_STRING: string type */
/* TYPE_POINTER: pointer type */
/* TYPE_ARRAY: array type */
struct GTY(()) scalar_and_pointer_types {
    int scalar_field;                    /* TYPE_SCALAR */
    char *string_field;                  /* TYPE_STRING */
    struct opaque *opaque_ptr;           /* TYPE_POINTER to undefined type */
    int *int_ptr;                        /* TYPE_POINTER */
    int array_field[10];                 /* TYPE_ARRAY */
    const char *const_string;            /* TYPE_STRING with const */
};

/* TYPE_STRUCT: regular GTY-annotated struct */
struct GTY(()) my_struct {
    int id;
    char *name;
    struct my_struct *next;              /* TYPE_POINTER within struct */
};

/* TYPE_USER_STRUCT: struct with user-defined GC markers */
struct GTY((user)) user_struct {
    int data;
    void *user_data;
};

/* TYPE_UNION: GTY-annotated union */
union GTY(()) my_union {
    int int_val;
    char *str_val;
    double dbl_val;
    struct my_struct *struct_ptr;        /* TYPE_POINTER within union */
};

/* TYPE_CALLBACK: function pointer typedef */
typedef void (*GTY((callback)) callback_func)(int, const char*);

/* Struct using callback type */
struct GTY(()) struct_with_callback {
    int id;
    callback_func handler;               /* TYPE_CALLBACK */
    void (*direct_func_ptr)(void);       /* Another function pointer */
};

/* Nested structures for complex type relationships */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_data;
        struct inner_struct *self_ptr;   /* TYPE_POINTER to self */
    } *inner;                           /* TYPE_POINTER to inner struct */
    
    union GTY(()) inner_union {
        int a;
        char *b;
    } union_field;                      /* TYPE_UNION field */
    
    struct scalar_and_pointer_types mixed; /* TYPE_STRUCT field */
};

/* Array of pointers */
struct GTY(()) array_container {
    struct my_struct *ptr_array[5];      /* TYPE_ARRAY of TYPE_POINTER */
    int int_array[20];                   /* TYPE_ARRAY of TYPE_SCALAR */
};

/* For TYPE_LANG_STRUCT - typically used in language-specific frontends */
/* This requires special handling in gengtype */
#ifdef TEST_LANG_STRUCT
struct GTY(()) lang_specific_struct {
    int lang_data;
    void *lang_private;
};
#endif

/* Global variables with GTY annotations */
extern struct my_struct * GTY((tag("MY_STRUCT_TAG"))) global_struct_ptr;
extern union my_union GTY((skip)) global_union_var;

#endif /* GTY_TEST_H */
