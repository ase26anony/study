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
    int scalar_field;           /* TYPE_SCALAR */
    char *string_field;         /* TYPE_STRING */
    struct opaque *opaque_ptr;  /* TYPE_POINTER to undefined type */
};

/* TYPE_USER_STRUCT: User-defined struct with tag */
typedef struct GTY((tag("USER_STRUCT"))) user_struct {
    int id;
    char *name;
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) test_union {
    int as_int;
    char *as_string;
    void *as_pointer;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.lang_type"))) lang_struct {
    int lang_type;
    union test_union data;
};

/* Main structure containing all type variations */
struct GTY(()) container {
    /* TYPE_POINTER: Various pointer types */
    struct basic_struct *struct_ptr;
    union test_union *union_ptr;
    callback_func callback_ptr;
    int *scalar_ptr;
    char **string_ptr_ptr;
    
    /* TYPE_ARRAY: Array types */
    int scalar_array[10];           /* Fixed-size array */
    char *string_array[5];          /* Array of pointers */
    struct basic_struct *struct_array[3]; /* Array of struct pointers */
    
    /* Nested structures */
    struct basic_struct nested_struct;
    union test_union nested_union;
    
    /* Callback field */
    callback_func callback_field;
    
    /* Language structure */
    struct lang_struct lang_field;
    
    /* User structure */
    user_struct_t user_field;
};

/* Another structure with pointer chains for complex testing */
struct GTY(()) pointer_chain {
    struct pointer_chain *next;  /* Self-referential pointer */
    struct container *container;
    void *generic_ptr;
};

/* Union containing pointers */
union GTY(()) pointer_union {
    struct container *c_ptr;
    struct pointer_chain *p_ptr;
    void *v_ptr;
};

#endif /* GTY_TEST_H */
