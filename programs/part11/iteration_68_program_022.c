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
    struct opaque *undefined_ptr;
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
typedef struct test_struct *GTY(()) struct_pointer_type;

/* TYPE_ARRAY (standalone) */
typedef int GTY(()) int_array_type[20];

/* Nested structure for additional coverage */
struct GTY(()) outer_struct {
    /* TYPE_LANG_STRUCT (simulated through nested struct) */
    struct GTY(()) inner_lang_struct {
        int lang_field1;
        char *lang_field2;
    } lang_member;
    
    /* Multiple pointer types */
    union test_union *union_ptr;
    
    /* Array of pointers */
    struct test_struct *ptr_array[5];
    
    /* Callback field */
    callback_type callback_field;
};

/* Additional union with GTY markers on fields */
union GTY(()) complex_union {
    struct GTY(()) {
        int a;
        int b;
    } nested_struct;
    
    struct GTY(()) another_struct *GTY((skip)) ptr_field;
    
    int GTY((length("0"))) variable_length_array[0];
};

#endif /* GTY_TEST_H */
