/* test-gtype.h - Comprehensive test of all gengtype type categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to create TYPE_UNDEFINED cases */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr;

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_unsigned_scalar_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_CALLBACK */
typedef void (*my_callback_t)(void *data, int value);
typedef int (*complex_callback_t)(struct my_base_struct *s, const char *str);

/* TYPE_USER_STRUCT */
struct GTY((user)) my_user_struct {
  int id;
  void *data;
};

/* User-defined marking function for my_user_struct */
void gt_ggc_mx_my_user_struct(void *p);

/* TYPE_STRUCT with various field types */
struct GTY(()) my_base_struct {
  int scalar_field;                     /* TYPE_SCALAR */
  const char * GTY((skip)) name;        /* TYPE_STRING with skip */
  struct my_base_struct *next;          /* TYPE_POINTER to same struct */
  struct my_nested_struct *nested;      /* TYPE_POINTER to another struct */
  my_callback_t callback;               /* TYPE_CALLBACK */
  struct my_user_struct *user_data;     /* TYPE_POINTER to USER_STRUCT */
  forward_ptr undefined_ptr;            /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Another struct for testing dependencies */
struct GTY(()) my_nested_struct {
  int value;
  struct my_base_struct *parent;
  struct my_union *variant;            /* TYPE_POINTER to UNION */
};

/* TYPE_UNION with discriminator */
struct GTY(()) my_union {
  int tag;
  union {
    int as_int;
    struct my_base_struct *as_struct;
    const char *as_string;
    struct GTY((tag("1"))) {
      struct my_nested_struct *nested;
    } variant1;
  } GTY((desc("tag"))) u;
};

/* TYPE_ARRAY examples */
struct GTY(()) array_container {
  int count;
  struct my_base_struct * GTY((length("count"))) items[];  /* Variable array */
};

struct GTY(()) fixed_array_container {
  struct my_nested_struct fixed[10];    /* Fixed array */
  struct my_user_struct *user_array[5]; /* Fixed array of pointers */
};

/* Linked list example using chain_next */
struct GTY(()) linked_list {
  int value;
  struct linked_list * GTY((chain_next("%h.next"))) next;
};

/* Parametrized structure */
struct GTY((param_is(T))) template_like {
  void * GTY((skip)) data;
  int (*compare)(void *a, void *b);
};

/* Language-specific structure (TYPE_LANG_STRUCT) */
#ifdef GENERATOR_FILE
struct GTY(()) c_tree_node {
  enum tree_code code;
  union tree_node * GTY((tag("0"))) operands[1];
};
#endif

/* Maybe undefined structure */
struct GTY((maybe_undef)) possibly_undefined {
  int value;
  struct forward_declared_struct *ptr;
};

#endif /* TEST_GTYPE_H */
