/* test-gty.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include all specialized type definition headers */
#include "scalar-types.h"
#include "string-types.h"
#include "struct-types.h"
#include "union-types.h"
#include "pointer-types.h"
#include "array-types.h"
#include "callback-types.h"
#include "lang-struct-types.h"
#include "user-struct-types.h"
#include "complex-nested-types.h"
#include "macro-generated-types.h"

/* Forward declarations for complex relationships */
struct GTY(()) forward_declared_struct;
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;  /* Both pointer and string? */

/* Another edge case: pointer to pointer */
typedef GTY(()) forward_ptr_t *double_ptr_t;

/* Void pointer type */
typedef GTY(()) void *generic_ptr_t;

/* Function pointer with complex return type */
typedef struct GTY(()) my_result {
    int status;
    const char *GTY((skip)) message;
} result_t;

typedef result_t *(*GTY(()) complex_callback_t)(int, const char*);

#endif /* TEST_GTY_H */
