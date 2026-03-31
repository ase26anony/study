/* test-gty.h - Primary header for gengtype coverage testing */
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
#include "complex-nested.h"
#include "macro-generated.h"

/* Forward declarations for complex relationships */
struct GTY(()) forward_declared_struct;
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;  /* Both pointer and string? */

/* Another edge case: pointer to array */
typedef GTY(()) int (*array_ptr_t)[5];

/* Void pointer type */
typedef GTY(()) void *generic_ptr_t;

/* Self-referential structure */
struct GTY(()) self_ref {
    int value;
    struct self_ref *GTY((skip)) next;  /* Skip to avoid infinite recursion */
};

/* Multiple inheritance-like structure using GTY tags */
struct GTY((desc("0"))) base_struct {
    int base_field;
};

struct GTY((desc("1"))) derived_struct {
    struct base_struct base;
    int derived_field;
};

#endif /* TEST_GTY_H */
