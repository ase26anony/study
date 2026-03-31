/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct forward_declared_struct;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct basic_struct GTY(())
{
  int x;
  double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union basic_union GTY(())
{
  int int_val;
  double double_val;
  void* ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(())
{
  /* Regular pointer */
  struct basic_struct* GTY((skip)) regular_ptr;
  
  /* Pointer to forward declared struct */
  struct forward_declared_struct* GTY((skip)) forward_ptr;
  
  /* Pointer to self (recursive type) */
  struct pointer_container* GTY((skip)) self_ptr;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(())
{
  /* Fixed-size array */
  int GTY((length("10"))) fixed_array[10];
  
  /* Variable-length array (requires length expression) */
  char* GTY((length("strlen($)"))) variable_array;
  
  /* Array of pointers */
  struct basic_struct* GTY((skip)) GTY((length("5"))) ptr_array[5];
};

/* TYPE_SCALAR: Direct GTY on scalar types */
long GTY((skip)) global_counter;

/* TYPE_STRING: String fields */
struct string_container GTY(())
{
  const char* GTY((skip)) constant_string;
  char* GTY((skip)) mutable_string;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(())
{
  callback_func GTY((skip)) handler;
  void* GTY((skip)) user_data;
};

/* Complex nested type for type graph testing */
struct complex_node GTY(())
{
  int id;
  struct complex_node* GTY((skip)) next;
  struct complex_node* GTY((skip)) prev;
  union basic_union GTY((tag("type"))) data;
};

/* Template-like macro for generating multiple structs */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct basic_struct*);

/* Forward declared struct definition */
struct forward_declared_struct GTY(())
{
  int magic;
  struct basic_struct* GTY((skip)) child;
};

#endif /* TEST_GTY_H */
