/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;

/* TYPE_SCALAR: Plain integer */
struct GTY(()) scalar_struct {
  int scalar_field;  /* TYPE_SCALAR */
};

/* TYPE_STRING: String pointer */
struct GTY(()) string_struct {
  const char * GTY((skip)) string_field;  /* TYPE_STRING */
};

/* TYPE_STRUCT: Regular struct */
struct GTY(()) regular_struct {
  int id;
  char name[32];
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) user_struct {
  void *data;
  int (*custom_marker)(void *);
};

/* TYPE_UNION: Union type */
union GTY(()) test_union {
  int as_int;
  float as_float;
  void *as_ptr;
};

/* TYPE_POINTER: Pointer to another GTY type */
struct GTY(()) pointer_struct {
  struct regular_struct * GTY((tag("0"))) ptr_field;  /* TYPE_POINTER */
  struct opaque *opaque_ptr;  /* TYPE_UNDEFINED via pointer */
};

/* TYPE_ARRAY: Array of GTY types */
struct GTY(()) array_struct {
  struct regular_struct array_field[10];  /* TYPE_ARRAY */
  int int_array[5];  /* TYPE_SCALAR array */
};

/* TYPE_CALLBACK: Function pointer typedef */
typedef void (* GTY((callback)) callback_func)(void *data, int value);

struct GTY(()) callback_struct {
  callback_func handler;  /* TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct {
  /* This would be processed differently for language-specific generators */
  void *lang_specific;
};
#endif

/* Nested structures to ensure all counters are hit */
struct GTY(()) container {
  struct scalar_struct scalar;      /* TYPE_STRUCT */
  struct string_struct string;      /* TYPE_STRUCT */
  union test_union union_field;     /* TYPE_UNION */
  struct pointer_struct pointers;   /* TYPE_STRUCT */
  struct array_struct arrays;       /* TYPE_STRUCT */
  struct callback_struct callback;  /* TYPE_STRUCT */
  
  /* Direct scalar, string, pointer, array */
  int direct_scalar;                /* TYPE_SCALAR */
  const char *direct_string;        /* TYPE_STRING */
  void *direct_pointer;             /* TYPE_POINTER */
  int direct_array[3];              /* TYPE_SCALAR array */
};

/* Additional union with GTY annotation */
union GTY(()) another_union {
  struct container *c_ptr;
  int value;
};

#endif /* GTY_TEST_H */
