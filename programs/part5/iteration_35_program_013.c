/* Primary header file that includes all GTY type definitions */
#ifndef TEST_GTY_H
#define TEST_GTY_H

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

/* Forward declarations for complex relationships */
struct GTY(()) forward_declared_struct;
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;
typedef GTY(()) char * mutable_string_ptr_t;

/* Macro-generated type variants */
#define DEF_GTY_SCALAR_TYPE(T, name) typedef GTY(()) T name##_t
#define DEF_GTY_PTR_TYPE(T, name) typedef GTY(()) T * name##_ptr_t
#define DEF_GTY_ARRAY_TYPE(T, size, name) typedef GTY(()) T name##_array_t[size]

DEF_GTY_SCALAR_TYPE(unsigned long, ulong_scalar);
DEF_GTY_PTR_TYPE(struct my_struct, my_struct);
DEF_GTY_ARRAY_TYPE(int, 20, int_array);

#endif /* TEST_GTY_H */
