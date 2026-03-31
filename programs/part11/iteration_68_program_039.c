/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque pointer */
struct opaque;

/* TYPE_SCALAR: Plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: String type */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) test_struct {
    /* TYPE_SCALAR member */
    int scalar_field;
    
    /* TYPE_STRING member */
    char *string_field;
    
    /* TYPE_POINTER member */
    struct test_struct *next;
    
    /* TYPE_ARRAY member */
    int array_field[10];
    
    /* TYPE_POINTER to opaque/undefined type */
    struct opaque *opaque_ptr;
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) test_union {
    int int_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_USER_STRUCT: Typedef of struct with GTY */
typedef struct GTY(()) user_struct_def {
    int id;
    char *name;
} user_struct_t;

/* TYPE_CALLBACK: Function pointer type with GTY */
typedef void (*GTY(()) callback_func)(int, char *);

/* TYPE_LANG_STRUCT: Simulating language-specific struct */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_type_node {
    int lang_specific;
    void *lang_data;
};
#endif

/* Another struct with nested types to ensure coverage */
struct GTY(()) container_struct {
    /* TYPE_ARRAY of structs */
    struct test_struct items[5];
    
    /* TYPE_ARRAY of pointers */
    struct test_struct *pointers[3];
    
    /* TYPE_CALLBACK field */
    callback_func handler;
    
    /* TYPE_UNION field */
    union test_union data;
};

/* Additional pointer types for TYPE_POINTER coverage */
typedef struct test_struct *GTY(()) struct_ptr_t;
typedef union test_union *GTY(()) union_ptr_t;
typedef int *GTY(()) int_ptr_t;

/* Array typedefs for TYPE_ARRAY coverage */
typedef int GTY(()) int_array_t[20];
typedef struct test_struct *GTY(()) ptr_array_t[15];

#endif /* GTY_TEST_H */
