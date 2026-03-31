/* test-gty.h - Primary header for gengtype type classification coverage test */

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
#include "edge-case-types.h"

/* Forward declarations for complex type relationships */
struct GTY(()) forward_decl_struct;
typedef struct forward_decl_struct *forward_ptr_t;

/* Type that might be classified as TYPE_NONE if something goes wrong */
/* This should trigger gcc_unreachable() if classification fails completely */

#endif /* TEST_GTY_H */
