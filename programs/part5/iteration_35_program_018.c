/* test-gty.h - Comprehensive GTY type definitions for coverage testing */

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
typedef struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that might be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;
typedef GTY(()) char * mutable_string_ptr_t;

/* Special GTY options combinations */
struct GTY((length("len"))) variable_length_struct {
    int len;
    int GTY((skip)) *data;
};

/* Chain of structures for GC traversal */
struct GTY((chain_next("next"))) chainable_struct {
    int value;
    struct chainable_struct *next;
};

/* Tagged union with discriminator */
union GTY((desc("tag"))) tagged_union {
    int tag;
    struct {
        int tag;
        int value;
    } GTY((tag("1"))) as_int;
    struct {
        int tag;
        const char *name;
    } GTY((tag("2"))) as_string;
};

#endif /* TEST_GTY_H */
