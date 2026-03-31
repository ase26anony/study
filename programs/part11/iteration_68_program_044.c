/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED test */
struct opaque;

/* TYPE_SCALAR: Plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: String type */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT: Basic struct */
struct GTY(()) basic_struct {
    int GTY(()) scalar_field;      /* TYPE_SCALAR */
    char *GTY(()) string_field;    /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd struct */
typedef struct GTY(()) {
    int x;
    int y;
} user_struct_type;

/* TYPE_UNION: Union type */
union GTY(()) basic_union {
    int GTY(()) int_val;
    char *GTY(()) str_val;
    struct basic_struct *GTY(()) struct_ptr;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_LANG_STRUCT: Language-specific struct (simulated) */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
    struct lang_struct *GTY(()) next;
    struct lang_struct *GTY(()) prev;
    int data;
};

/* Main struct containing all type categories */
struct GTY(()) container {
    /* TYPE_SCALAR */
    int GTY(()) count;
    
    /* TYPE_STRING */
    char *GTY(()) name;
    
    /* TYPE_STRUCT */
    struct basic_struct GTY(()) basic;
    
    /* TYPE_USER_STRUCT */
    user_struct_type GTY(()) user;
    
    /* TYPE_UNION */
    union basic_union GTY(()) value;
    
    /* TYPE_POINTER */
    struct basic_struct *GTY(()) ptr;
    
    /* TYPE_ARRAY */
    int GTY(()) array[10];
    
    /* TYPE_CALLBACK */
    callback_type GTY(()) callback;
    
    /* TYPE_UNDEFINED reference */
    struct opaque *GTY(()) opaque_ptr;
    
    /* Nested pointer for additional TYPE_POINTER coverage */
    union basic_union *GTY(()) union_ptr;
    
    /* Another array for TYPE_ARRAY coverage */
    struct basic_struct GTY(()) struct_array[5];
    
    /* Pointer array */
    struct basic_struct *GTY(()) ptr_array[3];
    
    /* TYPE_LANG_STRUCT */
    struct lang_struct *GTY(()) lang_chain;
};

/* Additional union with pointer members */
union GTY(()) complex_union {
    struct container *GTY(()) container_ptr;
    callback_type GTY(()) func_ptr;
    struct lang_struct *GTY(()) lang_ptr;
};

/* Array of structs */
struct basic_struct GTY(()) global_array[20];

/* Pointer to array */
struct basic_struct (*GTY(()) array_ptr)[20];

#endif /* GTY_TEST_H */
