#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT - Basic struct with GTY annotation */
struct GTY(()) base_struct {
  int x;
  double y;
};

/* TYPE_UNION - Basic union with GTY annotation */
union GTY(()) base_union {
  int as_int;
  double as_double;
  void* as_ptr;
};

/* TYPE_POINTER - Struct containing pointers */
struct GTY(()) pointer_container {
  /* Regular pointer */
  struct base_struct* GTY((skip)) regular_ptr;
  
  /* Pointer with length specifier */
  int* GTY((length("len"))) variable_len_ptr;
  unsigned len;
};

/* TYPE_ARRAY - Struct with arrays */
struct GTY(()) array_container {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_arr[10];
  
  /* Variable-length array */
  char GTY((length("str_len"))) variable_arr[1];
  size_t str_len;
};

/* TYPE_SCALAR - Direct scalar types with GTY */
typedef long GTY((skip)) counter_type;
typedef unsigned GTY((skip)) flags_type;

/* TYPE_STRING - String types */
struct GTY(()) string_container {
  const char* GTY((skip)) constant_string;
  char* GTY((skip)) mutable_string;
};

/* TYPE_CALLBACK - Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

/* Complex nested structure for type graph */
struct GTY(()) complex_node {
  struct complex_node* GTY((skip)) next;
  struct complex_node* GTY((skip)) prev;
  union base_union GTY((tag("TYPE_UNION"))) data;
  struct array_container arrays;
};

/* Template-like macro for generating multiple structs */
#define DEF_PAIR(T) struct pair_##T { \
  T first; \
  T second; \
} GTY(())

/* Instantiate template-like structs */
DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct base_struct*);

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl;
struct GTY(()) recursive_struct {
  struct forward_decl* GTY((skip)) fwd_ptr;
  struct recursive_struct* GTY((skip)) self_ptr;
};

struct GTY(()) forward_decl {
  struct recursive_struct* GTY((skip)) back_ptr;
  int value;
};

/* Language-specific structure simulation */
struct GTY((tag("TS_VAR_DECL"))) lang_specific_node {
  int decl_uid;
  struct lang_specific_node* GTY((skip)) chain;
  const char* GTY((skip)) name;
};

/* User-defined struct type reference */
#ifdef USER_STRUCT_ENABLED
struct user_defined_type* GTY((skip)) user_ref;
#endif

#endif /* TEST_GTY_H */
