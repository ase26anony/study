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
struct GTY(()) my_struct {
    int scalar_field;           /* TYPE_SCALAR */
    char *string_field;         /* TYPE_STRING */
    struct opaque *opaque_ptr;  /* TYPE_POINTER to undefined type */
    int (*callback)(void);      /* TYPE_CALLBACK */
    int array_field[10];        /* TYPE_ARRAY */
};

/* TYPE_USER_STRUCT */
typedef struct GTY(()) my_struct my_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
    int int_val;
    char *str_val;
    struct my_struct *struct_ptr;
};

/* TYPE_POINTER */
typedef struct my_struct *GTY(()) struct_ptr_t;

/* TYPE_ARRAY */
typedef int GTY(()) int_array_t[20];

/* TYPE_CALLBACK */
typedef int GTY(()) (*callback_t)(int, char *);

/* TYPE_LANG_STRUCT - Simulated with special tag */
struct GTY((tag("LANG"))) lang_struct {
    int lang_specific;
};

/* Additional complex types for thorough testing */
struct GTY(()) container {
    union my_union u;           /* TYPE_UNION */
    callback_t cb;              /* TYPE_CALLBACK */
    int_array_t arr;            /* TYPE_ARRAY */
    struct lang_struct *lang;   /* TYPE_POINTER to lang struct */
};

/* Nested pointer/array combinations */
struct GTY(()) complex_type {
    struct my_struct *GTY((skip)) ptr_array[5];  /* Array of pointers */
    callback_t (*func_ptr_array[3])(void);       /* Array of function pointers */
};

#endif /* GTY_TEST_H */
