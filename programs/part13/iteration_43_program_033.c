/* test-coverage.gt - Test file for covering gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

enum my_enum {
  E1,
  E2,
  E3
};

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char small_array[5];

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user-defined structure */
struct GTY((user)) user_struct {
  void *data;
  int id;
  callback_type callback;
};

/* TYPE_UNION: Union type */
union my_union {
  int a;
  float b;
  void *c;
  string_type d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
  enum my_enum lang_enum;
};

/* Complex nested types to ensure deep processing */

/* A GTY structure containing various nested types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER field */
  void *ptr_field;
  
  /* TYPE_ARRAY field */
  int array_field[8];
  
  /* TYPE_STRING field */
  const char *name;
  
  /* TYPE_SCALAR fields */
  int count;
  enum my_enum status;
  
  /* Nested TYPE_STRUCT */
  struct inner_struct {
    int inner_field;
    float inner_float;
  } inner;
  
  /* TYPE_UNION field */
  union my_union data_union;
  
  /* TYPE_CALLBACK field */
  callback_type handler;
};

/* Another GTY structure with flexible array member */
struct GTY(()) struct_with_flexarray {
  int length;
  char data[];
};

/* Union containing various pointer types */
union GTY(()) pointer_union {
  int *int_ptr;
  void **void_ptr_ptr;
  struct complex_struct *struct_ptr;
  callback_type func_ptr;
};

/* Array of pointers */
typedef struct complex_struct *struct_ptr_array[5];

/* Structure containing array of callbacks */
struct GTY(()) callback_container {
  int num_callbacks;
  callback_type callbacks[3];
};

/* Forward declaration for circular reference */
struct forward_declared;

/* Structure with pointer to forward-declared type */
struct GTY(()) circular_ref {
  int id;
  struct forward_declared *next;
};

/* Definition of forward-declared structure */
struct GTY(()) forward_declared {
  int value;
  struct circular_ref *prev;
};

/* Mixed structure with all types */
struct GTY(()) all_types_struct {
  /* TYPE_SCALAR */
  int scalar_int_field;
  enum my_enum enum_field;
  
  /* TYPE_POINTER */
  void *pointer_field;
  int *int_pointer_field;
  
  /* TYPE_STRING */
  const char *string_field;
  
  /* TYPE_ARRAY */
  int int_array[10];
  char char_array[20];
  
  /* TYPE_STRUCT (nested) */
  struct {
    int nested_int;
    float nested_float;
  } nested_struct;
  
  /* TYPE_UNION */
  union {
    int union_int;
    float union_float;
    void *union_ptr;
  } data_union;
  
  /* TYPE_CALLBACK */
  callback_type callback_field;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_struct *user_struct_ptr;
  
  /* Pointer to TYPE_LANG_STRUCT */
  struct lang_specific *lang_struct_ptr;
};

/* Typedef for a function type (another form of callback) */
typedef int (comparison_func)(const void *, const void *);

/* Structure using function type typedef */
struct GTY(()) sortable_array {
  void **elements;
  int size;
  comparison_func *compare;
};

/* Opaque pointer type */
typedef struct opaque_struct *opaque_ptr;

/* Self-referential structure */
struct GTY(()) tree_node {
  int type;
  struct tree_node *left;
  struct tree_node *right;
  struct tree_node *parent;
};

/* Structure with bitfields (scalar type) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  unsigned int value : 26;
};

/* Union with nested structure */
union GTY(()) nested_union {
  struct {
    int x;
    int y;
  } point;
  struct {
    float r;
    float g;
    float b;
    float a;
  } color;
};

/* Array of unions */
typedef union nested_union union_array[4];

/* Final structure that references many other types */
struct GTY(()) master_container {
  struct all_types_struct *all_types;
  struct complex_struct complex;
  union pointer_union pointers;
  struct callback_container callbacks;
  struct tree_node *tree_root;
  struct bitfield_struct flags;
  union nested_union graphics_data;
  union_array color_palette;
  string_type description;
  callback_type on_update;
  struct forward_declared *chain_start;
  struct circular_ref *chain_link;
  opaque_ptr hidden_data;
};
