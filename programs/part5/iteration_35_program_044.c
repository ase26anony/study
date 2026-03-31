/* test-gty.h - Comprehensive GTY type test suite for gengtype coverage */

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

/* Additional edge cases and ambiguous types */
typedef GTY(()) const char * const_string_ptr_t;  /* Could be TYPE_STRING or TYPE_POINTER */
typedef GTY(()) volatile int volatile_scalar_t;   /* TYPE_SCALAR with qualifiers */
typedef GTY(()) const struct my_struct * const_struct_ptr_t;  /* Const pointer to struct */

/* Self-referential structures to test graph traversal */
struct GTY(()) self_ref_struct {
    int data;
    struct self_ref_struct *GTY((skip)) next;  /* Skip to avoid infinite recursion */
};

/* Mutually recursive structures */
struct GTY(()) type_a;
struct GTY(()) type_b;

struct GTY(()) type_a {
    int id;
    struct type_b *GTY((skip)) b_ptr;
};

struct GTY(()) type_b {
    int id;
    struct type_a *GTY((skip)) a_ptr;
};

/* Empty structures and unions */
struct GTY(()) empty_struct {
    /* No members */
};

union GTY(()) empty_union {
    /* No members */
};

/* Typedef chains */
typedef GTY(()) int base_int_t;
typedef GTY(()) base_int_t alias_int_t;
typedef GTY(()) alias_int_t double_alias_int_t;

/* Function types (not callbacks) */
typedef GTY(()) int (*func_ptr_t)(int, int);  /* Regular function pointer */

/* Anonymous struct/union in typedef */
typedef GTY(()) struct {
    int x;
    double y;
} anon_struct_t;

typedef GTY(()) union {
    int i;
    float f;
} anon_union_t;

#endif /* TEST_GTY_H */
