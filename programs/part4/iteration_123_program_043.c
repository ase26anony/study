/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC type nodes to ensure all type_enum
   categories are exercised during gengtype processing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Global variables with GTY annotations to force type processing */
static GTY(()) tree global_scalar = NULL_TREE;
static GTY(()) tree global_pointer = NULL_TREE;
static GTY(()) tree global_array = NULL_TREE;
static GTY(()) tree global_struct = NULL_TREE;
static GTY(()) tree global_union = NULL_TREE;
static GTY(()) tree global_string = NULL_TREE;
static GTY(()) tree global_callback = NULL_TREE;
static GTY(()) tree global_user_struct = NULL_TREE;
static GTY(()) tree global_lang_struct = NULL_TREE;

/* Helper to create a struct with variable field count */
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

/* Helper to create a union with variable member count */
static tree
create_union_with_members(int member_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree member_list = NULL_TREE;
  
  for (int i = 0; i < member_count; i++) {
    tree member = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("member"),
                            integer_type_node);
    DECL_CONTEXT(member) = union_type;
    member_list = chainon(member_list, member);
  }
  
  TYPE_FIELDS(union_type) = member_list;
  layout_type(union_type);
  return union_type;
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* TYPE_SCALAR - basic scalar types */
  global_scalar = integer_type_node;
  gt_ggc_mx(global_scalar);
  
  /* TYPE_POINTER - pointer to scalar */
  global_pointer = build_pointer_type(integer_type_node);
  gt_ggc_mx(global_pointer);
  
  /* TYPE_ARRAY - array of scalars */
  tree index_type = build_index_type(size_int(__FIELD_COUNT__));
  global_array = build_array_type(integer_type_node, index_type);
  gt_ggc_mx(global_array);
  
  /* TYPE_STRUCT - regular struct */
  global_struct = create_struct_with_fields(__FIELD_COUNT__);
  gt_ggc_mx(global_struct);
  
  /* TYPE_UNION - regular union */
  global_union = create_union_with_members(__FIELD_COUNT__);
  gt_ggc_mx(global_union);
  
  /* TYPE_STRING - char pointer (string type) */
  global_string = build_pointer_type(char_type_node);
  gt_ggc_mx(global_string);
  
  /* TYPE_CALLBACK - function pointer */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  global_callback = build_pointer_type(func_type);
  gt_ggc_mx(global_callback);
  
  /* TYPE_USER_STRUCT - struct with user flag */
  tree user_struct = create_struct_with_fields(2);
  TYPE_USER_ALIGN(user_struct) = 1;
  global_user_struct = user_struct;
  gt_ggc_mx(global_user_struct);
  
  /* TYPE_LANG_STRUCT - language-specific struct */
  tree lang_struct = create_struct_with_fields(3);
#ifdef TYPE_LANG_FLAG
  TYPE_LANG_FLAG(lang_struct) = 1;
#endif
  global_lang_struct = lang_struct;
  gt_ggc_mx(global_lang_struct);
  
  /* Process additional scalar types for completeness */
  gt_ggc_mx(boolean_type_node);
  gt_ggc_mx(char_type_node);
  gt_ggc_mx(void_type_node);
  
  /* Create and process more complex types */
  tree nested_struct = create_struct_with_fields(1);
  tree struct_ptr = build_pointer_type(nested_struct);
  gt_ggc_mx(struct_ptr);
  
  tree multi_dim_array = build_array_type(
    build_array_type(integer_type_node, index_type),
    index_type);
  gt_ggc_mx(multi_dim_array);
  
  /* Create a struct with pointer field */
  tree struct_with_ptr = make_node(RECORD_TYPE);
  tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("ptr_field"),
                             global_pointer);
  DECL_CONTEXT(ptr_field) = struct_with_ptr;
  TYPE_FIELDS(struct_with_ptr) = ptr_field;
  layout_type(struct_with_ptr);
  gt_ggc_mx(struct_with_ptr);
  
  /* Force processing of all globals */
  gt_ggc_mx(&global_scalar);
  gt_ggc_mx(&global_pointer);
  gt_ggc_mx(&global_array);
  gt_ggc_mx(&global_struct);
  gt_ggc_mx(&global_union);
  gt_ggc_mx(&global_string);
  gt_ggc_mx(&global_callback);
  gt_ggc_mx(&global_user_struct);
  gt_ggc_mx(&global_lang_struct);
}

/* Entry point for standalone test compilation */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Alternative: Use as plugin initialization */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
