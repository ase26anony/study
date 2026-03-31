/* test-gty.h - Comprehensive GTY test types for coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_SCALAR - Basic scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef bool flag_t;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_fn)(void *data);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_STRING - String type */
typedef const char *string_t;

/* TYPE_USER_STRUCT - User-defined struct with special handling */
typedef struct user_data {
    int id;
    string_t name;
} user_data_t;

#endif /* TEST_GTY_H */
