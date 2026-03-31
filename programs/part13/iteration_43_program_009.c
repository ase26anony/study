/* test-coverage.gt - Test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_UNDEFINED: void type */
typedef void void_type;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_SCALAR: Enum type */
enum my_enum {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
};

/* TYPE_STRING: String types */
typedef const char *string_type;
typedef char *mutable_string_type;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char *, void *);
typedef void (*void_callback)(void);

/* TYPE_POINTER: Simple pointer types */
typedef int *int_ptr;
typedef void **void_ptr_ptr;

/* TYPE_ARRAY: Fixed-size array */
typedef int fixed_array[10];
typedef char string_array[5][20];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware struct */
struct GTY((user)) user_struct {
  void *data;
  int id;
  const char *name;
};

/* TYPE_UNION: Plain union */
union my_union {
  int int_val;
  float float_val;
  void *ptr_val;
  char str_val[20];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"), chain_next("next"), chain_prev("prev"))) lang_specific {
  int lang_field;
  void *lang_data;
  struct lang_specific *next;
  struct lang_specific *prev;
};

/* TYPE_POINTER within GTY struct */
struct GTY(()) pointer_struct {
  void *ptr_field;
  int *int_ptr_field;
  struct plain_struct *struct_ptr;
};

/* TYPE_ARRAY within GTY struct */
struct GTY(()) array_struct {
  int fixed_size[5];
  char variable_length[];
};

/* TYPE_UNION within GTY struct */
struct GTY(()) union_container {
  int type;
  union {
    int int_val;
    void *ptr_val;
    float float_val;
  } data;
};

/* TYPE_CALLBACK within GTY struct */
struct GTY(()) callback_container {
  simple_callback cb1;
  complex_callback cb2;
  void (*inline_cb)(int, float);
};

/* TYPE_STRING within GTY struct */
struct GTY(()) string_container {
  const char *const_string;
  char *mutable_string;
  string_type typedef_string;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
  /* TYPE_POINTER to TYPE_USER_STRUCT */
  struct user_struct *user_ptr;
  
  /* TYPE_ARRAY of TYPE_POINTER */
  void *ptr_array[8];
  
  /* TYPE_UNION with various types */
  union {
    int int_member;
    float float_member;
    struct plain_struct *struct_ptr;
  } nested_union;
  
  /* TYPE_CALLBACK field */
  void (*nested_callback)(struct complex_nested *);
  
  /* TYPE_STRING field */
  const char *description;
  
  /* TYPE_SCALAR fields */
  int counter;
  enum my_enum status;
  
  /* TYPE_ARRAY of TYPE_SCALAR */
  int scores[5];
  
  /* Flexible array member (TYPE_ARRAY) */
  char dynamic_data[];
};

/* Another GTY struct with chain for garbage collection */
struct GTY((chain_next("next"))) linked_node {
  int value;
  struct linked_node *next;
  struct linked_node *prev;
};

/* Union with GTY marker */
union GTY(()) gty_union {
  int as_int;
  void *as_ptr;
  float as_float;
};

/* Array of structs */
struct GTY(()) array_of_structs {
  struct plain_struct elements[3];
  struct user_struct *user_elements[2];
};

/* Struct containing all type kinds */
struct GTY(()) all_types_container {
  /* TYPE_SCALAR */
  int scalar_field;
  float float_field;
  enum my_enum enum_field;
  
  /* TYPE_STRING */
  const char *string_field;
  
  /* TYPE_POINTER */
  void *pointer_field;
  int *int_pointer_field;
  struct plain_struct *struct_pointer_field;
  
  /* TYPE_ARRAY */
  int int_array[10];
  char char_array[20];
  
  /* TYPE_UNION */
  union {
    int union_int;
    void *union_ptr;
  } union_field;
  
  /* TYPE_CALLBACK */
  void (*callback_field)(int);
  
  /* TYPE_STRUCT (embedded) */
  struct plain_struct embedded_struct;
  
  /* TYPE_USER_STRUCT (pointer) */
  struct user_struct *user_struct_ptr;
  
  /* TYPE_LANG_STRUCT (pointer) */
  struct lang_specific *lang_struct_ptr;
};

/* Template-like structure for additional coverage */
struct GTY(()) template_container {
  /* Nested anonymous struct */
  struct {
    int x;
    int y;
  } point;
  
  /* Array of pointers to callbacks */
  void (*callbacks[5])(void);
  
  /* Multi-dimensional array */
  int matrix[3][3];
};

/* Final catch-all structure */
struct GTY((final)) final_struct {
  int magic_number;
  struct all_types_container *container;
  union gty_union final_union;
  void (*finalize_callback)(struct final_struct *);
};
