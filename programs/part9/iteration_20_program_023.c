/* test-base.gtype - Base type definitions for gengtype coverage */

/* TYPE_SCALAR */
typedef int my_scalar_t GTY((tag("SCALAR")));

/* TYPE_STRING */
typedef const char *my_string_t GTY((tag("STRING")));

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY((tag("STRUCT"))) {
  int scalar_field;
  const char *string_field GTY((skip));  /* skip from GC */
  struct my_base_struct *next GTY((chain_next));  /* linked list */
  unsigned long array_length;  /* for variable length array */
};

/* TYPE_UNION with discriminator */
union my_discriminated_union GTY((tag("UNION"))) {
  int int_val;
  const char *str_val;
  struct my_base_struct *struct_ptr;
} GTY((desc("union_tag")));

/* TYPE_ARRAY - variable length */
struct my_varray_struct GTY((tag("VARRAY"))) {
  int count;
  union my_discriminated_union elements[1] 
    GTY((length("count")));
};

/* TYPE_ARRAY - fixed length */
struct my_fixed_array_struct GTY(()) {
  int data[10];
};

/* TYPE_POINTER */
typedef struct my_base_struct *my_struct_ptr_t GTY((tag("PTR")));
typedef union my_discriminated_union *my_union_ptr_t;

/* Forward declaration for TYPE_UNDEFINED test */
struct forward_declared_struct GTY((maybe_undef));

/* Structure using forward declared type */
struct uses_forward_decl GTY(()) {
  struct forward_declared_struct *fwd_ptr;  /* Will be TYPE_UNDEFINED initially */
};

/* Now define the forward declared structure */
struct forward_declared_struct GTY(()) {
  int defined_now;
};
