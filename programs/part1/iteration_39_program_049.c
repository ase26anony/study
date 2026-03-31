/* Test header for gengtype coverage - covers all type categories */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Plain struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;  /* skip annotation for variety */
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  int data;
  /* User-defined markers would be defined elsewhere */
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((atomic)) str_val;  /* atomic annotation */
};

/* TYPE_POINTER: Pointer to struct with GTY annotation */
typedef struct my_test_struct * GTY(()) my_struct_ptr;

/* TYPE_ARRAY: Array with GTY annotation */
extern int GTY((length("my_array_len"))) my_test_array[];
extern size_t my_array_len;

/* TYPE_ARRAY: Fixed-size array */
struct GTY(()) array_container {
  int GTY((tag("0"))) fixed_array[10];  /* tag annotation */
};

/* TYPE_CALLBACK: Function pointer with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.lang_code"))) lang_specific_struct {
  int lang_code;
  void * GTY((skip)) language_data;
};

/* TYPE_POINTER: Chain of pointers for more coverage */
struct GTY(()) pointer_chain {
  struct pointer_chain * GTY((skip)) next;
  struct pointer_chain * GTY((chain_next("%h.next"))) chain_next_field;
};

/* TYPE_STRUCT: Nested structures */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int inner_field;
  } inner;
  
  union GTY(()) inner_union {
    int a;
    char b;
  } u;
};

/* TYPE_ARRAY: Array of pointers */
struct GTY(()) tree_node * GTY((length("num_nodes"))) node_array[];
extern int num_nodes;

/* TYPE_CALLBACK: More complex callback signature */
typedef int (*GTY(()) complex_callback)(struct my_test_struct *, 
                                        my_callback_fn, 
                                        int GTY((skip)));

/* TYPE_UNION: Tagged union */
union GTY((tag("TYPE"))) tagged_union {
  int type;
  struct {
    int type;  /* Must be first to match tag */
    int value;
  } integer;
  struct {
    int type;  /* Must be first to match tag */
    double value;
  } real;
};

/* TYPE_UNDEFINED: Forward declaration that might be used */
struct GTY(()) forward_declared_struct;

/* Test variable declarations using our types */
extern struct my_test_struct GTY(()) global_struct;
extern union my_test_union GTY(()) global_union;
extern my_callback_fn GTY(()) global_callback;

/* Parameterized structure */
struct GTY(()) param_struct {
  int GTY((param_is("struct my_test_struct *"))) is_param;
  void *data;
};

#endif /* MYTEST_GTY_H */
