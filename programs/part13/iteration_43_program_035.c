/* test-coverage.gt - Comprehensive test file for gengtype coverage */

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
typedef void* generic_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[256];

/* TYPE_STRUCT: Plain C struct without GC tracking */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_USER_STRUCT: GC-aware struct with user tag */
struct GTY((user)) user_struct {
  void *data;
  int size;
};

/* TYPE_UNION: Union type */
union my_union {
  int int_val;
  float float_val;
  void *ptr_val;
  char str_val[20];
};

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
  
  /* TYPE_ARRAY - fixed size */
  int numbers[5];
  
  /* TYPE_ARRAY - flexible array member */
  char flexible_array GTY((length("strlen(data) + 1")));
  
  /* TYPE_CALLBACK */
  callback_type callback;
  
  /* TYPE_SCALAR */
  int count;
  enum my_enum status;
  
  /* TYPE_POINTER to another GC type */
  struct user_struct * GTY((skip)) user_data;
  
  /* TYPE_UNION */
  union my_union value;
};

/* Another GC struct with nested structures */
struct GTY(()) container_struct {
  /* TYPE_STRUCT (embedded) */
  struct plain_struct plain;
  
  /* TYPE_USER_STRUCT */
  struct user_struct user;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific lang;
  
  /* TYPE_ARRAY of pointers */
  void * GTY((skip)) ptr_array[8];
  
  /* TYPE_ARRAY of structs */
  struct plain_struct struct_array[3];
  
  /* TYPE_CALLBACK array */
  callback_type callbacks[4];
};

/* Union containing GC-tracked pointers */
union GTY((desc("%1.type"), tag("TYPE"))) tagged_union {
  int type;
  struct complex_struct * GTY((tag("0"))) complex;
  struct container_struct * GTY((tag("1"))) container;
  string_type GTY((tag("2"))) str;
};

/* A struct with conditional fields */
struct GTY(()) conditional_struct {
  int GTY((skip)) flag;
  
  /* Conditional pointer based on flag */
  void * GTY((condition("flag != 0"))) conditional_ptr;
  
  /* Array with variable length */
  int * GTY((length("flag"))) variable_array;
};

/* TYPE_NONE should not be directly triggerable as it's for error cases */

/* Additional pointer types with different attributes */
typedef void * GTY((skip)) skipped_ptr;
typedef void * GTY((atomic)) atomic_ptr;

/* String array type */
typedef const char *string_array_type[];

/* Function pointer with complex signature */
typedef void (*complex_callback)(struct complex_struct *, 
                                 struct container_struct *,
                                 callback_type);

/* Final test: struct containing all major types */
struct GTY(()) master_struct {
  /* All basic types */
  scalar_int int_field;
  scalar_float float_field;
  string_type string_field;
  
  /* Composite types */
  struct plain_struct plain_field;
  struct user_struct user_field;
  union my_union union_field;
  struct lang_specific lang_field;
  
  /* Pointers */
  int_ptr int_pointer;
  generic_ptr generic_pointer;
  opaque_ptr opaque_pointer;
  
  /* Arrays */
  fixed_array fixed;
  string_array string;
  
  /* Callback */
  callback_type callback_field;
  complex_callback complex_callback_field;
  
  /* Nested GC structures */
  struct complex_struct *complex_ptr;
  struct container_struct container_field;
  
  /* Tagged union */
  union tagged_union tagged;
  
  /* Conditional structure */
  struct conditional_struct conditional;
};

/* Global variable declarations for completeness */
extern struct master_struct GTY((root)) global_master;
extern struct complex_struct * GTY((root)) global_complex_array[];
extern string_type GTY((root)) global_strings[];
