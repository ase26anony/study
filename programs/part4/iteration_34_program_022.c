/* test_types.h - Type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(())
{
  char* data GTY((length("str_len")));
  int str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(())
{
  int x;
  double y;
  my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_struct GTY((user))
{
  void* custom_data;
  int tag;
};

/* TYPE_UNION: Union type */
union my_union GTY(())
{
  int as_int;
  double as_double;
  char* as_string GTY((length("10")));
  struct plain_struct* as_struct;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_struct GTY(())
{
  struct plain_struct* next GTY((skip));
  struct string_struct* str_ptr;
  union my_union* union_ptr;
  void* opaque_ptr GTY((skip));
};

/* TYPE_ARRAY: Array types (fixed and variable length) */
struct array_struct GTY(())
{
  int fixed_array[10];
  char* variable_array GTY((length("array_len")));
  int array_len;
  struct plain_struct* struct_array GTY((length("struct_count")));
  int struct_count;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_struct GTY(())
{
  callback_func handler;
  void* user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_struct GTY((lang_struct))
{
  int language_specific_field;
  void* language_data;
};

/* Complex nested type to ensure deep traversal */
struct nested_types GTY(())
{
  /* Contains all type kinds indirectly */
  struct plain_struct base;
  struct string_struct* str_field;
  union my_union variant;
  struct array_struct arrays;
  struct pointer_struct* pointers;
  callback_func cb;
  struct lang_struct lang_data;
  
  /* Self-referential for pointer cycles */
  struct nested_types* next GTY((skip));
  struct nested_types* prev GTY((skip));
};

/* Forward declarations for complex type graph */
struct forward_declared GTY(());
typedef struct forward_declared* forward_ptr GTY(());

struct forward_declared GTY(())
{
  int id;
  forward_ptr next;
};

/* TYPE_UNDEFINED: Might be triggered by incomplete types or special cases */
/* Using a struct with a flexible array member might help */
struct undefined_helper GTY(())
{
  int count;
  /* This might be treated as TYPE_UNDEFINED in some contexts */
  int flexible_array[];
};

/* Enum type (also scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Bitfield struct */
struct bitfield_struct GTY(())
{
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Function pointer in struct */
struct funcptr_struct GTY(())
{
  int (*compare)(const void*, const void*);
  void (*destructor)(void*);
};

/* GCC attribute examples */
struct attributed_struct GTY(())
{
  int aligned_field __attribute__((aligned(16)));
  volatile int volatile_field;
  const char* const_string;
} __attribute__((packed));

#endif /* TEST_TYPES_H */
