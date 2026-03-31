/* test-gtypes.h - Comprehensive test file for gengtype coverage */
#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration without definition */
struct opaque_struct;

/* TYPE_SCALAR - Basic scalar types */
typedef enum {
    TEST_ENUM_A,
    TEST_ENUM_B,
    TEST_ENUM_C
} test_enum_type;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*test_callback_fn)(void *data) GTY((callback));

/* TYPE_STRING - String type */
typedef const char *test_string_type;

/* TYPE_STRUCT - Regular struct */
struct GTY(()) test_struct {
    int GTY((skip)) scalar_field;          /* TYPE_SCALAR */
    test_string_type string_field;         /* TYPE_STRING */
    struct test_struct *GTY((tag("0"))) next;  /* TYPE_POINTER */
    unsigned long array_field[10];         /* TYPE_ARRAY (fixed size) */
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
typedef struct GTY((user)) test_user_struct {
    void *GTY((skip)) user_data;
    int user_id;
} test_user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    double double_val;
    test_string_type str_val;
    struct test_struct *GTY((tag("1"))) struct_ptr;
};

/* Forward declaration for recursive types */
struct GTY(()) recursive_struct;

/* More complex struct with nested types */
struct GTY(()) complex_struct {
    /* TYPE_ARRAY of pointers */
    struct test_struct *GTY((length("array_len"))) ptr_array[5];
    
    /* TYPE_ARRAY of scalars */
    int GTY((length("scalar_len"))) scalar_array[8];
    
    /* Nested union */
    union test_union nested_union;
    
    /* Callback function pointer */
    test_callback_fn callback;
    
    /* Recursive pointer */
    struct recursive_struct *GTY((tag("2"))) recursive_ptr;
};

/* Complete the recursive struct definition */
struct GTY(()) recursive_struct {
    int data;
    struct complex_struct *GTY((tag("3"))) complex_ptr;
    struct recursive_struct *GTY((tag("4"))) next;
};

/* Variable length array struct */
struct GTY(()) var_array_struct {
    int count;
    int GTY((length("count"))) variable_array[1];
};

#endif /* TEST_GTYPES_H */
