/* test-gty.h - Comprehensive GTY type definitions for coverage testing */

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
union GTY(()) forward_declared_union;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;  /* Both pointer and string */

/* Another edge case: pointer to array */
typedef GTY(()) int (*pointer_to_array_t)[5];

/* Void pointer type */
typedef GTY(()) void * void_ptr_t;

/* Function pointer returning pointer */
typedef void * (*GTY(()) func_returning_ptr_t)(int);

/* Self-referential structure (common in GCC) */
struct GTY(()) tree_node {
  int code;
  union GTY((desc ("%1.code"))) tree_node_u {
    struct GTY((tag ("0"))) tree_common common;
    struct GTY((tag ("1"))) tree_decl decl;
  } u;
  struct tree_node *GTY((skip)) next;
};

#endif /* TEST_GTY_H */
