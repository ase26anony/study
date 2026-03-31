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
struct GTY(()) my_struct {
    int scalar_field;           /* TYPE_SCALAR */
    char *string_field;         /* TYPE_STRING */
    struct my_struct *next;     /* TYPE_POINTER */
    int array_field[10];        /* TYPE_ARRAY */
    struct opaque *opaque_ptr;  /* TYPE_UNDEFINED (forward declared) */
};

/* TYPE_USER_STRUCT - A typedef'd struct */
typedef struct GTY(()) {
    int x;
    int y;
} user_struct_type;

/* TYPE_UNION */
union GTY(()) my_union {
    int int_val;
    char *str_val;
    struct my_struct *struct_ptr;
};

/* TYPE_CALLBACK - Function pointer */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_LANG_STRUCT - Simulated language-specific structure */
#ifdef IN_LANG_SPECIFIC
struct GTY(()) lang_specific_struct {
    int lang_data;
    void (*lang_callback)(void);
};
#endif

/* Nested structures to ensure full processing */
struct GTY(()) container {
    /* TYPE_POINTER */
    struct my_struct *GTY((tag("0"))) struct_ptr;
    
    /* TYPE_ARRAY of pointers */
    struct my_struct *GTY((length("5"))) ptr_array[5];
    
    /* TYPE_ARRAY of scalars */
    int GTY((skip)) int_array[20];
    
    /* TYPE_CALLBACK field */
    callback_type handler;
    
    /* TYPE_UNION field */
    union my_union data;
    
    /* TYPE_USER_STRUCT field */
    user_struct_type point;
};

/* Another structure with various pointer types */
struct GTY(()) pointer_test {
    /* Pointer to scalar */
    int *GTY((skip)) scalar_ptr;
    
    /* Pointer to string */
    char **GTY((skip)) string_ptr_ptr;
    
    /* Pointer to array */
    int (*GTY((skip)) array_ptr)[10];
    
    /* Pointer to callback */
    callback_type *GTY((skip)) callback_ptr;
    
    /* Self-referential pointer */
    struct pointer_test *GTY((skip)) self;
};

/* Union containing various types */
union GTY(()) complex_union {
    struct my_struct s;
    struct container c;
    callback_type cb;
    int array[5];
};

#endif /* GTY_TEST_H */
