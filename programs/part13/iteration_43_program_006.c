/* test-coverage.gt - Comprehensive test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[][20];

/* TYPE_UNION: Union type */
union my_union {
  int a;
  void *b;
  float c;
};

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user struct */
struct GTY((user)) user_struct {
  void *data;
  int id;
  callback_type callback;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
  enum my_enum lang_enum;
};

/* Complex nested structures to ensure deep processing */

/* A GC-tracked struct containing various type combinations */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  void *ptr;
  
  /* TYPE_ARRAY (fixed size) */
  int numbers[5];
  
  /* TYPE_STRING */
  const char *name;
  
  /* TYPE_CALLBACK */
  callback_type handler;
  
  /* TYPE_UNION */
  union my_union data_union;
  
  /* TYPE_SCALAR */
  enum my_enum status;
  
  /* TYPE_POINTER to another struct */
  struct plain_struct *plain;
  
  /* TYPE_POINTER to user struct */
  struct user_struct *user;
  
  /* TYPE_POINTER to lang struct */
  struct lang_specific *lang;
};

/* Another struct with flexible array member */
struct GTY(()) flexible_struct {
  int count;
  int data GTY((length("%h.count"))) [];
};

/* Union containing pointers */
union pointer_union {
  void *any_ptr;
  int *int_ptr;
  struct complex_struct *complex_ptr;
};

/* Struct with nested arrays and pointers */
struct GTY(()) nested_arrays {
  /* Multi-dimensional array */
  int matrix[3][3];
  
  /* Array of pointers */
  void *pointers[10];
  
  /* Pointer to array */
  int (*array_ptr)[5];
  
  /* Array of structs */
  struct plain_struct items[4];
};

/* Callback struct that uses all features */
struct GTY(()) callback_container {
  /* Multiple callback types */
  callback_type on_start;
  int (*on_data)(const char *, size_t);
  void (*on_end)(struct complex_struct *);
  
  /* String array */
  const char *messages[5];
  
  /* Union with callback */
  union {
    callback_type cb;
    void *data;
  } handler;
};

/* Opaque pointer typedef */
typedef struct opaque_struct *GTY((user)) opaque_handle;

/* Array of callbacks */
typedef void (*callback_array[10])(int);

/* Struct with bitfields (scalar type) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int value : 16;
};

/* Self-referential structure */
struct GTY(()) tree_node {
  int type;
  struct tree_node *GTY((skip)) left;
  struct tree_node *GTY((skip)) right;
  struct tree_node *parent;
};

/* Union with struct members */
union complex_union {
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
  callback_type action;
};

/* Final catch-all structure that references everything */
struct GTY(()) master_type {
  /* All basic types */
  scalar_int int_val;
  scalar_float float_val;
  string_type str_val;
  
  /* All compound types */
  struct plain_struct plain;
  struct user_struct *user;
  struct lang_specific lang;
  union my_union uni;
  struct complex_struct *complex;
  struct flexible_struct *flex;
  struct nested_arrays arrays;
  struct callback_container callbacks;
  struct bitfield_struct bits;
  struct tree_node *tree;
  union complex_union cu;
  
  /* Arrays of various types */
  int int_array[20];
  struct plain_struct struct_array[5];
  callback_type callback_array[3];
  
  /* Special cases */
  opaque_handle handle;
  callback_array cb_array;
  
  /* Void pointer for generic data */
  void *generic_data;
  
  /* Function pointer with complex signature */
  int (*complex_callback)(struct master_type *, const char *, ...);
};
