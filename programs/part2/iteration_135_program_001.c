/* test-gtype.h - Comprehensive test for all gengtype type categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef bool boolean_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_USER_STRUCT - requires typedef struct pattern */
typedef struct user_def {
  int id;
  char *name;
} user_def_t;

/* TYPE_POINTER - pointer type definition */
typedef struct base_struct *base_ptr;

#endif /* TEST_GTYPE_H */
