/* test-gty.h - Comprehensive GTY type coverage test */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include all specialized type headers */
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

/* Edge case: typedef that might be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;  /* Could be TYPE_POINTER or TYPE_STRING */

/* Another edge case: pointer to array */
typedef GTY(()) int (*pointer_to_array_t)[5];

/* Void pointer type */
typedef GTY(()) void *void_ptr_t;

/* Function type (not callback) */
typedef GTY(()) int func_type_t(int, int);

/* Complete the forward declaration */
struct GTY(()) forward_declared_struct {
    int value;
    forward_ptr_t GTY((skip)) next;  /* Skip to avoid infinite recursion */
};

#endif /* TEST_GTY_H */
