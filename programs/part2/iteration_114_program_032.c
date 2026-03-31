#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_UNDEFINED: Forward declaration that won't be defined */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY((user));

/* TYPE_STRING: String type with length attribute */
struct string_container {
  char * GTY((length("strlen($1)"))) str_field;
  int length;
};

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY((tag("base"))) {
  my_scalar_t scalar_field;
  struct base_struct *next GTY((skip));
};

/* Nested struct for complexity */
struct outer_struct GTY((tag("outer"))) {
  struct base_struct inner GTY((skip));
  int count;
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled_struct GTY((user)) {
  int user_data;
  void *user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY((tag("data_union"))) {
  int int_val;
  char * GTY((length("strlen($1)"))) str_val;
  struct base_struct *struct_ptr;
};

/* TYPE_POINTER: Struct with pointer fields creating circular references */
struct pointer_network GTY((tag("network"))) {
  struct pointer_network *self_ptr;
  struct pointer_network *next GTY((skip));
  struct undefined_struct *forward_ptr;  /* TYPE_UNDEFINED reference */
  union data_union *union_ptr;
};

/* TYPE_ARRAY: Structs with various array types */
struct array_container GTY((tag("arrays"))) {
  /* Fixed-size array */
  int fixed_array[10];
  
  /* Zero-length array */
  char flexible_array[0];
  
  /* Array with length attribute */
  struct base_struct * GTY((length("$1->count"))) ptr_array;
  int count;
  
  /* Nested array in struct */
  struct {
    int matrix[3][3];
  } nested;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void *) GTY((callback));

struct callback_container {
  callback_func_t handler GTY((skip));
  void *user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((tag("lang"), lang_struct (1))) {
  int lang_data;
  void *lang_pointer;
};

/* Complex nested type combining multiple kinds */
struct master_container GTY((tag("master"))) {
  /* TYPE_STRUCT */
  struct base_struct base;
  
  /* TYPE_UNION */
  union data_union data;
  
  /* TYPE_POINTER (circular) */
  struct master_container *next;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_STRING */
  char * GTY((length("strlen($1)"))) name;
  
  /* TYPE_CALLBACK */
  callback_func_t callback;
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_specific_struct *lang_ptr;
  
  /* TYPE_USER_STRUCT */
  struct user_handled_struct user;
};

/* Additional forward declarations for more TYPE_UNDEFINED cases */
struct another_undefined GTY((tag("another_undef")));
union undefined_union GTY((tag("undef_union")));

#endif /* TEST_GENGTYPE_H */
