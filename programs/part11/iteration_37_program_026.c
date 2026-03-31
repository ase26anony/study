/* test-gtype.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type GTY(());
typedef double my_float_type GTY(());
typedef unsigned long my_ulong_type GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef char *mutable_string_type GTY((string));
typedef const char *const constant_string_ptr GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_plain_struct GTY(())
{
  int field1;
  double field2;
  my_scalar_type field3;
};

/* Another struct for variety */
struct another_struct GTY(())
{
  struct my_plain_struct *next GTY((skip));
  int data;
};

/* TYPE_USER_STRUCT: User-defined structure type */
/* Defined with special marker to distinguish from regular structs */
#define USER_GTY_MARKER
struct user_defined_struct GTY((user))
{
  int user_data;
  void *user_ptr GTY((skip));
};

/* TYPE_UNION: Union types */
union my_union_type GTY(())
{
  int int_value;
  double float_value;
  void *pointer_value GTY((skip));
  const char *string_value GTY((string));
};

/* TYPE_POINTER: Pointer types with various annotations */
typedef struct opaque_struct *opaque_pointer GTY((ptr));
typedef void *generic_pointer GTY((skip));
typedef const struct my_plain_struct *const_struct_ptr GTY((skip));

/* Forward declaration for pointer to incomplete type */
struct incomplete_struct;
typedef struct incomplete_struct *incomplete_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_plain_struct struct_array[] GTY(());

/* Special array with length callback */
extern int get_array_length(void);
typedef double dynamic_array[] GTY((length("get_array_length()")));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *input GTY((string)), 
                               void *context GTY((skip))) GTY((callback));
typedef void (*complex_callback)(struct my_plain_struct *data GTY(()),
                                 union my_union_type *union_data GTY(())) 
                                 GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_specific_struct GTY((tag("LANG_SPECIFIC")))
{
  int lang_data;
  void *lang_private GTY((skip));
  struct lang_specific_struct *next GTY((skip));
};

/* Nested language struct for more coverage */
struct outer_lang_struct GTY((tag("OUTER_LANG")))
{
  struct lang_specific_struct inner GTY(());
  int counter;
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
/* These should be categorized as undefined */
struct undefined_struct;
typedef struct undefined_struct undefined_type;

/* Malformed or ambiguous GTY annotation */
struct ambiguous_struct GTY((unknown_tag))
{
  int x;
};

/* Pointer chain that leads to undefined */
typedef struct completely_unknown *unknown_ptr_chain GTY((ptr));

/* Self-referential incomplete type */
struct self_ref_incomplete GTY(())
{
  struct self_ref_incomplete *next GTY((skip));
  /* Missing closing brace intentionally? This would cause issues */
};

/* Mixed type containing various categories */
struct container_struct GTY(())
{
  /* Scalar */
  int scalar_field GTY(());
  
  /* String */
  const char *name GTY((string));
  
  /* Pointer */
  struct container_struct *next GTY((skip));
  
  /* Array */
  int scores[] GTY((length("10")));
  
  /* Union */
  union my_union_type data GTY(());
  
  /* Callback */
  simple_callback handler GTY((callback));
  
  /* Nested struct */
  struct nested_struct GTY(())
  {
    int nested_data;
  } nested;
};

/* Template-like macro for generating multiple instances */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct gty_struct_##name GTY(()) { \
    field_type data; \
    struct gty_struct_##name *next GTY((skip)); \
  }

/* Instantiate some template structs */
DEFINE_GTY_STRUCT(int, int);
DEFINE_GTY_STRUCT(double, double);
DEFINE_GTY_STRUCT(string, const char * GTY((string)));

/* Extern declarations to test linkage */
extern struct my_plain_struct global_struct GTY(());
extern const char *global_string GTY((string));
extern simple_callback global_callback GTY((callback));

#endif /* TEST_GTYPE_H */
