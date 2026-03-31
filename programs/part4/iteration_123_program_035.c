/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC type nodes to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "stringpool.h"
#include "attribs.h"
#include "stor-layout.h"

/* Global variables with GTY annotations to force type processing */
static GTY(()) tree scalar_types[5];
static GTY(()) tree pointer_types[3];
static GTY(()) tree array_types[3];
static GTY(()) tree struct_types[2];
static GTY(()) tree union_types[2];
static GTY(()) tree callback_types[2];
static GTY(()) tree string_type;
static GTY(()) tree lang_struct_type;

/* Helper to create a simple struct with variable field count */
static tree
create_test_struct(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier_with_length("field", 5),
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
create_test_union(int member_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree member_list = NULL_TREE;
  
  for (int i = 0; i < member_count; i++) {
    tree member = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier_with_length("member", 6),
                            (i % 2 == 0) ? integer_type_node : char_type_node);
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
  /* TYPE_SCALAR cases */
  scalar_types[0] = integer_type_node;
  scalar_types[1] = char_type_node;
  scalar_types[2] = boolean_type_node;
  scalar_types[3] = size_type_node;
  scalar_types[4] = ptrdiff_type_node;
  
  /* TYPE_POINTER cases */
  pointer_types[0] = build_pointer_type(integer_type_node);
  pointer_types[1] = build_pointer_type(char_type_node);
  pointer_types[2] = build_pointer_type(void_type_node);
  
  /* TYPE_ARRAY cases - using __FIELD_COUNT__ as placeholder */
  int array_dim = __FIELD_COUNT__ > 0 ? __FIELD_COUNT__ : 10;
  tree index_type = build_index_type(build_int_cst(integer_type_node, array_dim - 1));
  array_types[0] = build_array_type(char_type_node, index_type);
  array_types[1] = build_array_type(integer_type_node, index_type);
  array_types[2] = build_array_type(build_pointer_type(char_type_node), index_type);
  
  /* TYPE_STRUCT cases */
  int struct_field_count = __FIELD_COUNT__ > 0 ? __FIELD_COUNT__ : 3;
  struct_types[0] = create_test_struct(struct_field_count);
  struct_types[1] = create_test_struct(struct_field_count + 1);
  
  /* TYPE_UNION cases */
  int union_member_count = __FIELD_COUNT__ > 0 ? __FIELD_COUNT__ : 2;
  union_types[0] = create_test_union(union_member_count);
  union_types[1] = create_test_union(union_member_count + 1);
  
  /* TYPE_STRING case */
  string_type = build_pointer_type(char_type_node);
  
  /* TYPE_CALLBACK cases (function pointers) */
  tree void_ftype = build_function_type(void_type_node, NULL_TREE);
  callback_types[0] = build_pointer_type(void_ftype);
  
  tree int_ftype = build_function_type(integer_type_node, NULL_TREE);
  callback_types[1] = build_pointer_type(int_ftype);
  
  /* TYPE_LANG_STRUCT case - simulate language-specific type */
  lang_struct_type = make_node(RECORD_TYPE);
  tree lang_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                              get_identifier("lang_data"),
                              integer_type_node);
  DECL_CONTEXT(lang_field) = lang_struct_type;
  TYPE_FIELDS(lang_struct_type) = lang_field;
  
  /* Mark as language-specific structure */
#ifdef TYPE_LANG_SPECIFIC
  SET_TYPE_LANG_SPECIFIC(lang_struct_type, (struct lang_type *)1);
#endif
  
  layout_type(lang_struct_type);
  
  /* TYPE_USER_STRUCT case - using __TYPE_KIND__ to vary approach */
#ifdef TYPE_USER_STRUCT
  if (__TYPE_KIND__ == 1) {
    /* Mark as user-defined structure */
    TYPE_USER_ALIGN(struct_types[0]) = 1;
  }
#endif
  
  /* Force GC processing of all types */
  gt_ggc_mx_tree_node(&scalar_types[0]);
  gt_ggc_mx_tree_node(&pointer_types[0]);
  gt_ggc_mx_tree_node(&array_types[0]);
  gt_ggc_mx_tree_node(&struct_types[0]);
  gt_ggc_mx_tree_node(&union_types[0]);
  gt_ggc_mx_tree_node(&string_type);
  gt_ggc_mx_tree_node(&callback_types[0]);
  gt_ggc_mx_tree_node(&lang_struct_type);
  
  /* Process through alternative path if available */
#ifdef gt_pch_nx
  gt_pch_nx(&scalar_types[0]);
  gt_pch_nx(&struct_types[0]);
#endif
  
  /* Additional processing for coverage */
  for (int i = 0; i < 5; i++) {
    if (scalar_types[i])
      gt_ggc_mx_tree_node(&scalar_types[i]);
  }
  
  for (int i = 0; i < 3; i++) {
    if (pointer_types[i])
      gt_ggc_mx_tree_node(&pointer_types[i]);
    if (array_types[i])
      gt_ggc_mx_tree_node(&array_types[i]);
  }
  
  /* Create nested structures for additional coverage */
  tree nested_struct = make_node(RECORD_TYPE);
  tree nested_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("nested_int"),
                                 integer_type_node);
  tree nested_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("nested_ptr"),
                                 build_pointer_type(char_type_node));
  DECL_CONTEXT(nested_field1) = nested_struct;
  DECL_CONTEXT(nested_field2) = nested_struct;
  TYPE_FIELDS(nested_struct) = chainon(nested_field1, nested_field2);
  layout_type(nested_struct);
  
  gt_ggc_mx_tree_node(&nested_struct);
  
  /* Create array of pointers for additional coverage */
  tree array_of_ptrs_index = build_index_type(build_int_cst(integer_type_node, 4));
  tree array_of_ptrs = build_array_type(build_pointer_type(integer_type_node),
                                       array_of_ptrs_index);
  gt_ggc_mx_tree_node(&array_of_ptrs);
}

/* Main entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Alternative: Plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
