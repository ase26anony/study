/* Primary header file for GTY type classification coverage test */
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
typedef GTY(()) const char * const_string_ptr_t;
typedef GTY(()) char * mutable_string_ptr_t;

/* Special case: void pointer */
typedef GTY(()) void * void_ptr_t;

/* Complete the forward declaration */
struct GTY(()) forward_declared_struct {
    int value;
    forward_ptr_t GTY((skip)) next;
};

#endif /* TEST_GTY_H */
