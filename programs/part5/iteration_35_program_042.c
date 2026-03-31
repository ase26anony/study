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

/* Forward declarations for complex type relationships */
struct GTY(()) forward_declared_struct;
union GTY(()) forward_declared_union;

/* Edge case: typedef that might be ambiguous */
typedef GTY(()) const volatile char * ambiguous_string_ptr_t;

/* Another edge case: pointer to pointer */
typedef GTY(()) void ** double_ptr_t;

/* Function to force inclusion of all types */
void GTY(()) use_all_types(void);

#endif /* TEST_GTY_H */
