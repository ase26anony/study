/* Primary header file for GTY type classification coverage test */
#ifndef TEST_GTY_H
#define TEST_GTY_H

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

/* Forward declarations for undefined types */
struct GTY(()) undefined_struct;
typedef struct undefined_struct *GTY(()) undefined_ptr_t;

/* Additional undefined type to trigger TYPE_UNDEFINED */
struct GTY(()) another_undefined;

#endif /* TEST_GTY_H */
