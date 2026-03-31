/* test-coverage.gt - Comprehensive test file to cover all gengtype TYPE_* cases */

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

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef void* void_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char small_array[5];

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user structure */
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
  callback_type d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
  enum my_enum lang_enum;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  void *ptr_field;
  
  /* TYPE_ARRAY (fixed size) */
  int array_field[20];
  
  /* TYPE_STRING */
  const char *name;
  
  /* TYPE_CALLBACK */
  callback_type handler;
  
  /* TYPE_SCALAR */
  int count;
  enum my_enum status;
  
  /* TYPE_UNION */
  union my_union data_union;
  
  /* Pointer to another GC type */
  struct user_struct * GTY((skip)) user_data;
  
  /* Pointer to language-specific struct */
  struct lang_specific * GTY((tag("LANG"))) lang_ptr;
};

/* Another struct with flexible array member (TYPE_ARRAY) */
struct GTY(()) flex_struct {
  int length;
  char data GTY((length("%0.length"))) [];
};

/* Struct containing pointer to callback */
struct GTY(()) callback_container {
  callback_type start_callback;
  callback_type end_callback;
  string_type description;
};

/* Union containing various pointer types */
union GTY(()) pointer_union {
  int_ptr int_pointer;
  void_ptr void_pointer;
  string_type string_pointer;
  struct complex_struct *struct_pointer;
};

/* Array of structs */
struct GTY(()) array_container {
  struct complex_struct elements[5];
  int count;
};

/* Struct with nested anonymous union */
struct GTY(()) nested_union_struct {
  int type;
  union {
    int int_value;
    float float_value;
    string_type string_value;
  } data;
};

/* Forward declaration that will be TYPE_UNDEFINED initially */
struct forward_declared;

/* Struct that references forward-declared type */
struct GTY(()) uses_forward {
  struct forward_declared *forward_ptr;
  int valid;
};

/* Now define the forward-declared struct */
struct GTY(()) forward_declared {
  int id;
  string_type name;
  struct uses_forward *back_ref;
};

/* Type with callback that returns a pointer */
typedef struct complex_struct* (*factory_callback)(int, string_type);

/* Struct using factory callback */
struct GTY(()) factory_user {
  factory_callback create;
  factory_callback destroy;
  void *context;
};

/* Mixed struct with both GTY and non-GTY fields */
struct GTY(()) mixed_struct {
  /* GTY-tracked fields */
  void * GTY((skip)) gc_data;
  string_type gc_name;
  
  /* Non-GTY fields (plain scalars) */
  int plain_int;
  float plain_float;
  
  /* Static array */
  char buffer[256];
};

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
  int hidden;
  void *secret;
};

/* Additional scalar typedefs */
typedef long long scalar_ll;
typedef unsigned long long scalar_ull;
typedef _Bool scalar_bool;

/* Additional string types */
typedef char *mutable_string;
typedef const char * const constant_string;

/* Complex callback signature */
typedef int (*complex_callback)(struct complex_struct *, union my_union, callback_type);

/* Final struct containing everything */
struct GTY(()) master_struct {
  /* All basic types */
  scalar_int int_field;
  scalar_float float_field;
  string_type string_field;
  callback_type callback_field;
  
  /* Composite types */
  struct complex_struct complex_field;
  union pointer_union union_field;
  struct lang_specific lang_field GTY((tag("LANG")));
  
  /* Arrays */
  fixed_array fixed_field;
  struct complex_struct struct_array[3];
  
  /* Special cases */
  struct user_struct *user_ptr;
  struct forward_declared *forward_field;
  struct flex_struct *flex_field;
  
  /* Nested anonymous struct */
  struct {
    int nested_int;
    void *nested_ptr;
  } anonymous_field;
};
