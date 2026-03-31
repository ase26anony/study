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
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;  /* Both pointer and string */

/* Another edge case: pointer to array */
typedef GTY(()) int (*ptr_to_array_t)[5];

/* Void pointer type */
typedef GTY(()) void *void_ptr_t;

/* Function returning pointer type */
typedef GTY(()) struct my_struct *(*func_returning_ptr_t)(int);

/* Self-referential structure for complex graph traversal */
struct GTY(()) self_ref_struct {
    int value;
    struct self_ref_struct *GTY((skip)) next;  /* Skip for GC */
    struct self_ref_struct *GTY((skip)) prev;
};

/* Union with anonymous struct (edge case) */
union GTY(()) anon_union_member {
    struct {
        int x;
        int y;
    } GTY((tag("0"))) point;
    long coordinates;
};

#endif /* TEST_GTY_H */
