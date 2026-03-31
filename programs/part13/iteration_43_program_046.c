/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_ENUM is also scalar */
enum my_enum {
  ENUM_VALUE1,
  ENUM_VALUE2,
  ENUM_VALUE3
};

/* TYPE_STRING: String type */
typedef const char *string_type;
typedef char *mutable_string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, void*);
typedef int (*another_callback)(const char*);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;
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

/* TYPE_USER_STRUCT: GTY-marked user struct */
struct GTY((user)) user_struct {
  void *data;
  int id;
  callback_type callback;
};

/* TYPE_UNION: Plain union */
union my_union {
  int int_val;
  float float_val;
  void *ptr_val;
  char str_val[20];
};

/* GTY-marked union */
union GTY(()) tagged_union {
  int tag;
  struct plain_struct *ps;
  union my_union *mu;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
  enum my_enum lang_enum;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_struct {
  /* TYPE_POINTER field */
  struct plain_struct *plain_ptr;
  
  /* TYPE_ARRAY field */
  int numbers[5];
  
  /* TYPE_STRING field */
  const char *name;
  
  /* TYPE_CALLBACK field */
  callback_type handler;
  
  /* TYPE_UNION field */
  union my_union data;
  
  /* TYPE_SCALAR fields */
  int count;
  enum my_enum status;
  
  /* TYPE_USER_STRUCT pointer */
  struct user_struct *user_data;
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_specific *lang_data;
  
  /* Flexible array member (TYPE_ARRAY) */
  char flexible_array[];
};

/* Another GTY structure with nested arrays */
struct GTY(()) container {
  /* Array of pointers (TYPE_ARRAY of TYPE_POINTER) */
  struct complex_struct *items[10];
  
  /* Pointer to array (TYPE_POINTER to TYPE_ARRAY) */
  int (*matrix)[5];
  
  /* Callback array */
  callback_type callbacks[3];
  
  /* String array */
  const char *strings[4];
};

/* Union containing various types */
union GTY(()) mega_union {
  struct complex_struct cs;
  struct container ct;
  struct user_struct us;
  struct lang_specific ls;
  callback_type cb;
  string_type str;
  int_ptr ip;
};

/* Typedef for a complex callback */
typedef void (*complex_callback)(struct complex_struct*, union mega_union*, int);

/* Structure using the complex callback */
struct GTY(()) callback_container {
  complex_callback func;
  void *user_data;
  struct callback_container *next;  /* Linked list */
};

/* Forward declaration for circular reference */
struct forward_declared;

/* Structure with circular reference */
struct GTY(()) circular_struct {
  int value;
  struct forward_declared *fd;  /* TYPE_UNDEFINED until defined below */
  struct circular_struct *self;  /* Self-reference */
};

/* Now define the forward declared struct */
struct GTY(()) forward_declared {
  char data[50];
  struct circular_struct *back_ref;
};

/* Array of unions */
union my_union union_array[8];

/* Pointer to array */
int (*array_ptr)[10];

/* Multi-dimensional array */
int matrix[3][4];

/* Structure with bitfields (also scalar) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Void pointer typedef */
typedef void *void_ptr;

/* Constant pointer typedef */
typedef int *const const_int_ptr;

/* Structure with all basic types */
struct GTY(()) all_types {
  /* Scalars */
  char c;
  short s;
  int i;
  long l;
  float f;
  double d;
  
  /* Pointers */
  void *vp;
  int *ip;
  char *cp;
  const char *ccp;
  
  /* Arrays */
  int arr1[5];
  char arr2[10];
  
  /* Nested struct */
  struct plain_struct ps;
  
  /* Union */
  union my_union u;
  
  /* Enum */
  enum my_enum e;
  
  /* String */
  const char *str;
  
  /* Callback */
  callback_type cb;
};

/* Template-like structure for testing parameterized types */
#define DECLARE_STRUCT(T) \
struct GTY(()) struct_##T { \
  T data; \
  struct struct_##T *next; \
}

/* Instantiate with different types */
DECLARE_STRUCT(int);
DECLARE_STRUCT(void*);
DECLARE_STRUCT(const char*);
DECLARE_STRUCT(callback_type);

/* Clean up the macro */
#undef DECLARE_STRUCT

/* Final catch-all structure referencing everything */
struct GTY(()) master_struct {
  struct all_types *at;
  struct complex_struct *cs;
  struct container *cn;
  union mega_union *mu;
  struct user_struct *us;
  struct lang_specific *ls;
  struct circular_struct *cr;
  struct forward_declared *fd;
  struct bitfield_struct *bf;
  struct struct_int *si;
  struct struct_void__ *sv;
  struct struct_const_char__ *sc;
  struct struct_callback_type_ *sct;
  struct callback_container *cc;
  
  /* Direct embeddings */
  struct plain_struct ps;
  union my_union u;
  enum my_enum e;
  
  /* Arrays of various types */
  int int_array[7];
  void *ptr_array[3];
  struct plain_struct struct_array[2];
  union my_union union_array[4];
  callback_type callback_array[2];
  
  /* The ultimate flexible array */
  char trailing_data[];
};
