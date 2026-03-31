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
#include "macro-generated-types.h"

/* Forward declarations for complex relationships */
struct GTY(()) forward_declared_struct;
union GTY(()) forward_declared_union;

/* Type that might be classified as TYPE_UNDEFINED during processing */
typedef GTY(()) struct incomplete *incomplete_ptr_t;

/* Edge case: typedef with GTY on incomplete type */
typedef GTY(()) struct truly_incomplete *truly_incomplete_t;

#endif /* TEST_GTY_H */
