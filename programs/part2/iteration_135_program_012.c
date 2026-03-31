/* test-gtypes.h - Comprehensive test of all gengtype type classifications */
#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - basic scalar types */
typedef enum {
    TEST_ENUM_A,
    TEST_ENUM_B,
    TEST_ENUM_C
} test_enum_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*test_callback_fn)(void *data) GTY((callback));

/* TYPE_STRING - string type */
typedef const char *test_string_type;

/* TYPE_STRUCT - standard struct */
struct GTY(()) test_struct {
    int GTY((skip)) scalar_field;          /* TYPE_SCALAR */
    test_string_type string_field;         /* TYPE_STRING */
    struct test_struct *GTY((skip)) next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - user-defined struct with special handling */
typedef struct GTY((user)) test_user_struct {
    int id;
    void *GTY((skip)) user_data;
} test_user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    double double_val;
    test_string_type str_val;
};

/* TYPE_ARRAY - fixed size array */
struct GTY(()) test_array_container {
    int GTY((length("10"))) fixed_array[10];
    struct test_struct *GTY((length("count"))) var_array;
    int count;
};

/* TYPE_POINTER - standalone pointer type definition */
typedef struct test_struct *test_struct_ptr;

/* Recursive structure for deep processing */
struct GTY(()) recursive_struct {
    int value;
    struct recursive_struct *GTY((skip)) left;
    struct recursive_struct *GTY((skip)) right;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    union test_union data;
    struct test_array_container arrays;
    test_callback_fn callback;
};

#endif /* TEST_GTYPES_H */
