/* test-gty.h - Comprehensive GTY test types for coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_SCALAR - Basic scalar types */
typedef enum {
    TEST_ENUM_A,
    TEST_ENUM_B,
    TEST_ENUM_C
} test_enum GTY(());

typedef bool test_bool GTY(());

/* TYPE_CALLBACK - Function pointer type */
typedef void (*test_callback_fn)(void *data) GTY(());

/* TYPE_STRING */
typedef const char *test_string GTY(());

/* TYPE_STRUCT - Standard struct */
struct test_struct GTY(())
{
    int scalar_field;
    test_enum enum_field;
    struct test_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
typedef struct user_struct
{
    int id;
    void *user_data;
} user_struct_t;
#define GTY_USER_STRUCT user_struct_t

/* TYPE_UNION */
union test_union GTY(())
{
    int int_val;
    double double_val;
    struct test_struct *struct_ptr;
};

/* TYPE_ARRAY - Fixed size array */
struct array_container GTY(())
{
    int fixed_array[10];  /* Fixed-size array */
    int *variable_array GTY((length("len")));  /* Variable-length array */
    size_t len;
};

/* TYPE_POINTER - Various pointer types */
struct pointer_container GTY(())
{
    struct test_struct *direct_ptr;
    void *void_ptr;
    test_callback_fn callback_ptr;
};

#endif /* TEST_GTY_H */
