/* Test coverage file for gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *);

/* TYPE_ENUM (handled as TYPE_SCALAR) */
enum my_enum {
  E1,
  E2,
  E3
};

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
  int field1;
  float field2;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
  void *data;
  int id;
};

/* TYPE_UNION: Union type */
union my_union {
  int a;
  float b;
  void *c;
};

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef struct plain_struct* struct_ptr;

/* TYPE_ARRAY: Array types */
int fixed_array[10];
extern int variable_array[];

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  void *ptr_field;
  
  /* TYPE_STRING */
  const char *name;
  
  /* TYPE_ARRAY (flexible array member) */
  int flexible_array[];
};

/* Another GC struct with nested structures */
struct GTY(()) container {
  /* TYPE_STRUCT (nested) */
  struct inner_struct {
    int x;
    int y;
  } inner;
  
  /* TYPE_UNION */
  union {
    int int_val;
    float float_val;
  } value;
  
  /* TYPE_POINTER to callback */
  callback_type callback;
  
  /* TYPE_ARRAY of pointers */
  void *ptr_array[5];
  
  /* TYPE_SCALAR */
  enum my_enum enum_field;
};

/* Union with GTY marker */
union GTY(()) tagged_union {
  int as_int;
  float as_float;
  struct container * GTY((tag("0"))) as_container;
};

/* Array of structs */
struct GTY(()) array_element {
  int id;
  const char *label;
};

typedef struct array_element element_array[20];

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Struct with callback field */
struct GTY(()) has_callback {
  callback_type handler;
  void *context;
};

/* Opaque pointer type */
typedef struct opaque_struct *opaque_ptr;

/* Mixed types in a union */
union GTY((desc ("%1.type"), tag ("0"))) variant {
  int type;
  struct container * GTY((tag ("1"))) container;
  struct complex_struct * GTY((tag ("2"))) complex;
  string_type GTY((tag ("3"))) str;
};

/* Additional scalar types */
typedef _Bool bool_type;
typedef long long int64_type;
typedef unsigned long long uint64_type;

/* Function pointer with complex signature */
typedef void (*complex_callback)(struct container *, int, const char *);

/* Struct containing all major types */
struct GTY(()) all_types {
  /* TYPE_SCALAR */
  int scalar_int_field;
  enum my_enum enum_field;
  
  /* TYPE_STRUCT */
  struct plain_struct plain;
  
  /* TYPE_POINTER */
  void *pointer_field;
  int_ptr int_pointer;
  
  /* TYPE_ARRAY */
  int fixed_size[8];
  char string_buffer[256];
  
  /* TYPE_STRING */
  const char *string_field;
  
  /* TYPE_CALLBACK */
  callback_type callback_field;
  
  /* TYPE_UNION */
  union {
    int option_a;
    float option_b;
  } choice;
  
  /* Reference to lang struct */
  struct lang_specific *lang_ptr;
};

/* Chain of pointers for depth testing */
struct GTY(()) node {
  int value;
  struct node * GTY((skip)) next;
  struct node *prev;
};

/* Template-like structure with conditional fields */
struct GTY(()) conditional_struct {
  int type;
  union {
    int int_data;
    float float_data;
    void *ptr_data;
  } GTY((desc ("%0.type"))) data;
};

/* For testing TYPE_UNDEFINED more thoroughly */
struct undefined_type;
typedef struct undefined_type *undefined_ptr;

/* Another way to get TYPE_UNDEFINED */
typedef void void_type;

/* Array of undefined pointers */
struct undefined_type *undefined_array[5];

/* Struct with undefined member */
struct GTY(()) has_undefined {
  int defined_field;
  struct undefined_type *undefined_field;
};

/* Multiple levels of indirection */
typedef struct container ***triple_ptr;

/* Callback returning pointer */
typedef struct container *(*allocator_callback)(int size);

/* Struct with array of callbacks */
struct GTY(()) callback_container {
  allocator_callback allocators[4];
  complex_callback processors[2];
};

/* Ensure we cover parameterized types */
struct GTY(()) param_struct {
  /* This should generate appropriate type handling */
  struct GTY((hwm)) high_water_mark {
    int current;
    int maximum;
  } hwm;
};

/* Final catch-all structure referencing everything */
struct GTY(()) master_type {
  struct all_types all;
  struct container container;
  union variant variant;
  struct node *list;
  struct callback_container callbacks;
  struct has_undefined with_undefined;
  triple_ptr triple;
  element_array elements;
  string_type strings[3];
  callback_type handlers[5];
};
