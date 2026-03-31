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

#endif /* TEST_GTYPES_H */
