/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR */
typedef int GTY(()) scalar_type;

/* TYPE_STRING */
typedef const char * GTY(()) string_type;

/* TYPE_STRUCT */
struct GTY(()) test_struct {
    /* TYPE_SCALAR */
    int scalar_member;
    
    /* TYPE_STRING */
    const char *string_member;
    
    /* TYPE_POINTER */
    struct test_struct *next;
    
    /* TYPE_ARRAY */
    int array_member[10];
    
    /* Pointer to undefined type */
    struct opaque *undefined_ptr;
};

/* TYPE_UNION */
union GTY(()) test_union {
    int int_member;
    float float_member;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, const char*);

/* TYPE_USER_STRUCT - Requires special handling */
/* This is typically for types that need custom marking routines */
typedef struct GTY((user)) user_struct {
    int data;
    void (*cleanup)(struct user_struct*);
} user_struct_t;

/* Another struct with nested types */
struct GTY(()) container_struct {
    /* TYPE_STRUCT (nested) */
    struct GTY(()) nested_struct {
        int x;
        int y;
    } nested;
    
    /* TYPE_UNION (nested) */
    union GTY(()) nested_union {
        int a;
        float b;
    } u_nested;
    
    /* TYPE_CALLBACK usage */
    callback_type callback;
    
    /* TYPE_ARRAY of pointers */
    struct test_struct *ptr_array[5];
    
    /* TYPE_ARRAY of scalars */
    int int_array[20];
};

/* TYPE_LANG_STRUCT - Typically for language-specific structures */
/* In GCC, these are marked with GTY for language frontends */
struct GTY(()) lang_specific {
    int lang_tag;
    void *lang_data;
};

/* Additional pointer types for coverage */
typedef struct test_struct * GTY(()) struct_ptr_t;
typedef union test_union * GTY(()) union_ptr_t;
typedef int (*GTY(()) func_ptr_t)(void);

/* Array typedefs */
typedef int GTY(()) int_array_t[50];
typedef struct test_struct * GTY(()) struct_ptr_array_t[10];

#endif /* GTY_TEST_H */
