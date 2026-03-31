/* test-gtype.h - Comprehensive test file for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

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
typedef void (*test_callback_fn)(void *data);

/* TYPE_STRING - string type */
typedef const char *test_string_type;

/* TYPE_STRUCT - standard struct */
struct GTY(()) test_struct {
    int GTY((skip)) scalar_field;          /* TYPE_SCALAR */
    test_string_type GTY((tag("0"))) name; /* TYPE_STRING */
    struct test_struct *GTY((skip)) next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - user-defined struct */
typedef struct GTY((user)) test_user_struct {
    int id;
    void *GTY((skip)) user_data;
} test_user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
    int GTY((tag("0"))) int_val;
    double GTY((tag("1"))) double_val;
    test_string_type GTY((tag("2"))) str_val;
};

/* TYPE_ARRAY - fixed size array */
struct GTY(()) test_array_container {
    int GTY((length("10"))) fixed_array[10];
    struct test_struct *GTY((length("5"))) ptr_array[5];
};

/* TYPE_POINTER - various pointer types */
struct GTY(()) test_pointer_struct {
    struct test_struct *GTY((skip)) direct_ptr;
    test_user_struct_t *GTY((skip)) user_ptr;
    union test_union *GTY((skip)) union_ptr;
};

/* Recursive structure for deep processing */
struct GTY(()) test_recursive {
    int value;
    struct test_recursive *GTY((skip)) left;
    struct test_recursive *GTY((skip)) right;
    struct test_array_container *GTY((skip)) container;
};

#endif /* TEST_GTYPE_H */
