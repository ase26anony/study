/* test-base.gtype - Base type definitions for gengtype coverage */

/* TYPE_SCALAR */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT with various fields */
struct my_struct {
  int scalar_field;
  unsigned long ulong_field;
  const char *string_field;
  struct my_struct *next;  /* % chain pointer */
  struct undefined_struct *undef_ptr;  /* Forward reference */
};

/* TYPE_UNION */
union my_union {
  int int_val;
  unsigned long ulong_val;
  struct my_struct *struct_ptr;
  const char *string_val;
};

/* TYPE_POINTER */
typedef struct my_struct *my_struct_ptr_t;
typedef union my_union *my_union_ptr_t;

/* TYPE_ARRAY - variable length */
struct array_container {
  int length;
  struct my_struct *elements GTY((length("%h.length")));
};

/* TYPE_ARRAY - fixed length */
struct fixed_array {
  int data[10];
  struct my_struct *ptr_array[5];
};

/* TYPE_USER_STRUCT */
struct user_defined {
  int data;
  void *opaque;
} GTY((user));

/* Linked list structure for chain_next/chain_prev */
struct linked_list {
  int value;
  struct linked_list *next GTY((chain_next("%h.next")));
  struct linked_list *prev GTY((chain_prev("%h.prev")));
};

/* Discriminated union */
struct tagged_union {
  enum { TAG_INT, TAG_PTR, TAG_STRING } tag;
  union {
    int int_val;
    struct my_struct *struct_ptr;
    const char *string_val;
  } u GTY((desc("%0.tag")));
};

/* Skip annotation */
struct with_skipped_fields {
  int tracked;
  void *skipped GTY((skip));
  struct my_struct *tracked_ptr;
};

/* Maybe undefined */
struct maybe_undefined {
  struct undefined_struct *ptr GTY((maybe_undef));
  int value;
};
