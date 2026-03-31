/* Test header for gengtype coverage - contains all type categories */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for struct/union types */
struct mytest_struct;
union mytest_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) mytest_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) mytest_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) mytest_struct {
  mytest_scalar_t field1;
  int * GTY((skip)) field2;  /* Skip this field for GC */
};

/* TYPE_USER_STRUCT: User-defined struct (forward declared then defined) */
struct GTY((user)) mytest_user_struct {
  int data;
  struct mytest_struct * GTY((tag("0"))) link;
};

/* TYPE_UNION: Union type */
union GTY(()) mytest_union {
  int ival;
  double dval;
  struct mytest_struct * GTY((tag("1"))) sptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct mytest_struct * GTY(()) mytest_pointer;
extern union mytest_union * GTY((length("sizeof(union mytest_union)"))) mytest_union_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) mytest_array[10];
extern struct mytest_struct * GTY((length("5"))) mytest_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) mytest_callback_fn)(int, struct mytest_struct *);
extern mytest_callback_fn GTY(()) mytest_callback;

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((desc("%0.type"), chain_next("%0.next"))) mytest_lang_struct {
  int type;
  struct mytest_lang_struct *next;
  struct mytest_struct *data;
};

/* Nested types to ensure thorough processing */
struct GTY(()) mytest_container {
  /* Contains one of each type category */
  mytest_scalar_t scalar_field;
  const char * GTY((length("strlen(%h.scalar_field ? \"yes\" : \"no\")"))) string_field;
  struct mytest_struct struct_field;
  union mytest_union union_field;
  struct mytest_struct *pointer_field;
  int array_field[5];
  mytest_callback_fn callback_field;
  struct mytest_lang_struct *lang_struct_field;
};

/* Variable declarations using our types */
extern struct mytest_container GTY(()) mytest_global_container;
extern struct mytest_lang_struct * GTY((chain_next("%h.next"))) mytest_lang_chain;

#endif /* MYTEST_GTY_H */
