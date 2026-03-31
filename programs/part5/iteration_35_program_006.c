/* test-gty.h - Primary header for gengtype type classification coverage */
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

/* Forward declarations for complex type relationships */
struct GTY(()) forward_declared_struct;
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;  /* Both pointer and string? */

/* Another edge case: pointer to array */
typedef GTY(()) int (*pointer_to_array_t)[5];

/* Void pointer type */
typedef GTY(()) void *void_ptr_t;

/* Function pointer returning pointer to struct */
typedef struct GTY(()) my_result_struct *(*GTY(()) func_returning_ptr_t)(int);

#endif /* TEST_GTY_H */
