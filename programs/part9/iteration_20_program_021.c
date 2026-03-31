/* test-gtype.h - Comprehensive test for gengtype type coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_unsigned_scalar_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT with various fields */
struct my_struct GTY(())
{
  /* Scalar fields */
  int field1;
  unsigned long field2;
  
  /* Pointer fields marked with % for GC */
  struct my_struct * GTY((skip)) next_skip;  /* Skip from GC */
  struct my_struct * GTY((tag("0"))) next_tagged;
  
  /* String field */
  const char * GTY((length("strlen(%h.field3) + 1"))) field3;
  
  /* Pointer to undefined type */
  struct undefined_struct * GTY((maybe_undef)) undefined_ptr;
  
  /* Array field */
  int GTY((length("field5_length"))) *field5;
  size_t field5_length;
};

/* TYPE_USER_STRUCT with custom marking */
struct user_struct GTY((user))
{
  void *custom_data;
  size_t data_size;
};

/* TYPE_UNION */
union my_union GTY((desc("type_tag")))
{
  int type_tag;
  
  struct {
    int int_value;
    double double_value;
  } GTY((tag("1"))) variant1;
  
  struct {
    char * GTY((length("str_len"))) string_value;
    size_t str_len;
  } GTY((tag("2"))) variant2;
  
  struct my_struct * GTY((tag("3"))) struct_ptr;
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  struct my_struct * GTY((length("count"))) elements;
  size_t count;
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  int GTY((length("10"))) fixed[10];
  struct user_struct GTY((length("5"))) users[5];
};

/* TYPE_POINTER - various pointer types */
typedef struct my_struct *my_struct_ptr;
typedef union my_union *union_ptr;
typedef int (*callback_func)(struct my_struct *, int);

/* TYPE_CALLBACK */
typedef int GTY((callback)) (*my_callback_t)(
  struct my_struct * GTY((skip)) context,
  const char *message,
  int value
);

/* Linked list using chain_next */
struct linked_node GTY(())
{
  int value;
  struct linked_node * GTY((chain_next("%h.next"))) next;
  struct linked_node * GTY((chain_prev("%h.prev"))) prev;
};

/* Nested structure for complex relationships */
struct complex_container GTY(())
{
  /* Pointer to union */
  union my_union * GTY((tag("container_type"))) data;
  
  /* Array of user structs */
  struct user_struct * GTY((length("user_count"))) users;
  size_t user_count;
  
  /* Callback function */
  my_callback_t callback;
  
  /* String array */
  const char * GTY((length("string_count"))) *strings;
  size_t string_count;
};

/* Now define the previously undefined struct */
struct undefined_struct GTY(())
{
  int defined_now;
  struct my_struct *ptr_to_struct;
};

/* Template-like structure with param_is */
struct template_struct GTY((param_is(T)))
{
  void * GTY((skip)) data;
  size_t size;
};

/* Language-specific structure (TYPE_LANG_STRUCT) */
#ifdef GENERATOR_FILE
/* This would normally be in a language-specific header */
struct c_tree_node GTY(())
{
  int code;
  struct c_tree_node * GTY((skip)) chain;
  union lang_tree_node *language_specific;
};
#endif

/* Discriminated union with desc */
struct discriminated GTY((desc("discriminator")))
{
  int discriminator;
  
  union {
    int as_int GTY((tag("0")));
    double as_double GTY((tag("1")));
    struct my_struct *as_struct GTY((tag("2")));
  } GTY((desc("discriminator"))) u;
};

#endif /* TEST_GTYPE_H */
