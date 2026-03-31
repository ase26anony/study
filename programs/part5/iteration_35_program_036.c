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
typedef struct forward_declared_struct * GTY(()) forward_ptr_t;

/* Edge case: typedef that might be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;
typedef GTY(()) char * mutable_string_ptr_t;

/* Multiple levels of indirection */
typedef GTY(()) forward_ptr_t *** triple_indirect_t;

/* Self-referential structure for graph traversal */
struct GTY(()) recursive_node {
    int value;
    struct recursive_node * GTY((skip)) next;
    struct recursive_node * GTY((skip)) prev;
};

/* Mixed type in union */
union GTY(()) mixed_container {
    int scalar;
    const char * GTY((tag("0"))) str;
    void (*GTY((tag("1"))) callback)(void);
    struct recursive_node * GTY((tag("2"))) node;
};

#endif /* TEST_GTY_H */
