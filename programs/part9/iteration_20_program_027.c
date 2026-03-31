/* base-types.gtype - Basic type definitions for gengtype testing */

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_POINTER */
typedef struct my_struct *my_struct_ptr;
typedef union my_union *my_union_ptr;

/* Forward declarations to create TYPE_UNDEFINED cases */
struct forward_declared;
typedef struct forward_declared *forward_ptr;

/* TYPE_STRUCT with various fields */
struct my_struct GTY(()) {
  int scalar_field;
  unsigned long ulong_field;
  color_enum enum_field;
  const char * GTY((skip)) skip_field;  /* TYPE_STRING with skip */
  struct my_struct * GTY((tag("0"))) next;  /* TYPE_POINTER with tag */
  union my_union * GTY((maybe_undef)) maybe_union;  /* May be undefined */
  forward_ptr forward_field;  /* TYPE_UNDEFINED reference */
};

/* TYPE_UNION with discriminator */
union my_union GTY((desc("tag_field"))) {
  int tag_field;
  struct {
    int x;
    int y;
  } GTY((tag("1"))) point;
  struct {
    const char *name;
    int value;
  } GTY((tag("2"))) named_value;
};

/* TYPE_ARRAY examples */
struct array_container GTY(()) {
  int count;
  struct my_struct * GTY((length("%h.count"))) variable_array[1];
  int fixed_array[10];
  union my_union * GTY((length("5"))) fixed_ptr_array[5];
};

/* Linked list example using chain_next */
struct linked_list GTY(()) {
  int data;
  struct linked_list * GTY((chain_next("%h.next"))) next;
  struct linked_list * GTY((chain_prev("%h.prev"))) prev;
};

/* TYPE_CALLBACK */
typedef void (*callback_func) GTY((callback)) (struct my_struct *arg, int value);

struct callback_container GTY(()) {
  callback_func handler;
  void * GTY((skip)) user_data;
};
