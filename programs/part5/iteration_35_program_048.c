/* test-gty.h - Primary header for GTY type classification coverage test */

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

/* Forward declarations for complex type relationships */
struct GTY(()) forward_declared_struct;
union GTY(()) forward_declared_union;

/* Additional edge case types */
typedef GTY(()) volatile int volatile_scalar_t;
typedef GTY(()) const volatile int cv_scalar_t;

/* Pointer to incomplete type (should still be classified) */
typedef GTY(()) struct incomplete_struct *incomplete_ptr_t;

/* Multi-level const pointer */
typedef GTY(()) const char * const * const_double_ptr_t;

/* Function returning pointer type */
typedef GTY(()) struct my_struct * (*func_returning_ptr_t)(void);

#endif /* TEST_GTY_H */
