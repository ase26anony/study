/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC type nodes to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree test_scalar_type;
static GTY(()) tree test_pointer_type;
static GTY(()) tree test_array_type;
static GTY(()) tree test_struct_type;
static GTY(()) tree test_union_type;
static GTY(()) tree test_string_type;
static GTY(()) tree test_callback_type;
static GTY(()) tree test_user_struct_type;
static GTY(()) tree test_lang_struct_type;

/* Helper to create a struct with variable field count */
static tree create_test_struct(int field_count)
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

/* Helper to create a union with variable field count */
static tree create_test_union(int field_count)
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

/* Main test function */
void test_gengtype_categorization(void)
{
  /* TYPE_SCALAR - basic scalar types */
  test_scalar_type = integer_type_node;
  gt_ggc_mx(tree, &test_scalar_type);
  
  test_scalar_type = char_type_node;
  gt_ggc_mx(tree, &test_scalar_type);
  
  test_scalar_type = boolean_type_node;
  gt_ggc_mx(tree, &test_scalar_type);
  
  /* TYPE_POINTER - pointer to various types */
  test_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx(tree, &test_pointer_type);
  
  /* TYPE_ARRAY - arrays of different dimensions */
  tree index_type = build_index_type(size_int(__FIELD_COUNT__));
  test_array_type = build_array_type(integer_type_node, index_type);
  gt_ggc_mx(tree, &test_array_type);
  
  /* Multi-dimensional array */
  tree array2d = build_array_type(test_array_type, index_type);
  gt_ggc_mx(tree, &array2d);
  
  /* TYPE_STRUCT - regular struct types */
  test_struct_type = create_test_struct(__FIELD_COUNT__);
  gt_ggc_mx(tree, &test_struct_type);
  
  /* TYPE_UNION - union types */
  test_union_type = create_test_union(__FIELD_COUNT__);
  gt_ggc_mx(tree, &test_union_type);
  
  /* TYPE_STRING - string type (char pointer) */
  test_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(tree, &test_string_type);
  
  /* TYPE_CALLBACK - function pointer type */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  test_callback_type = build_pointer_type(func_type);
  gt_ggc_mx(tree, &test_callback_type);
  
  /* TYPE_USER_STRUCT - struct with user flag */
  test_user_struct_type = create_test_struct(2);
  TYPE_USER_ALIGN(test_user_struct_type) = 1;
  gt_ggc_mx(tree, &test_user_struct_type);
  
  /* TYPE_LANG_STRUCT - language-specific struct */
  test_lang_struct_type = create_test_struct(3);
  
  /* Create language-specific data */
  struct lang_type *lang_data = ggc_alloc<struct lang_type>();
  lang_data->u.specific = NULL;
  
  /* Set language-specific info */
  SET_TYPE_LANG_SPECIFIC(test_lang_struct_type, lang_data);
  gt_ggc_mx(tree, &test_lang_struct_type);
  
  /* Process all types through gengtype machinery */
  gt_types_enum_last = gt_types_enum_last;
  
  /* Force processing of all created types */
  {
    tree types[] = {
      test_scalar_type,
      test_pointer_type,
      test_array_type,
      test_struct_type,
      test_union_type,
      test_string_type,
      test_callback_type,
      test_user_struct_type,
      test_lang_struct_type,
      NULL_TREE
    };
    
    for (int i = 0; types[i]; i++) {
      if (types[i]) {
        gt_pch_nx(types[i]);
        gt_ggc_e(types[i]);
      }
    }
  }
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
