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
typedef int (*another_callback)(const char *, void *);

/* TYPE_ENUM: Enumeration type */
enum my_enum {
  ENUM_VALUE_1,
  ENUM_VALUE_2,
  ENUM_VALUE_3
};

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* void_ptr;
typedef struct plain_struct* struct_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[256];

/* TYPE_UNION: Union type */
union my_union {
  int int_field;
  float float_field;
  void *pointer_field;
  callback_type callback_field;
};

/* TYPE_STRUCT: Plain C struct (no GTY marker) */
struct plain_struct {
  int field1;
  float field2;
  char field3;
  fixed_array array_field;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
  void * GTY((skip)) data;
  int id;
  string_type name;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  enum my_enum lang_enum;
  void_ptr lang_pointer;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
  /* TYPE_POINTER field */
  struct plain_struct* GTY((skip)) plain_ptr;
  
  /* TYPE_ARRAY field */
  int GTY((length("array_len"))) variable_array[];
  
  /* TYPE_UNION field */
  union my_union union_field;
  
  /* TYPE_STRING field */
  const char* GTY((skip)) description;
  
  /* TYPE_CALLBACK field */
  callback_type handler;
  
  /* TYPE_SCALAR fields */
  int array_len;
  enum my_enum current_state;
  
  /* Nested TYPE_STRUCT */
  struct inner_struct {
    int inner_field;
    float inner_float;
  } inner;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_struct* GTY((skip)) user_data;
};

/* Another structure with flexible array member */
struct GTY(()) flexible_struct {
  int count;
  int GTY((length("count"))) items[];
};

/* Union containing various types */
union GTY(()) mixed_union {
  struct plain_struct plain;
  struct user_struct* user;
  int_ptr numbers;
  callback_type func;
  string_type str;
};

/* Array of pointers */
typedef struct GTY(()) pointer_array {
  void* GTY((skip)) elements[8];
} pointer_array_t;

/* Structure with callback array */
struct GTY(()) callback_container {
  int num_callbacks;
  callback_type GTY((length("num_callbacks"))) callbacks[];
};

/* Opaque pointer type for TYPE_UNDEFINED testing */
typedef struct opaque_struct* opaque_ptr;

/* Structure that references undefined type */
struct GTY(()) references_undefined {
  opaque_ptr GTY((skip)) unknown;
  int known_field;
};

/* Test case for TYPE_NONE - this should not appear in normal parsing
   as TYPE_NONE is used internally for error cases */

/* Additional scalar typedefs */
typedef long long large_scalar;
typedef unsigned char byte;

/* Array of unions */
union GTY(()) small_union {
  int i;
  char c;
  float f;
};

struct GTY(()) union_array_container {
  int count;
  union small_union GTY((length("count"))) unions[];
};

/* String array type */
typedef const char* string_array_type[10];

/* Nested pointer indirection */
struct GTY(()) deep_pointer {
  struct deep_pointer* GTY((skip)) next;
  void* GTY((skip)) data;
  callback_type GTY((skip)) processor;
};

/* Mixed type containing everything */
struct GTY(()) kitchen_sink {
  /* Basic scalars */
  int integer;
  float floating;
  enum my_enum enumeration;
  
  /* Pointers */
  void* generic_pointer;
  int_ptr int_pointer;
  struct plain_struct* struct_pointer;
  
  /* Arrays */
  fixed_array fixed;
  int GTY((length("dynamic_count"))) dynamic[];
  
  /* Strings */
  const char* static_string;
  string_type typed_string;
  
  /* Callbacks */
  callback_type callback;
  
  /* Unions */
  union my_union u;
  
  /* Nested structs */
  struct inner {
    int value;
  } nested;
  
  /* Reference to user struct */
  struct user_struct* user_ref;
  
  /* Language-specific struct */
  struct lang_specific* lang_ref;
  
  /* Count for dynamic array */
  int dynamic_count;
  
  /* Opaque reference */
  opaque_ptr mystery;
};
