/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to exercise all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "stringpool.h"
#include "attribs.h"

/* Guard for GCC version compatibility */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 0
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 0
#endif

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree __scalar_type__ = NULL_TREE;
static GTY(()) tree __pointer_type__ = NULL_TREE;
static GTY(()) tree __array_type__ = NULL_TREE;
static GTY(()) tree __struct_type__ = NULL_TREE;
static GTY(()) tree __union_type__ = NULL_TREE;
static GTY(()) tree __string_type__ = NULL_TREE;
static GTY(()) tree __callback_type__ = NULL_TREE;
static GTY(()) tree __user_struct_type__ = NULL_TREE;
static GTY(()) tree __lang_struct_type__ = NULL_TREE;

/* Helper function to create a struct with variable field count */
static tree
create_struct_with_fields(int field_count)
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

/* Helper function to create a union with variable field count */
static tree
create_union_with_fields(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier_with_length("ufield", 6),
                           (i % 2 == 0) ? integer_type_node : char_type_node);
    DECL_CONTEXT(field) = union_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  return union_type;
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* TYPE_SCALAR - various scalar types */
  __scalar_type__ = integer_type_node;      /* int */
  gt_ggc_mx(integer_type_node);
  
  __scalar_type__ = char_type_node;         /* char */
  gt_ggc_mx(char_type_node);
  
  __scalar_type__ = boolean_type_node;      /* bool */
  gt_ggc_mx(boolean_type_node);
  
  __scalar_type__ = void_type_node;         /* void */
  gt_ggc_mx(void_type_node);
  
  /* TYPE_POINTER - pointer types */
  __pointer_type__ = build_pointer_type(integer_type_node);
  gt_ggc_mx(__pointer_type__);
  
  tree double_ptr = build_pointer_type(build_pointer_type(char_type_node));
  gt_ggc_mx(double_ptr);
  
  /* TYPE_ARRAY - array types with different dimensions */
  __array_type__ = build_array_type(integer_type_node, NULL_TREE);
  gt_ggc_mx(__array_type__);
  
  tree array_with_bound = build_array_type_nelts(char_type_node, 10);
  gt_ggc_mx(array_with_bound);
  
  /* Multi-dimensional array */
  tree inner_array = build_array_type(integer_type_node, NULL_TREE);
  tree multi_array = build_array_type(inner_array, NULL_TREE);
  gt_ggc_mx(multi_array);
  
  /* TYPE_STRUCT - struct types with variable field counts */
  __struct_type__ = create_struct_with_fields(__FIELD_COUNT__);
  gt_ggc_mx(__struct_type__);
  
  /* Another struct with different layout */
  tree struct2 = create_struct_with_fields(3);
  gt_ggc_mx(struct2);
  
  /* TYPE_UNION - union types */
  __union_type__ = create_union_with_fields(2);
  gt_ggc_mx(__union_type__);
  
  tree union2 = create_union_with_fields(4);
  gt_ggc_mx(union2);
  
  /* TYPE_STRING - string type (char*) */
  __string_type__ = build_pointer_type(char_type_node);
  gt_ggc_mx(__string_type__);
  
  /* Also test const char* */
  tree const_char_type = build_qualified_type(char_type_node, TYPE_QUAL_CONST);
  tree const_char_ptr = build_pointer_type(const_char_type);
  gt_ggc_mx(const_char_ptr);
  
  /* TYPE_CALLBACK - function pointer types */
  tree void_ftype = build_function_type(void_type_node, NULL_TREE);
  __callback_type__ = build_pointer_type(void_ftype);
  gt_ggc_mx(__callback_type__);
  
  /* Function type with parameters */
  tree arg_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
  tree func_type = build_function_type(integer_type_node, arg_list);
  tree func_ptr = build_pointer_type(func_type);
  gt_ggc_mx(func_ptr);
  
  /* TYPE_USER_STRUCT - marked with user flag */
  __user_struct_type__ = create_struct_with_fields(2);
  /* Mark as user struct if supported */
  #ifdef TYPE_LANG_FLAG
  TYPE_LANG_FLAG(__user_struct_type__) = 1;
  #endif
  gt_ggc_mx(__user_struct_type__);
  
  /* TYPE_LANG_STRUCT - language-specific struct */
  __lang_struct_type__ = create_struct_with_fields(2);
  /* Set language-specific data if available */
  #ifdef SET_TYPE_LANG_SPECIFIC
  SET_TYPE_LANG_SPECIFIC(__lang_struct_type__, (struct lang_type *)1);
  #endif
  gt_ggc_mx(__lang_struct_type__);
  
  /* Process all types through gengtype machinery */
  /* Force processing by using gt_pch_nx on each type */
  gt_pch_nx(&__scalar_type__);
  gt_pch_nx(&__pointer_type__);
  gt_pch_nx(&__array_type__);
  gt_pch_nx(&__struct_type__);
  gt_pch_nx(&__union_type__);
  gt_pch_nx(&__string_type__);
  gt_pch_nx(&__callback_type__);
  gt_pch_nx(&__user_struct_type__);
  gt_pch_nx(&__lang_struct_type__);
  
  /* Also test arrays of different types */
  tree type_array[8];
  type_array[0] = integer_type_node;
  type_array[1] = __pointer_type__;
  type_array[2] = __array_type__;
  type_array[3] = __struct_type__;
  type_array[4] = __union_type__;
  type_array[5] = __string_type__;
  type_array[6] = __callback_type__;
  type_array[7] = __user_struct_type__;
  
  for (int i = 0; i < 8; i++) {
    gt_ggc_mx(type_array[i]);
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

/* Plugin entry point if compiled as plugin */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
