/* Primary header file for GTY type classification coverage test */
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
typedef struct forward_declared_struct * GTY(()) forward_ptr_t;

/* Edge case: typedef that might be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;
typedef GTY(()) char * mutable_string_ptr_t;

/* Macro-generated type variants */
#define DEF_GTY_SCALAR_TYPE(name, type) typedef GTY(()) type name##_t
#define DEF_GTY_PTR_TYPE(name, base) typedef GTY(()) base * name##_ptr_t
#define DEF_GTY_ARRAY_TYPE(name, base, size) typedef GTY(()) base name##_array_t[size]

DEF_GTY_SCALAR_TYPE(macro_int, int);
DEF_GTY_SCALAR_TYPE(macro_long, long);
DEF_GTY_PTR_TYPE(macro_int, int);
DEF_GTY_PTR_TYPE(macro_struct, struct my_basic_struct);
DEF_GTY_ARRAY_TYPE(macro_int, int, 20);
DEF_GTY_ARRAY_TYPE(macro_ptr, void *, 15);

/* Special case: pointer to array */
typedef GTY(()) int (*array_ptr_t)[10];

/* Special case: array of pointers */
typedef GTY(()) void *ptr_array_t[5];

#endif /* TEST_GTY_H */
