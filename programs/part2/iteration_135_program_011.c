/* test-gty.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_SCALAR - Basic scalar types */
typedef enum {
    RED,
    GREEN,
    BLUE
} color_enum;

typedef bool flag_type;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRING - String type */
typedef const char *string_type;

/* TYPE_USER_STRUCT - User-defined structure with special handling */
typedef struct user_def {
    int id;
    string_type name;
} user_def_t;

#endif /* TEST_GTY_H */
