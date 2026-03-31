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
typedef GTY(()) struct forward_declared_struct *forward_ptr_t;

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;
typedef GTY(()) char * mutable_string_ptr_t;

/* Self-referential structure (common in GCC) */
struct GTY(()) tree_common {
  int code;
  union GTY((desc ("0"))) tree_common_u {
    struct GTY((tag ("0"))) tree_common *GTY((skip)) next;
    int value;
  } u;
  struct tree_common *GTY((skip)) chain;
};

/* Language structure with chain_next (typical for TYPE_LANG_STRUCT) */
struct GTY((chain_next ("%h.next"))) lang_tree_node {
  int lang_specific;
  struct lang_tree_node *next;
  struct tree_common *base;
};

/* Another language structure variant */
struct GTY((chain_next ("%h.next_field"))) lang_struct_v2 {
  int field1;
  struct lang_struct_v2 *next_field;
  void *GTY((skip)) opaque_data;
};

#endif /* TEST_GTY_H */
