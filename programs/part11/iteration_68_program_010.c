/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;

/* TYPE_SCALAR - plain scalar type */
/* TYPE_STRING - string type */
/* TYPE_POINTER - pointer type */
/* TYPE_ARRAY - array type */
struct GTY(()) scalar_and_pointer_types {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct opaque *opaque_ptr;     /* TYPE_POINTER (to undefined type) */
    int array_field[10];           /* TYPE_ARRAY */
    const char *const_string;      /* TYPE_STRING with qualifiers */
};

/* TYPE_STRUCT - regular struct */
struct GTY(()) my_struct {
    int id;
    char *name;
    struct my_struct *next;        /* TYPE_POINTER within struct */
};

/* TYPE_UNION - union type */
union GTY(()) my_union {
    int int_val;
    char *str_val;
    double dbl_val;
    struct my_struct *struct_ptr;  /* TYPE_POINTER within union */
};

/* TYPE_USER_STRUCT - struct with user-defined marking */
typedef struct GTY((user)) user_struct {
    int data;
    void (*cleanup)(void*);        /* Function pointer (not GTY) */
} user_struct_t;

/* TYPE_CALLBACK - function pointer with GTY annotation */
typedef void GTY((callback)) (*callback_func)(int, char*);

/* Structure containing a callback */
struct GTY(()) struct_with_callback {
    int id;
    callback_func handler;         /* TYPE_CALLBACK */
};

/* Nested structures for complex testing */
struct GTY(()) outer_struct {
    int outer_id;
    
    /* Inner anonymous struct */
    struct GTY(()) {
        int inner_id;
        char *inner_name;
    } inner;
    
    /* Pointer to union */
    union my_union * GTY((skip)) union_ptr;  /* TYPE_POINTER with skip */
    
    /* Array of structs */
    struct my_struct struct_array[5];        /* TYPE_ARRAY of structs */
};

/* TYPE_LANG_STRUCT - language-specific structure */
/* This typically requires special handling in language frontends */
#ifdef GENERATOR_FILE
struct GTY((language)) lang_struct {
    int lang_specific;
    void *lang_data;
};
#endif

/* Self-referential structure */
struct GTY(()) tree_node {
    int code;
    union GTY((desc ("TREE_CODE ((tree)&%h)"))) {
        struct tree_node *child;   /* TYPE_POINTER */
        int value;
        char *string;
    } GTY((tag ("0"))) u;
    struct tree_node *next;        /* TYPE_POINTER */
};

/* Variable-length array structure */
struct GTY(()) varray {
    size_t length;
    int elements[1];               /* TYPE_ARRAY (variable length) */
};

/* Chain of structures for linked list testing */
struct GTY(()) chain {
    int value;
    struct chain * GTY((chain_next ("%h.next"))) next;
    struct chain *prev;
};

#endif /* GTY_TEST_H */
