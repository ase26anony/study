/* test-gty.h - Comprehensive GTY type definitions for coverage testing */

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

/* Additional edge cases and ambiguous types */
typedef GTY(()) const char * const * nested_string_ptr_t;
typedef GTY(()) volatile int volatile_scalar_t;

/* Type that could be borderline between multiple classifications */
struct GTY(()) ambiguous_container {
    int GTY((tag("0"))) tag_field;
    union {
        int scalar;
        const char *string;
        void *GTY((skip)) opaque;
    } GTY((desc("tag_field"))) data;
};

#endif /* TEST_GTY_H */
