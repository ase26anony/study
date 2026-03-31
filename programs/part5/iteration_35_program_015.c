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

/* Forward declarations for complex type relationships */
struct GTY(()) forward_declared_struct;
union GTY(()) forward_declared_union;

/* Additional edge case combinations */
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;
typedef GTY(()) union forward_declared_union *forward_union_ptr_t;

/* Mixed type in single declaration */
struct GTY(()) mixed_container {
    int scalar_field;
    const char *GTY((tag("0"))) string_field;
    struct forward_declared_struct *struct_field;
    void (*callback_field)(int);
    int array_field[5];
};

#endif /* TEST_GTY_H */
