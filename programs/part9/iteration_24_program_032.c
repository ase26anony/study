/* Master header file to include all type definitions */
#ifndef TEST_ALL_TYPES_H
#define TEST_ALL_TYPES_H

#include "test-basic-types.h"
#include "test-struct-union.h"
#include "test-pointers-arrays.h"
#include "test-callbacks.h"
#include "test-special-structs.h"
#include "test-nested-complex.h"

/* Additional edge cases */
typedef GTY(()) struct forward_declared *forward_ptr;  /* TYPE_UNDEFINED */

#endif /* TEST_ALL_TYPES_H */
