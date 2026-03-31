/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;

/* TYPE_ENUM (falls under TYPE_SCALAR) */
enum my_enum {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
};

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, void*);
typedef int (*another_callback)(const char*);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char small_array[5];

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
  const char *field3;
  fixed_array field4;  /* TYPE_ARRAY nested in struct */
};

/* TYPE_USER_STRUCT: GTY-marked user struct */
struct GTY((user)) user_struct {
  void *data;
  int data_size;
  callback_type cleanup_func;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"), desc("language_specific"))) lang_specific {
  int lang_field1;
  void *lang_field2;
  union my_union lang_field3;  /* TYPE_UNION nested in lang struct */
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
  /* TYPE_POINTER field */
  struct plain_struct *plain_ptr;
  
  /* TYPE_UNION field */
  union my_union union_field;
  
  /* TYPE_ARRAY field (fixed size) */
  int int_array[20];
  
  /* TYPE_ARRAY field (of pointers) */
  void *ptr_array[15];
  
  /* TYPE_STRING field */
  const char *name;
  
  /* TYPE_CALLBACK field */
  callback_type handler;
  
  /* Nested TYPE_STRUCT */
  struct inner_struct {
    int inner_field1;
    void *inner_field2;
  } inner;
  
  /* TYPE_ARRAY of structs */
  struct inner_struct struct_array[5];
  
  /* Flexible array member (variable length array) */
  int flexible_array[];
};

/* Another GTY structure with various type combinations */
struct GTY(()) another_gty_struct {
  /* TYPE_SCALAR fields */
  scalar_int int_field;
  enum my_enum enum_field;
  
  /* TYPE_POINTER to union */
  union my_union *union_ptr;
  
  /* TYPE_POINTER to callback */
  callback_type *callback_ptr;
  
  /* TYPE_ARRAY of strings */
  const char *string_array[8];
  
  /* TYPE_POINTER to opaque type */
  struct opaque_struct *opaque;
  
  /* TYPE_USER_STRUCT field */
  struct user_struct user_data;
};

/* Union with GTY marker */
union GTY(()) gty_union {
  struct complex_nested *nested_ptr;
  another_callback callback;
  string_type str;
};

/* Structure containing array of function pointers */
struct GTY(()) callback_container {
  /* TYPE_ARRAY of TYPE_CALLBACK */
  callback_type callbacks[4];
  
  /* TYPE_POINTER to array of callbacks */
  another_callback (*dynamic_callbacks)[];
};

/* For TYPE_ARRAY with unknown bound (incomplete array type) */
struct GTY(()) incomplete_array_struct {
  int count;
  /* Incomplete array - will be processed as TYPE_ARRAY */
  int data[];
};

/* Structure with pointer to itself (recursive type) */
struct GTY(()) recursive_struct {
  int value;
  struct recursive_struct *next;  /* TYPE_POINTER to same type */
  struct recursive_struct *prev;  /* Another self-reference */
};

/* Mixed structure with all types */
struct GTY(()) all_types_struct {
  /* TYPE_SCALAR */
  int scalar_int_field;
  float scalar_float_field;
  enum my_enum scalar_enum_field;
  
  /* TYPE_STRING */
  const char *string_field;
  
  /* TYPE_POINTER */
  void *pointer_field;
  int *int_pointer_field;
  struct plain_struct *struct_pointer_field;
  
  /* TYPE_CALLBACK */
  callback_type callback_field;
  
  /* TYPE_ARRAY */
  int int_array_field[10];
  callback_type callback_array[3];
  
  /* TYPE_UNION */
  union my_union union_field;
  
  /* TYPE_STRUCT (nested, no GTY) */
  struct {
    int nested_int;
    void *nested_ptr;
  } nested_struct_field;
  
  /* TYPE_USER_STRUCT */
  struct user_struct user_struct_field;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific lang_struct_field;
};

/* Typedefs for various types */
typedef struct complex_nested complex_nested_t;
typedef union gty_union gty_union_t;
typedef callback_type handler_func_t;

/* Global variables marked for GC */
GTY(()) struct complex_nested *global_nested = NULL;
GTY(()) string_type global_string = "test";
GTY(()) callback_type global_callback = NULL;

/* Another structure with bitfields (scalar type) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int value : 16;
};

/* Structure with array of structures containing arrays */
struct GTY(()) deeply_nested {
  struct array_element {
    int id;
    char name[32];
    void *data[4];
  } elements[8];
  
  struct array_element *extra_elements;
  int extra_count;
};

/* Test case for variable length array in the middle */
struct GTY(()) var_len_middle {
  int count;
  int data[];
  /* Note: This is invalid in standard C but might be processed */
};

/* Forward declaration that will be defined later */
struct forward_declared;

/* Structure using forward declared type */
struct GTY(()) uses_forward {
  struct forward_declared *fd_ptr;  /* TYPE_POINTER to undefined type */
  int valid;
};

/* Later definition of forward declared type */
struct GTY(()) forward_declared {
  int actual_field;
  struct uses_forward *back_ref;  /* Circular reference */
};

/* Empty structures */
struct GTY(()) empty_struct {
  /* No fields */
};

union GTY(()) empty_union {
  /* No fields */
};

/* Structure with only scalar fields */
struct GTY(()) only_scalars {
  int a;
  float b;
  double c;
  enum my_enum d;
  unsigned char e;
  short f;
  long g;
  long long h;
};

/* Structure with only pointer fields */
struct GTY(()) only_pointers {
  void *ptr1;
  int *ptr2;
  const char *ptr3;
  struct only_scalars *ptr4;
  callback_type ptr5;
};

/* Structure with const fields */
struct GTY(()) const_fields {
  const int read_only_int;
  const void *read_only_ptr;
  const char *const read_only_string;
};

/* Anonymous union within struct */
struct GTY(()) with_anonymous_union {
  int type;
  union {
    int int_value;
    float float_value;
    void *ptr_value;
  } data;
};

/* Anonymous struct within union */
union GTY(()) with_anonymous_struct {
  struct {
    int x;
    int y;
  } point;
  int coordinates[2];
};

/* Final catch-all structure that includes everything */
struct GTY(()) ultimate_test {
  /* Include pointers to all previously defined types */
  struct opaque_struct *opaque_ptr;
  struct plain_struct *plain_ptr;
  struct user_struct *user_ptr;
  struct lang_specific *lang_ptr;
  struct complex_nested *nested_ptr;
  struct another_gty_struct *another_ptr;
  union gty_union *union_ptr;
  struct callback_container *callback_ptr;
  struct incomplete_array_struct *incomplete_ptr;
  struct recursive_struct *recursive_ptr;
  struct all_types_struct *all_types_ptr;
  struct bitfield_struct *bitfield_ptr;
  struct deeply_nested *deeply_nested_ptr;
  struct uses_forward *forward_use_ptr;
  struct forward_declared *forward_def_ptr;
  struct empty_struct *empty_struct_ptr;
  union empty_union *empty_union_ptr;
  struct only_scalars *scalars_ptr;
  struct only_pointers *pointers_ptr;
  struct const_fields *const_ptr;
  struct with_anonymous_union *anon_union_ptr;
  union with_anonymous_struct *anon_struct_ptr;
  
  /* Direct inclusions */
  struct all_types_struct direct;
  union my_union direct_union;
  callback_type direct_callback;
  string_type direct_string;
  fixed_array direct_array;
  enum my_enum direct_enum;
  
  /* Array of various types */
  void *ptr_array[16];
  callback_type callback_array[8];
  const char *string_array[12];
  struct plain_struct struct_array[4];
  union my_union union_array[6];
};
