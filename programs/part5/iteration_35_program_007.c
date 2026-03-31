/* Primary header file that includes all GTY type definitions */
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

/* Additional combined types for edge cases */
typedef GTY(()) const char * const_string_ptr_t;
typedef GTY(()) volatile int volatile_scalar_t;

/* Macro-generated type variants */
#define DEF_GTY_PTR_TYPE(TYPE_NAME, BASE_TYPE) \
    typedef GTY(()) BASE_TYPE * TYPE_NAME##_ptr_t

DEF_GTY_PTR_TYPE(int, int);
DEF_GTY_PTR_TYPE(struct, struct my_struct);
DEF_GTY_PTR_TYPE(union, union my_union);

/* Template-like macro for arrays */
#define DEF_GTY_ARRAY_TYPE(TYPE_NAME, BASE_TYPE, SIZE) \
    typedef GTY(()) BASE_TYPE TYPE_NAME##_array_t[SIZE]

DEF_GTY_ARRAY_TYPE(int, int, 10);
DEF_GTY_ARRAY_TYPE(struct, struct my_struct, 5);

#endif /* TEST_GTY_H */
