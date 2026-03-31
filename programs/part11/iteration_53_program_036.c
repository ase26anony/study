/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/opaque type */
struct opaque_struct;

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  double b;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  void *data;
  int tag;
};

/* TYPE_UNION */
union my_union {
  int i;
  float f;
  char *str;
};

/* TYPE_POINTER */
struct with_pointers {
  int *int_ptr;
  struct my_struct *struct_ptr;
  union my_union *union_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
  int int_arr[10];
  char *str_arr[5];
  struct my_struct struct_arr[3];
};

/* TYPE_LANG_STRUCT - simulate GCC language-specific type */
struct GTY((desc("%1.type"), tag("TREE_CODE"))) lang_struct {
  enum tree_code type;
  union tree_node * GTY((refless)) args;
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long size_type;

/* TYPE_STRING */
struct with_strings {
  char *name;
  const char *path;
  char buffer[256];
};

/* TYPE_CALLBACK */
typedef void (*callback_func)(int, void*);
typedef int (*compare_func)(const void*, const void*);

/* Composite type using all kinds */
struct GTY(()) composite_type {
  /* scalar */
  int id;
  
  /* struct */
  struct my_struct nested;
  
  /* union */
  union my_union variant;
  
  /* pointer */
  struct composite_type *next;
  
  /* array */
  callback_func handlers[5];
  
  /* string */
  char *description;
  
  /* user struct */
  struct user_struct user_data;
  
  /* lang struct */
  struct lang_struct *lang_data;
};

/* Function pointer usage */
struct callback_container {
  callback_func on_start;
  callback_func on_end;
  compare_func compare;
};

/* Array of pointers */
struct pointer_array {
  void *items[20];
  struct my_struct *structs[10];
};

/* Nested structures */
struct outer_struct {
  struct {
    int x;
    int y;
  } GTY((skip)) coordinates;  /* skip for user struct test */
  
  struct inner_struct {
    int depth;
    struct outer_struct *parent;
  } *inner;
};

/* Enum type */
enum my_enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
};

/* Struct with enum */
struct enum_container {
  enum my_enum choice;
  int value;
};
