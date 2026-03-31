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

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[256];

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
  int int_val;
  float float_val;
  void *ptr_val;
  char str_val[20];
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
  
  /* TYPE_STRING field */
  const char *name;
  
  /* TYPE_ARRAY field (fixed size) */
  int numbers[5];
  
  /* TYPE_UNION field */
  union my_union data_union;
  
  /* TYPE_CALLBACK field */
  callback_type handler;
  
  /* TYPE_SCALAR fields */
  int counter;
  enum my_enum status;
  
  /* Nested TYPE_STRUCT */
  struct plain_struct nested_plain;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_struct * GTY((skip)) user_data;
  
  /* Pointer to TYPE_LANG_STRUCT */
  struct lang_specific *lang_ptr;
};

/* Another GTY structure with flexible array member */
struct GTY(()) flexible_struct {
  int length;
  char data GTY((length("%0.length"))) [];
};

/* Union containing various pointer types */
union GTY(()) pointer_union {
  int *int_ptr;
  void **void_ptr_ptr;
  struct complex_struct *struct_ptr;
  callback_type func_ptr;
};

/* Structure with array of pointers */
struct GTY(()) array_of_pointers {
  int count;
  void * GTY((length("%0.count"))) items[];
};

/* Structure with callback array */
struct GTY(()) callback_container {
  int num_callbacks;
  callback_type GTY((length("%0.num_callbacks"))) callbacks[];
};

/* Opaque pointer type for TYPE_UNDEFINED testing */
typedef struct opaque_struct *opaque_ptr;

/* Mixed structure with all types */
struct GTY(()) all_types_struct {
  /* SCALAR */
  int id;
  enum my_enum type;
  
  /* STRING */
  const char *description;
  
  /* POINTER */
  void *data;
  
  /* ARRAY */
  char tag[16];
  
  /* UNION */
  union my_union value;
  
  /* CALLBACK */
  callback_type notify;
  
  /* Nested STRUCT */
  struct plain_struct base;
  
  /* Pointer to USER_STRUCT */
  struct user_struct *user_info;
  
  /* Pointer to LANG_STRUCT */
  struct lang_specific *lang_info;
  
  /* Pointer to opaque (UNDEFINED) */
  struct opaque_struct *unknown;
};

/* Root structure that ties everything together */
struct GTY(()) root_container {
  struct complex_struct *first;
  struct flexible_struct *second;
  union pointer_union third;
  struct array_of_pointers *fourth;
  struct callback_container *fifth;
  struct all_types_struct sixth;
  
  /* Array of strings */
  const char * GTY((length("%0.string_count"))) strings[];
  int string_count;
};

/* Additional typedefs to cover more cases */
typedef struct complex_struct complex_t;
typedef union pointer_union pointers_t;
typedef struct all_types_struct all_types_t;

/* Function pointer with complex signature */
typedef void (*complex_callback)(struct complex_struct *, 
                                 union pointer_union, 
                                 const char *);
