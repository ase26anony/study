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
    int scalar_field;          /* TYPE_SCALAR */
    char *string_field;        /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int data;
} user_struct_type;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;            /* TYPE_STRING within union */
    void *ptr_val;            /* TYPE_POINTER within union */
};

/* TYPE_CALLBACK: function pointer type */
typedef void (*GTY(()) callback_type)(int, char*);

/* TYPE_POINTER: pointer type */
typedef struct basic_struct *GTY(()) struct_pointer;

/* TYPE_ARRAY: array type */
typedef int GTY(()) int_array[10];

/* Complex structure to trigger multiple classifications */
struct GTY(()) complex_type {
    /* TYPE_SCALAR */
    int count;
    
    /* TYPE_STRING */
    char *name;
    
    /* TYPE_POINTER */
    struct basic_struct *next;
    
    /* TYPE_ARRAY */
    int values[5];
    
    /* TYPE_POINTER to undefined type */
    struct opaque *unknown;
    
    /* TYPE_CALLBACK */
    callback_type handler;
    
    /* TYPE_UNION */
    union test_union data;
};

/* TYPE_LANG_STRUCT: Typically used for language-specific structures */
/* In GCC, these are often marked with GTY(()) and special tags */
struct GTY((tag("LANG"))) lang_struct {
    int lang_specific;
};

/* Nested structures for additional coverage */
struct GTY(()) container {
    /* TYPE_STRUCT within TYPE_STRUCT */
    struct basic_struct inner;
    
    /* TYPE_POINTER array */
    struct basic_struct *GTY((length("%0.count"))) items;
    
    /* TYPE_ARRAY of pointers */
    callback_type GTY((skip)) callbacks[3];
};

/* Template-like structure for edge cases */
struct GTY(()) variable_length {
    int length;
    /* Variable length array - treated specially by gengtype */
    char data[1];
};

#endif /* GTY_TEST_H */
