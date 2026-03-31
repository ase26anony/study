/* test-gty.h - Primary header for GTY type classification coverage test */
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

/* Complex nested type combinations */
#include "complex-nested-types.h"

/* Macro-generated type variants */
#include "macro-generated-types.h"

/* Edge cases and ambiguity tests */
#include "edge-case-types.h"

#endif /* TEST_GTY_H */
