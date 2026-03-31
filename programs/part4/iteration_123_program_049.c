/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "ggc.h"

/* Helper macro to ensure types are processed by gengtype */
#define PROCESS_TYPE_WITH_GTY(type) \
  do { \
    static GTY(()) tree __processed_##type = NULL_TREE; \
    if (__processed_##type == NULL_TREE) \
      __processed_##type = (type); \
  } while (0)

/* Create a struct with variable field count */
static tree
create_struct_with_fields(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
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

/* Create a union with variable field count */
static tree
create_union_with_fields(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("field"),
                           integer_type_node);
    DECL_CONTEXT(field) = union_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  
  return union_type;
}

/* Create an array with variable dimensions */
static tree
create_array_type_with_dims(tree element_type, int ndims)
{
  tree array_type = element_type;
  
  for (int i = 0; i < ndims; i++) {
    /* Create array type with 10 elements in each dimension */
    tree index_type = build_index_type(build_int_cst(integer_type_node, 9));
    array_type = build_array_type(array_type, index_type);
  }
  
  return array_type;
}

/* Create a callback (function pointer) type */
static tree
create_callback_type(void)
{
  /* Create a function type returning int with no arguments */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  /* Create pointer to function type */
  return build_pointer_type(func_type);
}

/* Mark a type as language-specific */
static void
mark_as_lang_struct(tree type)
{
#ifdef TYPE_LANG_STRUCT
  TYPE_LANG_STRUCT(type) = 1;
#else
  /* Alternative method for marking language-specific types */
  SET_TYPE_LANG_SPECIFIC(type);
#endif
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* TYPE_SCALAR - Basic scalar types */
  PROCESS_TYPE_WITH_GTY(integer_type_node);
  PROCESS_TYPE_WITH_GTY(char_type_node);
  PROCESS_TYPE_WITH_GTY(boolean_type_node);
  PROCESS_TYPE_WITH_GTY(void_type_node);
  
  /* TYPE_POINTER - Pointer types */
  tree int_ptr = build_pointer_type(integer_type_node);
  tree char_ptr = build_pointer_type(char_type_node);
  tree void_ptr = build_pointer_type(void_type_node);
  
  PROCESS_TYPE_WITH_GTY(int_ptr);
  PROCESS_TYPE_WITH_GTY(char_ptr);
  PROCESS_TYPE_WITH_GTY(void_ptr);
  
  /* TYPE_STRING - String type (char pointer) */
  /* In GCC, string types are typically represented as char* */
  tree string_type = build_pointer_type(char_type_node);
  PROCESS_TYPE_WITH_GTY(string_type);
  
  /* TYPE_ARRAY - Array types with variable dimensions */
  tree int_array_1d = create_array_type_with_dims(integer_type_node, 1);
  tree int_array_2d = create_array_type_with_dims(integer_type_node, 2);
  tree char_array_3d = create_array_type_with_dims(char_type_node, 3);
  
  PROCESS_TYPE_WITH_GTY(int_array_1d);
  PROCESS_TYPE_WITH_GTY(int_array_2d);
  PROCESS_TYPE_WITH_GTY(char_array_3d);
  
  /* TYPE_STRUCT - Struct types with variable field counts */
  tree struct_1_field = create_struct_with_fields(1);
  tree struct_5_fields = create_struct_with_fields(5);
  tree struct_10_fields = create_struct_with_fields(10);
  
  PROCESS_TYPE_WITH_GTY(struct_1_field);
  PROCESS_TYPE_WITH_GTY(struct_5_fields);
  PROCESS_TYPE_WITH_GTY(struct_10_fields);
  
  /* TYPE_UNION - Union types with variable field counts */
  tree union_1_field = create_union_with_fields(1);
  tree union_3_fields = create_union_with_fields(3);
  tree union_7_fields = create_union_with_fields(7);
  
  PROCESS_TYPE_WITH_GTY(union_1_field);
  PROCESS_TYPE_WITH_GTY(union_3_fields);
  PROCESS_TYPE_WITH_GTY(union_7_fields);
  
  /* TYPE_CALLBACK - Function pointer types */
  tree callback_type = create_callback_type();
  tree callback_type2 = build_pointer_type(
    build_function_type(void_type_node, 
                       tree_cons(NULL_TREE, integer_type_node, NULL_TREE)));
  
  PROCESS_TYPE_WITH_GTY(callback_type);
  PROCESS_TYPE_WITH_GTY(callback_type2);
  
  /* TYPE_USER_STRUCT - User-defined struct types */
  /* These are typically structs with GTY markers */
  tree user_struct = create_struct_with_fields(2);
#ifdef TYPE_USER_STRUCT
  TYPE_USER_STRUCT(user_struct) = 1;
#endif
  PROCESS_TYPE_WITH_GTY(user_struct);
  
  /* TYPE_LANG_STRUCT - Language-specific struct types */
  tree lang_struct = create_struct_with_fields(3);
  mark_as_lang_struct(lang_struct);
  PROCESS_TYPE_WITH_GTY(lang_struct);
  
  /* Complex nested types to ensure thorough processing */
  tree nested_struct = create_struct_with_fields(2);
  TYPE_FIELDS(nested_struct) = build_tree_list(
    build_decl(UNKNOWN_LOCATION, FIELD_DECL,
              get_identifier("nested_field"),
              struct_5_fields),
    build_decl(UNKNOWN_LOCATION, FIELD_DECL,
              get_identifier("array_field"),
              int_array_2d));
  layout_type(nested_struct);
  
  tree ptr_to_nested = build_pointer_type(nested_struct);
  tree array_of_ptrs = create_array_type_with_dims(ptr_to_nested, 2);
  
  PROCESS_TYPE_WITH_GTY(nested_struct);
  PROCESS_TYPE_WITH_GTY(ptr_to_nested);
  PROCESS_TYPE_WITH_GTY(array_of_ptrs);
  
  /* Force GC to potentially process these types */
  ggc_collect();
}

/* Global variables with GTY annotations to ensure processing */
static GTY(()) tree global_scalar = integer_type_node;
static GTY(()) tree global_pointer = NULL_TREE;
static GTY(()) tree global_array = NULL_TREE;
static GTY(()) tree global_struct = NULL_TREE;
static GTY(()) tree global_union = NULL_TREE;
static GTY(()) tree global_callback = NULL_TREE;

/* Test driver */
int
main(void)
{
  /* Initialize global variables with various types */
  global_pointer = build_pointer_type(char_type_node);
  global_array = create_array_type_with_dims(integer_type_node, 2);
  global_struct = create_struct_with_fields(3);
  global_union = create_union_with_fields(2);
  global_callback = create_callback_type();
  
  /* Run the comprehensive test */
  test_gengtype_categorization();
  
  return 0;
}
