/* test-gty.h - Primary header for gengtype type classification coverage */
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
typedef GTY(()) const volatile char * ambiguous_string_ptr_t;

/* Special case: opaque pointer type */
typedef GTY(()) void * GTY((atomic)) opaque_ptr_t;

/* Chain of types to ensure traversal */
struct GTY(()) chain_node {
    int data;
    struct chain_node *GTY((skip)) next;
    struct chain_node *GTY((skip)) prev;
};

/* Self-referential structure */
struct GTY(()) self_ref {
    int value;
    struct self_ref *GTY((skip)) self;
};

#endif /* TEST_GTY_H */
