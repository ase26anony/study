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
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;  /* TYPE_POINTER to TYPE_STRING */

/* Another edge case: pointer to pointer */
typedef GTY(()) forward_ptr_t *double_ptr_t;

/* Macro-generated type variants */
#define DEF_GTY_SCALAR_TYPE(name, type) typedef GTY(()) type name##_t
#define DEF_GTY_PTR_TYPE(name, base) typedef GTY(()) base * name##_ptr_t
#define DEF_GTY_ARRAY_TYPE(name, base, size) typedef GTY(()) base name##_array_t[size]

DEF_GTY_SCALAR_TYPE(macro_int, int);
DEF_GTY_SCALAR_TYPE(macro_long, long);
DEF_GTY_PTR_TYPE(macro_int_ptr, int);
DEF_GTY_PTR_TYPE(macro_struct_ptr, struct my_struct);
DEF_GTY_ARRAY_TYPE(macro_int, int, 20);
DEF_GTY_ARRAY_TYPE(macro_ptr, void*, 10);

/* Special case: anonymous struct/union */
struct GTY(()) {
    int x;
    double y;
} anonymous_struct_var;

union GTY(()) {
    int i;
    float f;
} anonymous_union_var;

#endif /* TEST_GTY_H */
