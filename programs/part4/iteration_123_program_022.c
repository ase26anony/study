/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "ggc.h"

/* Define placeholders for variability */
#ifndef __FIELD_COUNT__
#define __FIELD_COUNT__ 3
#endif

#ifndef __ARRAY_SIZE__
#define __ARRAY_SIZE__ 10
#endif

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree scalar_types[5];
static GTY(()) tree pointer_types[3];
static GTY(()) tree array_types[3];
static GTY(()) tree struct_types[2];
static GTY(()) tree union_types[2];
static GTY(()) tree callback_types[2];
static GTY(()) tree string_type;
static GTY(()) tree user_struct_type;
static GTY(()) tree lang_struct_type;

/* Helper function to create a struct with variable field count */
static tree
create_test_struct(int field_count, const char *name)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  /* Set the name for debugging */
  if (name)
    TYPE_NAME(struct_type) = get_identifier(name);
  
  /* Create fields based on field_count */
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("field"),
                           integer_type_node);
    DECL_CONTEXT(field) = struct_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(struct_type) = field_list;
  layout_type(struct_type);
  
  return struct_type;
}

/* Helper function to create a union with variable field count */
static tree
create_test_union(int field_count, const char *name)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  if (name)
    TYPE_NAME(union_type) = get_identifier(name);
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("field"),
                           (i % 2 == 0) ? integer_type_node : char_type_node);
    DECL_CONTEXT(field) = union_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  
  return union_type;
}

/* Create a callback (function pointer) type */
static tree
create_callback_type(void)
{
  /* Create a function type returning int with no arguments */
  tree func_type = build_function_type_list(integer_type_node, NULL_TREE);
  /* Create pointer to function type */
  return build_pointer_type(func_type);
}

/* Create a user struct type (marked with TYPE_USER_STRUCT) */
static tree
create_user_struct(void)
{
  tree struct_type = create_test_struct(2, "user_struct");
  
  /* Mark as user struct - this is implementation dependent */
  /* In GCC, user structs might be marked differently */
#ifdef TYPE_LANG_FLAG_0
  TYPE_LANG_FLAG_0(struct_type) = 1;
#endif
  
  return struct_type;
}

/* Create a language-specific struct type */
static tree
create_lang_struct(void)
{
  tree struct_type = create_test_struct(2, "lang_struct");
  
  /* Mark as language-specific struct */
  /* This typically requires setting TYPE_LANG_SPECIFIC */
#ifdef TYPE_LANG_SPECIFIC
  /* Allocate and set language-specific info if available */
  /* This is simplified - actual implementation varies by language */
#endif
  
  return struct_type;
}

/* Main test function that creates all type categories */
void
test_gengtype_categorization(void)
{
  int i;
  
  /* 1. SCALAR TYPES - TYPE_SCALAR */
  scalar_types[0] = integer_type_node;
  scalar_types[1] = char_type_node;
  scalar_types[2] = boolean_type_node;
  scalar_types[3] = size_type_node;
  scalar_types[4] = ptrdiff_type_node;
  
  /* Force processing of scalar types */
  for (i = 0; i < 5; i++) {
    if (scalar_types[i])
      gt_ggc_mx(scalar_types[i]);
  }
  
  /* 2. STRING TYPE - TYPE_STRING */
  /* String type is pointer to char */
  string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(string_type);
  
  /* 3. POINTER TYPES - TYPE_POINTER */
  pointer_types[0] = build_pointer_type(integer_type_node);
  pointer_types[1] = build_pointer_type(char_type_node);
  pointer_types[2] = build_pointer_type(void_type_node);
  
  for (i = 0; i < 3; i++) {
    gt_ggc_mx(pointer_types[i]);
  }
  
  /* 4. ARRAY TYPES - TYPE_ARRAY */
  /* Create array of integers */
  tree index_type = build_index_type(build_int_cst(integer_type_node, __ARRAY_SIZE__ - 1));
  array_types[0] = build_array_type(integer_type_node, index_type);
  
  /* Create array of pointers */
  array_types[1] = build_array_type(pointer_types[0], index_type);
  
  /* Create multi-dimensional array */
  tree range_type = build_range_type(integer_type_node,
                                     build_int_cst(integer_type_node, 0),
                                     build_int_cst(integer_type_node, 4));
  array_types[2] = build_array_type(char_type_node, range_type);
  
  for (i = 0; i < 3; i++) {
    if (array_types[i])
      gt_ggc_mx(array_types[i]);
  }
  
  /* 5. STRUCT TYPES - TYPE_STRUCT */
  struct_types[0] = create_test_struct(__FIELD_COUNT__, "test_struct_1");
  struct_types[1] = create_test_struct(__FIELD_COUNT__ + 1, "test_struct_2");
  
  for (i = 0; i < 2; i++) {
    if (struct_types[i])
      gt_ggc_mx(struct_types[i]);
  }
  
  /* 6. UNION TYPES - TYPE_UNION */
  union_types[0] = create_test_union(__FIELD_COUNT__, "test_union_1");
  union_types[1] = create_test_union(__FIELD_COUNT__ + 1, "test_union_2");
  
  for (i = 0; i < 2; i++) {
    if (union_types[i])
      gt_ggc_mx(union_types[i]);
  }
  
  /* 7. CALLBACK TYPES - TYPE_CALLBACK */
  callback_types[0] = create_callback_type();
  
  /* Another callback type with arguments */
  tree func_with_args = build_function_type_list(integer_type_node,
                                                 integer_type_node,
                                                 char_type_node,
                                                 NULL_TREE);
  callback_types[1] = build_pointer_type(func_with_args);
  
  for (i = 0; i < 2; i++) {
    if (callback_types[i])
      gt_ggc_mx(callback_types[i]);
  }
  
  /* 8. USER STRUCT TYPE - TYPE_USER_STRUCT */
  user_struct_type = create_user_struct();
  if (user_struct_type)
    gt_ggc_mx(user_struct_type);
  
  /* 9. LANG STRUCT TYPE - TYPE_LANG_STRUCT */
  lang_struct_type = create_lang_struct();
  if (lang_struct_type)
    gt_ggc_mx(lang_struct_type);
  
  /* Create a complex type that combines multiple categories */
  /* This ensures edge cases are covered */
  tree complex_struct = create_test_struct(2, "complex_struct");
  tree complex_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("array_field"),
                                 array_types[0]);
  DECL_CONTEXT(complex_field) = complex_struct;
  TYPE_FIELDS(complex_struct) = complex_field;
  layout_type(complex_struct);
  gt_ggc_mx(complex_struct);
  
  /* Create a struct containing pointers to ensure pointer field processing */
  tree ptr_struct = make_node(RECORD_TYPE);
  tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("ptr_field"),
                             pointer_types[0]);
  DECL_CONTEXT(ptr_field) = ptr_struct;
  TYPE_FIELDS(ptr_struct) = ptr_field;
  layout_type(ptr_struct);
  gt_ggc_mx(ptr_struct);
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC environment if needed */
  test_gengtype_categorization();
  return 0;
}
#endif

/* Plugin entry point if compiled as plugin */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
