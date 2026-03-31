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
struct GTY(()) test_struct {
    /* TYPE_SCALAR */
    int scalar_field;
    
    /* TYPE_STRING */
    char *string_field;
    
    /* TYPE_POINTER */
    struct test_struct *next;
    
    /* TYPE_ARRAY */
    int array_field[10];
    
    /* Pointer to undefined type */
    struct opaque *undefined_ptr;
};

/* TYPE_USER_STRUCT */
typedef struct GTY(()) {
    int x;
    double y;
} user_struct_type;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    double double_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, const char*);

/* TYPE_POINTER (standalone typedef) */
typedef struct test_struct *GTY(()) struct_ptr_type;

/* TYPE_ARRAY (standalone typedef) */
typedef int GTY(()) int_array_type[5];

/* Nested structure with all types */
struct GTY(()) container {
    /* TYPE_STRUCT */
    struct test_struct nested_struct;
    
    /* TYPE_UNION */
    union test_union nested_union;
    
    /* TYPE_POINTER */
    callback_type callback_ptr;
    
    /* TYPE_ARRAY of pointers */
    struct_ptr_type ptr_array[3];
    
    /* TYPE_ARRAY of scalars */
    scalar_type scalar_array[8];
    
    /* TYPE_STRING array */
    string_type string_array[4];
    
    /* TYPE_CALLBACK array */
    callback_type callbacks[2];
};

/* Language-specific structure (TYPE_LANG_STRUCT) */
struct GTY((tag("LANG"))) lang_struct {
    int lang_specific;
    void *lang_data;
};

/* Function pointer with complex signature */
typedef int (*GTY(()) complex_callback)(
    struct test_struct *,
    union test_union *,
    callback_type,
    int[]
);

#endif /* GTY_TEST_H */
