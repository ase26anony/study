/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2 };

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
  callback_type func;
};

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
  int field1;
  scalar_int field2;
  fixed_array field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
  void *data;
  int count;
  union my_union u;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  string_type lang_name;
  struct plain_struct *ps;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
  /* TYPE_POINTER within GC struct */
  struct lang_specific *lang_ptr;
  
  /* TYPE_UNION within GC struct */
  union my_union data_union;
  
  /* TYPE_ARRAY within GC struct */
  int numbers[5];
  
  /* TYPE_STRING within GC struct */
  const char *name;
  
  /* TYPE_CALLBACK within GC struct */
  callback_type handler;
  
  /* TYPE_SCALAR within GC struct */
  enum my_enum status;
  
  /* TYPE_UNDEFINED pointer */
  struct opaque_struct *opaque;
  
  /* Flexible array member (TYPE_ARRAY) */
  int flexible_array[];
};

/* Another GC structure with pointer chain */
struct GTY(()) container {
  struct complex_nested *nested;
  struct user_struct *user;
  int_ptr int_pointer;
  string_array strings;
};

/* Union with GTY marker */
union GTY((desc ("%1.type"))) tagged_union {
  int type;
  struct container *c;
  struct lang_specific *ls;
};

/* Array of pointers */
typedef struct GTY(()) node {
  int value;
  struct node *next;
} *node_array[100];

/* Callback structure with function pointers */
struct GTY(()) callback_container {
  callback_type on_start;
  int (*on_data)(void *, size_t);
  void (*on_end)(struct container *);
};

/* Mixed types in a single structure */
struct GTY(()) all_in_one {
  /* SCALAR */
  int id;
  float score;
  enum my_enum state;
  
  /* STRING */
  const char *description;
  
  /* POINTER */
  void *user_data;
  struct all_in_one *next;
  
  /* ARRAY */
  int values[8];
  char name[32];
  
  /* UNION */
  union my_union data;
  
  /* CALLBACK */
  callback_type notify;
  
  /* Nested structures */
  struct plain_struct plain;
  struct user_struct *user;
  
  /* Language-specific */
  struct lang_specific *lang;
};

/* Forward declaration that will remain TYPE_UNDEFINED */
struct never_defined;

/* Typedef for undefined type */
typedef struct never_defined *undefined_ptr;

/* Structure with pointer to undefined type */
struct GTY(()) has_undefined {
  struct never_defined *undef;
  undefined_ptr alias;
};

/* Enumeration type (SCALAR) */
typedef enum {
  RED,
  GREEN,
  BLUE
} color_enum;

/* Structure using the enum */
struct GTY(()) color_container {
  color_enum color;
  const char *color_name;
};

/* Variable length array in structure */
struct GTY(()) var_array {
  int length;
  int data[1];  /* Actually variable length */
};

/* Multiple indirection pointers */
typedef struct container ***triple_ptr;

/* Const pointer types */
typedef int * const const_int_ptr;
typedef const struct container * const const_container_ptr;

/* Anonymous union within struct */
struct GTY(()) with_anon_union {
  int type;
  union {
    int num;
    void *ptr;
    const char *str;
  } value;
};

/* Anonymous struct within union */
union GTY((desc ("%0.tag"))) with_anon_struct {
  int tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    const char *text;
    int length;
  } string;
};

/* Zero-length array */
struct GTY(()) zero_length {
  int count;
  int items[0];
};

/* Array of function pointers */
typedef void (*func_array[5])(void);

/* Structure containing array of function pointers */
struct GTY(()) has_func_array {
  func_array functions;
  int active;
};

/* Complete the opaque struct declaration to avoid errors */
struct opaque_struct {
  int defined_later;
  void *data;
};

/* Make sure TYPE_NONE is not triggered (it's for gcc_unreachable()) */
/* All other TYPE_* cases should be covered by the above definitions */
