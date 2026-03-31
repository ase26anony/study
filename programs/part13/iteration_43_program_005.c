/* Test coverage file for gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_UNDEFINED: void type */
typedef void undefined_type;

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
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_ARRAY: Fixed-size array */
typedef int fixed_array_type[10];

/* TYPE_ARRAY: Multi-dimensional array */
typedef int matrix_type[5][5];

/* TYPE_POINTER: Simple pointer types */
typedef int *int_ptr;
typedef void *generic_ptr;

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_UNION: Plain union without GTY marker */
union plain_union {
  int int_val;
  float float_val;
  void *ptr_val;
  char str_val[20];
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
  void *data;
  int id;
  callback_type callback;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"), length("lang_length"))) lang_specific {
  int lang_field;
  void *lang_data;
};

/* TYPE_STRUCT: Another struct with GTY marker for GC */
struct GTY(()) gc_struct {
  int count;
  string_type name;
  int_ptr numbers;
  fixed_array_type array;
  struct gc_struct *next;  /* TYPE_POINTER within struct */
};

/* TYPE_UNION: Union with GTY marker */
union GTY(()) gc_union {
  int as_int;
  float as_float;
  void *as_ptr;
  struct gc_struct *as_struct;
};

/* TYPE_ARRAY: Variable-length array in a GTY struct */
struct GTY(()) var_array_struct {
  int length;
  int data[1];  /* Flexible array member */
};

/* TYPE_ARRAY: Array of pointers */
typedef struct gc_struct *struct_ptr_array[5];

/* TYPE_POINTER: Pointer to function pointer */
typedef callback_type *callback_ptr;

/* TYPE_POINTER: Pointer to array */
typedef int (*array_ptr)[10];

/* TYPE_POINTER: Complex nested pointer */
typedef struct gc_struct **double_ptr;

/* TYPE_STRUCT: Struct containing union */
struct GTY(()) struct_with_union {
  int type;
  union {
    int int_val;
    float float_val;
    void *ptr_val;
  } value;
  callback_type handler;
};

/* TYPE_UNION: Union containing struct */
union GTY(()) union_with_struct {
  struct {
    int x;
    int y;
  } point;
  struct {
    float r;
    float g;
    float b;
  } color;
};

/* TYPE_ARRAY: Array within union */
union GTY(()) union_with_array {
  int ints[4];
  float floats[4];
  char chars[16];
};

/* TYPE_STRING: String array */
typedef const char *string_array[3];

/* TYPE_CALLBACK: Callback with complex signature */
typedef void (*complex_callback)(struct gc_struct *, union gc_union *, int);

/* TYPE_POINTER: Pointer to callback */
typedef complex_callback *callback_pointer;

/* TYPE_STRUCT: Final complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
  /* TYPE_SCALAR */
  int id;
  
  /* TYPE_STRING */
  const char *description;
  
  /* TYPE_POINTER */
  struct complex_nested *next;
  
  /* TYPE_ARRAY */
  int values[8];
  
  /* TYPE_UNION */
  union {
    /* TYPE_POINTER */
    void *data;
    /* TYPE_SCALAR */
    long counter;
  } storage;
  
  /* TYPE_CALLBACK */
  callback_type notify;
  
  /* TYPE_ARRAY of pointers */
  struct gc_struct *items[4];
  
  /* TYPE_POINTER to array */
  int (*matrix)[3][3];
  
  /* TYPE_USER_STRUCT reference */
  struct user_struct *user_data;
  
  /* TYPE_LANG_STRUCT reference */
  struct lang_specific *lang_data;
};

/* TYPE_ARRAY: Array of complex structures */
typedef struct complex_nested nested_array[2];

/* TYPE_POINTER: Pointer to undefined type */
struct opaque_struct *opaque_ptr;

/* TYPE_CALLBACK: Callback returning pointer */
typedef struct gc_struct *(*allocator_callback)(int size);

/* TYPE_STRUCT: Struct with all type kinds */
struct GTY(()) all_types_struct {
  /* TYPE_SCALAR */
  enum my_enum enum_field;
  
  /* TYPE_STRING */
  const char *string_field;
  
  /* TYPE_POINTER */
  void *pointer_field;
  
  /* TYPE_ARRAY */
  int array_field[5];
  
  /* TYPE_CALLBACK */
  callback_type callback_field;
  
  /* TYPE_STRUCT */
  struct plain_struct plain_field;
  
  /* TYPE_UNION */
  union plain_union union_field;
  
  /* TYPE_USER_STRUCT */
  struct user_struct *user_field;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific *lang_field;
  
  /* TYPE_POINTER to callback */
  callback_type *callback_ptr_field;
  
  /* TYPE_ARRAY of strings */
  const char *string_array_field[3];
  
  /* TYPE_POINTER to undefined */
  struct opaque_struct *opaque_field;
};
