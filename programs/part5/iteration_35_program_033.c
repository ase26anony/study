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
struct GTY(()) base_struct;
union GTY(()) base_union;

/* Additional edge case combinations */
typedef GTY(()) struct base_struct * volatile volatile_struct_ptr_t;
typedef GTY(()) const struct base_struct * const_struct_ptr_t;
typedef GTY(()) struct base_struct * restrict restrict_struct_ptr_t;

/* Mixed pointer types with different qualifiers */
typedef GTY(()) const char * const * string_ptr_ptr_t;
typedef GTY(()) const char * const * const string_const_ptr_ptr_t;

/* Function pointer with GTY attributes */
typedef void (*GTY((callback)) complex_callback_t)(int, const char *);

/* Array of function pointers */
typedef GTY(()) complex_callback_t callback_array_t[5];

/* Self-referential structure for graph traversal */
struct GTY(()) recursive_node {
    int value;
    struct recursive_node *GTY((skip)) next;
    struct recursive_node *GTY((skip)) prev;
};

/* Union with nested anonymous struct */
union GTY(()) nested_anon_union {
    struct {
        int x;
        int y;
    } point;
    long coordinates;
};

#endif /* TEST_GTY_H */
