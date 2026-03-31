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
union GTY(()) forward_declared_union;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;

/* Another edge case: pointer to const pointer */
typedef GTY(()) const int * const * double_const_ptr_t;

/* Mixed type with skip annotation */
struct GTY(()) mixed_skip_struct {
    int GTY((skip)) skipped_field;
    void *GTY((tag("0"))) tagged_ptr;
    const char *GTY(()) name;
};

#endif /* TEST_GTY_H */
