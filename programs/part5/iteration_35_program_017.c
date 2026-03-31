/* test-gty.h - Primary header for GTY type classification coverage test */

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

/* Forward declarations for complex type relationships */
struct GTY(()) forward_declared_struct;
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that might be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;
typedef GTY(()) char * mutable_string_ptr_t;

/* Another edge case: pointer to pointer */
typedef GTY(()) int **double_ptr_t;
typedef GTY(()) const char **string_ptr_ptr_t;

/* Self-referential structure (common in GCC) */
struct GTY(()) tree_node {
    int code;
    union GTY((desc ("TREE_CODE ((tree) &%h)"))) tree_node_u {
        struct GTY((tag ("0"))) tree_common common;
        struct GTY((tag ("1"))) tree_decl decl;
    } GTY((skip)) u;
    struct tree_node *GTY((skip)) next;
};

/* Type with chain_next (potentially TYPE_LANG_STRUCT) */
struct GTY((chain_next ("%h.next"))) chainable_struct {
    int value;
    struct chainable_struct *next;
};

#endif /* TEST_GTY_H */
