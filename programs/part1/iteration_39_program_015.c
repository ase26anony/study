/* Test header with diverse GTY-annotated types for gengtype coverage */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
};

/* TYPE_USER_STRUCT: User-defined struct (often used with tag) */
typedef struct GTY((tag("USER_STRUCT"))) my_user_struct {
  int data;
  void * GTY((skip)) extra;
} my_user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((skip)) str_val;
};

/* TYPE_POINTER: Pointer type */
extern struct my_test_struct * GTY(()) my_struct_pointer;

/* TYPE_ARRAY: Array type */
extern int GTY((length("my_array_len"))) my_test_array[];
extern unsigned int my_array_len;

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), chain_next("%0.next"))) lang_test_struct {
  int type;
  struct lang_test_struct *next;
  void *data;
};

/* Nested types to ensure thorough processing */
struct GTY(()) container_struct {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  struct my_test_struct *struct_ptr; /* TYPE_POINTER to TYPE_STRUCT */
  union my_test_union union_field;   /* TYPE_UNION */
  my_callback_fn callback;           /* TYPE_CALLBACK */
  
  /* Array field */
  int GTY((length("array_len"))) dynamic_array[];
  int array_len;
};

/* Forward declaration for pointer types */
struct GTY(()) forward_declared;
struct GTY(()) another_struct {
  struct forward_declared * GTY((skip)) fwd_ptr;
};

/* Complete the forward declaration */
struct GTY(()) forward_declared {
  int value;
  struct another_struct *link;
};

/* Global variables using these types */
extern struct my_test_struct GTY(()) global_struct;
extern union my_test_union GTY(()) global_union;
extern my_callback_fn GTY(()) global_callback;

#endif /* MYTEST_GTY_H */
