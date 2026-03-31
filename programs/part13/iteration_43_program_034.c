/* test-coverage.gt - Test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_ENUM (handled as TYPE_SCALAR) */
enum my_enum {
  ENUM_VALUE1,
  ENUM_VALUE2,
  ENUM_VALUE3
};

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
  int field1;
  double field2;
  enum my_enum field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
  void * GTY((skip)) data;
  int user_id;
};

/* TYPE_UNION: Plain union */
union my_union {
  int a;
  float b;
  void *c;
};

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef struct plain_struct* struct_ptr;
typedef union my_union* union_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array_type[10];
typedef const char *string_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void * GTY((skip)) lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  struct plain_struct * GTY((tag("0"))) nested_struct_ptr;
  
  /* TYPE_UNION */
  union my_union GTY((tag("1"))) my_union_field;
  
  /* TYPE_ARRAY */
  int GTY((tag("2"))) int_array[20];
  
  /* TYPE_STRING */
  const char * GTY((tag("3"))) name;
  
  /* TYPE_CALLBACK */
  callback_type GTY((tag("4"))) callback;
  
  /* TYPE_SCALAR */
  enum my_enum GTY((tag("5"))) current_state;
  
  /* Another TYPE_POINTER to TYPE_ARRAY */
  int (* GTY((tag("6"))) matrix_ptr)[10];
  
  /* Flexible array member (TYPE_ARRAY) */
  struct lang_specific * GTY((length("flexible_count"))) flexible_array[];
  
  /* Counter for flexible array */
  size_t flexible_count;
};

/* Another union with GTY marker */
union GTY((desc ("%1.union_type"))) tagged_union {
  int GTY((tag ("0"))) as_int;
  float GTY((tag ("1"))) as_float;
  struct complex_struct * GTY((tag ("2"))) as_complex;
};

/* Struct containing a union */
struct GTY(()) container_struct {
  union tagged_union GTY((tag ("container_union"))) data;
  
  /* TYPE_POINTER to TYPE_CALLBACK */
  int (* GTY((tag ("operation"))) operation_callback)(struct container_struct *);
  
  /* TYPE_ARRAY of TYPE_POINTER */
  void * GTY((tag ("ptr_array"))) *pointer_array[8];
};

/* Chain of pointers for deep traversal */
struct GTY(()) node {
  int value;
  struct node * GTY((tag ("next"))) next;
  struct node * GTY((tag ("prev"))) prev;
};

/* Root structure that references many types */
struct GTY(()) root_type {
  /* TYPE_STRUCT reference */
  struct plain_struct plain;
  
  /* TYPE_USER_STRUCT */
  struct user_struct * GTY((tag ("user_data"))) user_data;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific lang;
  
  /* TYPE_UNION */
  union my_union some_union;
  
  /* TYPE_ARRAY of different types */
  struct complex_struct * GTY((tag ("complex_array"))) complex_objects[5];
  
  /* TYPE_STRING array */
  const char * GTY((tag ("messages"))) messages[3];
  
  /* TYPE_CALLBACK array */
  callback_type GTY((tag ("callbacks"))) handlers[4];
  
  /* Linked list */
  struct node * GTY((tag ("list_head"))) head;
  
  /* TYPE_POINTER to TYPE_ARRAY */
  int (* GTY((tag ("matrix"))) dynamic_matrix)[];
  
  /* Opaque pointer (TYPE_POINTER to TYPE_UNDEFINED) */
  struct opaque_struct * GTY((tag ("opaque"))) opaque_ptr;
};

/* Additional scalar typedefs */
typedef volatile int volatile_scalar;
typedef const long const_scalar;

/* Function pointer with complex signature */
typedef struct root_type *(*factory_type)(
  const char *name,
  int count,
  void * GTY((skip)) context
);

/* Struct using the complex callback */
struct GTY(()) factory_container {
  factory_type GTY((tag ("factory"))) create_object;
  void * GTY((skip)) factory_context;
};

/* Ensure void type is processed */
typedef void void_type;

/* Ensure char type is processed */
typedef signed char byte_type;

/* Ensure bool type is processed (if available) */
#ifdef __cplusplus
typedef bool bool_type;
#else
typedef _Bool bool_type;
#endif

/* Final root variable declaration */
static struct root_type GTY((tag ("global_root"))) global_root;
