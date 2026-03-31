/* test-gengtype-types.h - Test types for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h)"))) my_string_3;

/* TYPE_STRUCT: Complete struct definitions */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  int a;
  double b;
  struct my_struct_2 * GTY((skip)) next;
  struct my_struct_2 *prev;
};

struct GTY((desc("%1.type"))) my_struct_3 {
  enum { TYPE_A, TYPE_B } type;
  union {
    int int_val;
    double dbl_val;
  } value;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_1 {
  void *opaque_data;
  int user_tag;
};

struct GTY((user)) user_struct_2 {
  long id;
  struct user_struct_1 *ref;
};

/* TYPE_UNION: Union definitions */
union GTY((desc("%0.kind"))) my_union_1 {
  int int_member;
  double double_member;
  char *string_member;
  enum { KIND_INT, KIND_DOUBLE, KIND_STRING } kind;
};

union GTY((tag("UNION_2"))) my_union_2 {
  struct my_struct_1 *sptr;
  struct my_struct_2 *sptr2;
  my_scalar_3 scalar;
};

union my_union_3 {
  int x;
  long y;
  void *z;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef struct my_struct_2 * GTY((skip)) my_pointer_2;
typedef union my_union_1 *my_pointer_3;
typedef my_string_1 *string_ptr;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef const char * GTY((length("strlen(%h[i])"))) string_array[20];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (*my_callback_1)(int, char *);
typedef int (*my_callback_2)(struct my_struct_1 *, my_scalar_2);
typedef void (* GTY((skip)) my_callback_3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.tag"), chain_next="%h.next")) lang_struct_type_1 {
  int tag;
  union {
    int ival;
    double dval;
    struct lang_struct_type_1 *child;
  } u;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("TREE_CODE(%h)"))) lang_struct_type_2 {
  enum tree_code code;
  union lang_tree_node * GTY((desc("%1.code"))) chain;
  location_t locus;
};

struct GTY((for_user)) lang_struct_type_3 {
  struct gimple_statement_base *next;
  enum gimple_code code;
};

/* Additional nested structures to ensure traversal */
struct GTY(()) container_struct {
  struct my_struct_1 *first;
  struct my_struct_2 *second;
  union my_union_1 third;
  my_array_1 fourth;
  my_callback_1 callback;
  struct lang_struct_type_1 *lang_data;
};

/* Enum definitions that might be referenced */
enum tree_code { TREE_VOID, TREE_INTEGER, TREE_REAL };
enum gimple_code { GIMPLE_ASSIGN, GIMPLE_CALL, GIMPLE_COND };

/* Forward declarations for language structs */
union lang_tree_node;
struct gimple_statement_base;

#endif /* TEST_GENGTYPE_TYPES_H */
