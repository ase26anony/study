/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar GTY(());
typedef unsigned int my_unsigned_scalar GTY(());
typedef double my_double_scalar GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string GTY((string));
typedef char *mutable_string GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_struct GTY(()) {
  int field1;
  double field2;
  my_string str_field;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr GTY(());

/* Another undefined type with malformed GTY annotation */
struct malformed_undefined GTY((invalid_option));

/* TYPE_USER_STRUCT: User-defined structure (simulated via special marker) */
/* In practice, this might be in a separate module or use special GTY options */
#define USER_STRUCT_MARKER
#ifdef USER_STRUCT_MARKER
struct user_defined_struct GTY((user)) {
  int user_data;
  struct my_struct *nested GTY((skip));
};
#endif

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val;
  my_string str_val;
};

/* TYPE_POINTER: Pointer types with various qualifiers */
typedef struct my_struct *struct_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef const int *const_int_ptr GTY((ptr));

/* Opaque pointer for TYPE_POINTER */
struct opaque_type;
typedef struct opaque_type *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_struct struct_array[] GTY(());

/* Flexible array member in a struct */
struct with_flex_array GTY(()) {
  int count;
  int data[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(int, const char *) GTY((callback));

/* Callback with arguments */
typedef void (*event_handler)(int event_id, void *user_data) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to simulate language-specific type */
struct lang_specific_struct GTY((tag("LANG_CPLUSPLUS"))) {
  int lang_data;
  void *lang_specific_ptr GTY((skip));
};

/* Another language struct with different tag */
struct java_lang_struct GTY((tag("LANG_JAVA"))) {
  long java_ref;
  struct lang_specific_struct *cross_lang_ref;
};

/* Nested types to ensure traversal */
struct container_struct GTY(()) {
  /* Contains multiple type categories */
  my_scalar scalar_member;
  my_string string_member;
  struct my_struct struct_member;
  union my_union union_member;
  struct_ptr pointer_member;
  fixed_array array_member;
  simple_callback callback_member;
  struct lang_specific_struct lang_member;
  
  /* For undefined type */
  undefined_ptr undefined_member;
};

/* Template-like structure for more complex cases */
struct template_struct GTY(()) {
  int template_param;
#define DECLARE_FIELD(TYPE, NAME) TYPE NAME
  DECLARE_FIELD(int, templated_field);
#undef DECLARE_FIELD
};

/* Enumeration (should be treated as scalar) */
typedef enum my_enum GTY(()) {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
} my_enum_type;

/* Function declarations that use these types */
void process_struct(struct my_struct *s GTY((skip)));
void handle_callback(simple_callback cb);
const char *get_string(my_string str);

/* Macro to generate multiple instances */
#define DEFINE_ARRAY_TYPE(NAME, SIZE) \
  typedef int NAME[SIZE] GTY(())

DEFINE_ARRAY_TYPE(array_5, 5);
DEFINE_ARRAY_TYPE(array_10, 10);

/* Conditional types */
#ifdef SPECIAL_FEATURE
struct conditional_struct GTY(()) {
  int special_feature_data;
};
#endif

/* Inline structure definition */
struct inline_example GTY(()) {
  struct {
    int nested_a;
    int nested_b;
  } nested_struct;
  union {
    int x;
    float y;
  } nested_union;
};

#endif /* TEST_GTYPE_H */
