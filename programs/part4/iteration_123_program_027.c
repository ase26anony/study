/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

/* Global variables with GTY annotations to force type processing */
static GTY(()) tree scalar_types[3];
static GTY(()) tree pointer_types[2];
static GTY(()) tree array_types[2];
static GTY(()) tree struct_types[2];
static GTY(()) tree union_types[1];
static GTY(()) tree callback_types[1];
static GTY(()) tree string_type;
static GTY(()) tree lang_struct_type;
static GTY(()) tree user_struct_type;

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
  /* TYPE_SCALAR cases */
  scalar_types[0] = integer_type_node;
  scalar_types[1] = char_type_node;
  scalar_types[2] = boolean_type_node;
  
  /* TYPE_POINTER cases */
  pointer_types[0] = build_pointer_type(integer_type_node);
  pointer_types[1] = build_pointer_type(char_type_node);
  
  /* TYPE_ARRAY cases - using variable dimensions */
  tree array_index_type = build_index_type(size_int(__FIELD_COUNT__));
  array_types[0] = build_array_type(integer_type_node, array_index_type);
  
  tree array_index_type2 = build_index_type(size_int(__FIELD_COUNT__ * 2));
  array_types[1] = build_array_type_nelts(char_type_node, __FIELD_COUNT__);
  
  /* TYPE_STRUCT cases */
  struct_types[0] = create_struct_with_fields(__FIELD_COUNT__);
  struct_types[1] = create_struct_with_fields(__FIELD_COUNT__ + 1);
  
  /* TYPE_UNION cases */
  union_types[0] = create_union_with_members(__FIELD_COUNT__);
  
  /* TYPE_STRING case */
  string_type = build_pointer_type(char_type_node);
  
  /* TYPE_CALLBACK case (function pointer) */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  callback_types[0] = build_pointer_type(func_type);
  
  /* TYPE_LANG_STRUCT case */
  lang_struct_type = create_struct_with_fields(2);
  SET_TYPE_LANG_SPECIFIC(lang_struct_type, (struct lang_type *)1);
  
  /* TYPE_USER_STRUCT case */
  user_struct_type = create_struct_with_fields(3);
  TYPE_USER_STRUCT(user_struct_type) = 1;
  
  /* Force GC processing of all types */
  gt_ggc_mx (scalar_types);
  gt_ggc_mx (pointer_types);
  gt_ggc_mx (array_types);
  gt_ggc_mx (struct_types);
  gt_ggc_mx (union_types);
  gt_ggc_mx (&string_type);
  gt_ggc_mx (callback_types);
  gt_ggc_mx (&lang_struct_type);
  gt_ggc_mx (&user_struct_type);
  
  /* Additional processing to ensure coverage */
  for (int i = 0; i < 3; i++) {
    if (scalar_types[i])
      gt_ggc_mx (&scalar_types[i]);
  }
  
  /* Process nested types */
  tree nested_struct = create_struct_with_fields(1);
  tree nested_array = build_array_type(nested_struct, array_index_type);
  gt_ggc_mx (&nested_array);
  
  /* Void pointer for completeness */
  tree void_ptr = build_pointer_type(void_type_node);
  gt_ggc_mx (&void_ptr);
  
  /* Complex type: pointer to array of structs */
  tree complex_type = build_pointer_type(
    build_array_type(
      create_struct_with_fields(2),
      build_index_type(size_int(5))
    )
  );
  gt_ggc_mx (&complex_type);
}

/* Main entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Alternative: Plugin initialization */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
