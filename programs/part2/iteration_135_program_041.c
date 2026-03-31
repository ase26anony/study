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
typedef void (*test_callback_fn)(int, void*);

/* TYPE_STRING - string type */
typedef const char *test_string_type;

/* TYPE_STRUCT - standard struct */
struct GTY(()) test_struct {
    int GTY((skip)) scalar_field;      /* TYPE_SCALAR */
    const char * GTY((skip)) name;     /* TYPE_STRING */
    test_enum_type GTY((skip)) enum_field; /* TYPE_SCALAR (enum) */
    struct test_struct * GTY((skip)) next; /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - user-defined struct with special handling */
typedef struct GTY((user)) test_user_struct {
    int id;
    void * GTY((skip)) user_data;
} test_user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
    int GTY((skip)) int_val;
    double GTY((skip)) double_val;
    struct test_struct * GTY((skip)) struct_ptr;
    const char * GTY((skip)) string_val;
};

/* TYPE_ARRAY - fixed size array */
struct GTY(()) test_array_container {
    int GTY((skip)) fixed_array[10];           /* Fixed array */
    struct test_struct * GTY((skip)) ptr_array[5]; /* Array of pointers */
    int GTY((length ("len"))) *variable_array; /* Variable length array */
    size_t len;
};

/* Recursive structure for deep processing */
struct GTY(()) test_recursive {
    int value;
    struct test_recursive * GTY((skip)) left;
    struct test_recursive * GTY((skip)) right;
    union test_union GTY((skip)) data;
};

/* Container with multiple type kinds */
struct GTY(()) test_container {
    struct test_struct GTY((skip)) base_struct;    /* TYPE_STRUCT */
    test_user_struct_t GTY((skip)) user_struct;    /* TYPE_USER_STRUCT */
    union test_union GTY((skip)) data_union;       /* TYPE_UNION */
    struct test_array_container GTY((skip)) arrays; /* TYPE_STRUCT containing arrays */
    test_callback_fn GTY((skip)) callback;         /* TYPE_CALLBACK */
    struct opaque_struct * GTY((skip)) opaque_ptr; /* TYPE_POINTER to TYPE_UNDEFINED */
};

#endif /* TEST_GTYPES_H */
