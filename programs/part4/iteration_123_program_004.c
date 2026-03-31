/* Test for gengtype.cc type categorization counters */
/* This test must be compiled as part of GCC's build system */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"

/* Guard for GCC version compatibility */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 0
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 0
#endif

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree global_scalar = NULL_TREE;
static GTY(()) tree global_pointer = NULL_TREE;
static GTY(()) tree global_array = NULL_TREE;
static GTY(()) tree global_struct = NULL_TREE;
static GTY(()) tree global_union = NULL_TREE;
static GTY(()) tree global_string = NULL_TREE;
static GTY(()) tree global_callback = NULL_TREE;
static GTY(()) tree global_user_struct = NULL_TREE;
static GTY(()) tree global_lang_struct = NULL_TREE;

/* Helper to create a struct type with configurable field count */
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

/* Helper to create a union type */
static tree
create_union_type(int variant_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < variant_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("variant"),
                           integer_type_node);
    DECL_CONTEXT(field) = union_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  return union_type;
}

/* Helper to create an array type */
static tree
create_array_type(tree element_type, int size)
{
  tree index_type = build_index_type(size_int(size - 1));
  return build_array_type(element_type, index_type);
}

/* Helper to create a callback (function pointer) type */
static tree
create_callback_type(void)
{
  tree return_type = void_type_node;
  tree arg_types = NULL_TREE;
  
  /* Add some parameters to make it interesting */
  arg_types = tree_cons(NULL_TREE, integer_type_node, arg_types);
  arg_types = tree_cons(NULL_TREE, char_type_node, arg_types);
  
  tree func_type = build_function_type(return_type, arg_types);
  return build_pointer_type(func_type);
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* TYPE_SCALAR - various scalar types */
  tree scalar_types[] = {
    integer_type_node,
    char_type_node,
    boolean_type_node,
    size_type_node,
    ptrdiff_type_node,
    NULL_TREE
  };
  
  for (int i = 0; scalar_types[i]; i++) {
    global_scalar = scalar_types[i];
    gt_ggc_mx(scalar_types[i]);
  }
  
  /* TYPE_POINTER */
  tree base_type = integer_type_node;
  tree pointer_type = build_pointer_type(base_type);
  global_pointer = pointer_type;
  gt_ggc_mx(pointer_type);
  
  /* TYPE_ARRAY - with configurable dimensions */
  tree array_types[] = {
    create_array_type(char_type_node, 10),      /* char[10] */
    create_array_type(integer_type_node, 5),    /* int[5] */
    create_array_type(pointer_type, 3),         /* int*[3] */
    NULL_TREE
  };
  
  for (int i = 0; array_types[i]; i++) {
    global_array = array_types[i];
    gt_ggc_mx(array_types[i]);
  }
  
  /* TYPE_STRUCT - with configurable field count */
  int struct_field_counts[] = {1, 3, 5, 0};
  for (int i = 0; i < sizeof(struct_field_counts)/sizeof(struct_field_counts[0]); i++) {
    tree struct_type = create_struct_type(struct_field_counts[i]);
    global_struct = struct_type;
    gt_ggc_mx(struct_type);
  }
  
  /* TYPE_UNION */
  int union_variant_counts[] = {2, 4, 1};
  for (int i = 0; i < sizeof(union_variant_counts)/sizeof(union_variant_counts[0]); i++) {
    tree union_type = create_union_type(union_variant_counts[i]);
    global_union = union_type;
    gt_ggc_mx(union_type);
  }
  
  /* TYPE_STRING - char* */
  tree string_type = build_pointer_type(char_type_node);
  global_string = string_type;
  gt_ggc_mx(string_type);
  
  /* TYPE_CALLBACK - function pointer */
  tree callback_type = create_callback_type();
  global_callback = callback_type;
  gt_ggc_mx(callback_type);
  
  /* TYPE_USER_STRUCT - mark a struct as user-defined */
  tree user_struct = create_struct_type(2);
  TYPE_USER_STRUCT(user_struct) = 1;
  global_user_struct = user_struct;
  gt_ggc_mx(user_struct);
  
  /* TYPE_LANG_STRUCT - language-specific struct */
  tree lang_struct = create_struct_type(3);
  TYPE_LANG_STRUCT(lang_struct) = 1;
  global_lang_struct = lang_struct;
  gt_ggc_mx(lang_struct);
  
  /* Process nested/complex types to ensure thorough coverage */
  tree complex_struct = create_struct_type(2);
  tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier("data"),
                          integer_type_node);
  tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier("next"),
                          build_pointer_type(complex_struct));
  TYPE_FIELDS(complex_struct) = chainon(field1, field2);
  layout_type(complex_struct);
  gt_ggc_mx(complex_struct);
  
  /* Array of structs */
  tree struct_array = create_array_type(complex_struct, 4);
  gt_ggc_mx(struct_array);
  
  /* Pointer to array */
  tree pointer_to_array = build_pointer_type(struct_array);
  gt_ggc_mx(pointer_to_array);
  
  /* Struct containing array */
  tree struct_with_array = create_struct_type(1);
  tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("arr"),
                               create_array_type(integer_type_node, 8));
  DECL_CONTEXT(array_field) = struct_with_array;
  TYPE_FIELDS(struct_with_array) = array_field;
  layout_type(struct_with_array);
  gt_ggc_mx(struct_with_array);
}

/* Entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Plugin entry point if compiled as GCC plugin */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
