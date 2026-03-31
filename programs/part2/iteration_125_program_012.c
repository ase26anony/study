/* Test types for gengtype coverage testing.
   This file defines types corresponding to all type_kind enum cases
   to ensure gengtype processes each category. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String pointer typedefs */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%0)"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((user)) my_user_struct_1 {
  int field1;
  void *field2;
};

struct GTY((user)) my_user_struct_2 {
  long id;
  const char *name;
};

/* TYPE_STRUCT: Regular structs with GTY markers */
struct GTY(()) my_struct_1 {
  int x;
  double y;
  const char *z;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  int value;
  struct my_struct_2 * GTY((skip)) next;
  struct my_struct_2 *prev;
};

struct GTY((desc("%0.type"))) my_struct_3 {
  enum { TYPE_A, TYPE_B } type;
  union {
    int int_val;
    double dbl_val;
  } data;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_type_1 {
  void *data;
  int tag;
};

struct GTY((user)) user_struct_type_2 {
  unsigned long handle;
  int (*callback)(void);
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((desc("%d.type"))) my_union_2 {
  struct {
    int type;
    int value;
  } tagged;
  long raw;
};

union GTY((user)) my_union_3 {
  int i;
  float f;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY((skip)) my_pointer_2;
typedef my_scalar_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef const char * GTY((length("strlen(%0[i])"))) my_array_3[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, double);
typedef int (*my_callback_2)(const char *);
typedef void (* GTY((skip)) my_callback_3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_specific;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%0.tag"), chain_next="%h.link")) lang_struct_type_2 {
  enum { TAG_A, TAG_B, TAG_C } tag;
  union {
    int num;
    const char *str;
  } value;
  struct lang_struct_type_2 *link;
};

/* Additional struct to increase count */
struct GTY(()) extra_struct_1 {
  my_scalar_1 id;
  my_string_1 name;
  my_array_1 values;
};

struct GTY((user)) extra_user_struct_1 {
  void *opaque;
  int refcount;
};

/* Additional union to increase count */
union GTY(()) extra_union_1 {
  my_callback_1 cb;
  my_pointer_1 ptr;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
