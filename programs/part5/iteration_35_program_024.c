/* test-gty.h - Comprehensive GTY type test suite for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include all specialized type definition headers */
#include "scalar-types.h"
#include "string-types.h"
#include "struct-types.h"
#include "user-struct-types.h"
#include "union-types.h"
#include "pointer-types.h"
#include "array-types.h"
#include "callback-types.h"
#include "lang-struct-types.h"
#include "complex-nested-types.h"
#include "macro-generated-types.h"

/* Forward declarations for complex relationships */
struct GTY(()) forward_declared_struct;
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;  /* TYPE_POINTER to TYPE_STRING */

/* Another edge case: pointer to array */
typedef GTY(()) int (*ptr_to_array_t)[5];

/* Self-referential structure for complex graph traversal */
struct GTY(()) self_ref {
    int value;
    struct self_ref *GTY((skip)) next;  /* Skip to avoid infinite recursion in gengtype */
};

/* Void pointer type */
typedef GTY(()) void *void_ptr_t;

/* Const pointer type */
typedef GTY(()) const int *const_int_ptr_t;

/* Volatile pointer type */
typedef GTY(()) volatile char *volatile_char_ptr_t;

#endif /* TEST_GTY_H */
