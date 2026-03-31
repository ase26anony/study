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

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct GTY(()) user_struct_def {
    int GTY(()) value;
    struct user_struct_def *GTY(()) next;  /* TYPE_POINTER */
} user_struct;

/* TYPE_UNION: Union type */
union GTY(()) test_union {
    int GTY(()) int_val;
    char *GTY(()) str_val;
    void *GTY(()) ptr_val;
};

/* TYPE_POINTER: Pointer type in struct */
struct GTY(()) pointer_container {
    struct basic_struct *GTY(()) struct_ptr;   /* TYPE_POINTER to TYPE_STRUCT */
    union test_union *GTY(()) union_ptr;       /* TYPE_POINTER to TYPE_UNION */
    struct opaque *GTY(()) opaque_ptr;         /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
    int GTY(()) scalar_array[10];              /* TYPE_ARRAY of TYPE_SCALAR */
    char *GTY(()) string_array[5];             /* TYPE_ARRAY of TYPE_STRING */
    struct basic_struct GTY(()) struct_array[3]; /* TYPE_ARRAY of TYPE_STRUCT */
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, char *);

struct GTY(()) callback_container {
    callback_func GTY(()) handler;             /* TYPE_CALLBACK */
    void (*GTY(()) direct_callback)(void);     /* Direct TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Language-specific struct */
/* This typically requires special handling in gengtype */
struct GTY((user)) lang_specific {
    int GTY(()) lang_data;
    void *GTY(()) lang_private;
};

/* Complex nested structure to test multiple classifications */
struct GTY(()) complex_nested {
    /* TYPE_STRUCT containing various types */
    struct basic_struct GTY(()) nested_struct;
    
    /* TYPE_UNION */
    union test_union GTY(()) nested_union;
    
    /* TYPE_POINTER */
    struct array_container *GTY(()) container_ptr;
    
    /* TYPE_ARRAY */
    callback_func GTY(()) callbacks[4];
    
    /* TYPE_CALLBACK */
    int (*GTY(()) compare_func)(const void *, const void *);
    
    /* Mixed scalar and string */
    int GTY(()) count;
    char *GTY(()) name;
    
    /* Pointer to undefined type */
    struct undefined_type *GTY(()) undefined_ptr;
};

#endif /* GTY_TEST_H */
