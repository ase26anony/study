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
    struct opaque *opaque_ptr;
};

/* TYPE_USER_STRUCT */
typedef struct GTY(()) {
    int x;
    int y;
} user_struct_type;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    char *string_val;
    struct test_struct *struct_ptr;
};

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_POINTER (standalone) */
typedef struct test_struct *GTY(()) struct_ptr_type;

/* TYPE_ARRAY (standalone) */
typedef int GTY(()) int_array_type[5];

/* Nested structure for additional coverage */
struct GTY(()) outer_struct {
    /* TYPE_LANG_STRUCT (simulated via nested struct) */
    struct GTY(()) inner_lang_struct {
        int lang_data;
        callback_type handler;
    } lang_field;
    
    union test_union union_field;
    user_struct_type user_struct_field;
};

/* Function pointer in struct for TYPE_CALLBACK */
struct GTY(()) callback_container {
    callback_type callback;
    void (*GTY(()) another_callback)(void);
};

/* Array of pointers */
struct GTY(()) array_of_pointers {
    struct test_struct *GTY((length("count"))) ptr_array[];
    int count;
};

#endif /* GTY_TEST_H */
