/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "ggc.h"

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree scalar_types[3];
static GTY(()) tree pointer_types[2];
static GTY(()) tree array_types[2];
static GTY(()) tree struct_types[2];
static GTY(()) tree union_types[1];
static GTY(()) tree string_type;
static GTY(()) tree callback_types[2];
static GTY(()) tree lang_struct_type;
static GTY(()) tree user_struct_type;

/* Helper function to create a struct with variable field count */
static tree
create_struct_type(int field_count)
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

/* Helper function to create a union with variable field count */
static tree
create_union_type(int field_count)
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
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* TYPE_SCALAR - covers nb_scalar++ */
  scalar_types[0] = integer_type_node;
  scalar_types[1] = char_type_node;
  scalar_types[2] = boolean_type_node;
  
  /* TYPE_POINTER - covers nb_pointer++ */
  pointer_types[0] = build_pointer_type(integer_type_node);
  pointer_types[1] = build_pointer_type(char_type_node);
  
  /* TYPE_ARRAY - covers nb_array++ */
  tree index_type = build_index_type(size_int(10));
  array_types[0] = build_array_type(integer_type_node, index_type);
  array_types[1] = build_array_type_nelts(char_type_node, 20);
  
  /* TYPE_STRUCT - covers nb_struct++ */
  struct_types[0] = create_struct_type(2);  /* Struct with 2 fields */
  struct_types[1] = create_struct_type(3);  /* Struct with 3 fields */
  
  /* TYPE_UNION - covers nb_union++ */
  union_types[0] = create_union_type(2);    /* Union with 2 fields */
  
  /* TYPE_STRING - covers nb_string++ */
  string_type = build_pointer_type(char_type_node);
  
  /* TYPE_CALLBACK - covers nb_callback++ */
  tree void_type = void_type_node;
  tree func_type = build_function_type(void_type, NULL_TREE);
  callback_types[0] = build_pointer_type(func_type);
  
  /* More complex callback type */
  tree arg_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
  arg_list = tree_cons(NULL_TREE, char_type_node, arg_list);
  tree func_type2 = build_function_type(integer_type_node, arg_list);
  callback_types[1] = build_pointer_type(func_type2);
  
  /* TYPE_LANG_STRUCT - covers nb_lang_struct++ */
  lang_struct_type = create_struct_type(1);
  SET_TYPE_LANG_SPECIFIC(lang_struct_type, (struct lang_type *)1);
  
  /* TYPE_USER_STRUCT - covers nb_user_struct++ */
  user_struct_type = create_struct_type(1);
  TYPE_USER_ALIGN(user_struct_type) = 1;
  
  /* Force GC marking of all types to trigger gengtype processing */
  gt_ggc_mx(scalar_types);
  gt_ggc_mx(pointer_types);
  gt_ggc_mx(array_types);
  gt_ggc_mx(struct_types);
  gt_ggc_mx(union_types);
  gt_ggc_mx(&string_type);
  gt_ggc_mx(callback_types);
  gt_ggc_mx(&lang_struct_type);
  gt_ggc_mx(&user_struct_type);
  
  /* Additional processing through different paths */
  for (int i = 0; i < 3; i++) {
    if (scalar_types[i])
      gt_pch_nx(&scalar_types[i]);
  }
  
  for (int i = 0; i < 2; i++) {
    if (pointer_types[i])
      gt_pch_nx(&pointer_types[i]);
    if (array_types[i])
      gt_pch_nx(&array_types[i]);
    if (struct_types[i])
      gt_pch_nx(&struct_types[i]);
    if (callback_types[i])
      gt_pch_nx(&callback_types[i]);
  }
  
  if (union_types[0])
    gt_pch_nx(&union_types[0]);
  
  /* Process through ggc_type_tab if available */
#ifdef ggc_type_tab
  for (int i = 0; i < gt_types_enum_last; i++) {
    if (ggc_type_tab[i])
      gt_ggc_mx(ggc_type_tab[i]);
  }
#endif
}

/* Main function for standalone testing */
#ifdef TEST_STANDALONE
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Plugin entry point for GCC plugin testing */
#ifdef PLUGIN_TEST
int plugin_is_GPL_compatible = 1;

int
plugin_init(struct plugin_name_args *plugin_info,
            struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
