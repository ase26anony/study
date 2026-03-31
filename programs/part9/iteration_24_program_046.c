/* Master header file that includes all type definitions */
#ifndef TEST_ALL_TYPES_H
#define TEST_ALL_TYPES_H

#include "test-basic-types.h"
#include "test-structs-unions.h"
#include "test-pointers-arrays.h"
#include "test-callbacks.h"
#include "test-special-types.h"
#include "test-nested-complex.h"

/* Include conditional definitions */
#ifdef MAKE_EXTRA_TYPES
#include "test-conditional-types.h"
#endif

#endif /* TEST_ALL_TYPES_H */
