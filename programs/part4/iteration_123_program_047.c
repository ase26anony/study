/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* This test creates various GCC internal type nodes to exercise all type_enum categories */

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

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree global_scalar_type;
static GTY(()) tree global_pointer_type;
static GTY(()) tree global_array_type;
static GTY(()) tree global_struct_type;
static GTY(()) tree global_union_type;
static GTY(()) tree global_string_type;
static GTY(()) tree global_callback_type;
static GTY(()) tree global_user_struct_type;
static GTY(()) tree global_lang_struct_type;

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
void
test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES - TYPE_SCALAR */
  global_scalar_type = integer_type_node;
  gt_ggc_mx(global_scalar_type);
  
  /* Also test other scalar types */
  gt_ggc_mx(char_type_node);
  gt_ggc_mx(boolean_type_node);
  gt_ggc_mx(size_type_node);
  gt_ggc_mx(ssizetype);
  
  /* 2. POINTER TYPES - TYPE_POINTER */
  tree ptr_to_int = build_pointer_type(integer_type_node);
  global_pointer_type = ptr_to_int;
  gt_ggc_mx(global_pointer_type);
  
  /* Multiple pointer variations */
  gt_ggc_mx(build_pointer_type(char_type_node));
  gt_ggc_mx(build_pointer_type(ptr_to_int)); /* pointer to pointer */
  
  /* 3. ARRAY TYPES - TYPE_ARRAY */
  tree array_index_type = build_index_type(size_int(10));
  tree int_array_type = build_array_type(integer_type_node, array_index_type);
  global_array_type = int_array_type;
  gt_ggc_mx(global_array_type);
  
  /* Variable length array */
  tree vla_type = build_array_type_nelts(char_type_node, size_int(__FIELD_COUNT__));
  gt_ggc_mx(vla_type);
  
  /* Multi-dimensional array */
  tree md_array_type = build_array_type(int_array_type, array_index_type);
  gt_ggc_mx(md_array_type);
  
  /* 4. STRUCT TYPES - TYPE_STRUCT */
  tree simple_struct = create_struct_with_fields(__FIELD_COUNT__);
  global_struct_type = simple_struct;
  gt_ggc_mx(global_struct_type);
  
  /* Struct with pointer field */
  tree struct_with_ptr = make_node(RECORD_TYPE);
  tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("ptr_field"),
                             ptr_to_int);
  DECL_CONTEXT(ptr_field) = struct_with_ptr;
  TYPE_FIELDS(struct_with_ptr) = ptr_field;
  layout_type(struct_with_ptr);
  gt_ggc_mx(struct_with_ptr);
  
  /* 5. UNION TYPES - TYPE_UNION */
  tree simple_union = create_union_with_fields(__FIELD_COUNT__);
  global_union_type = simple_union;
  gt_ggc_mx(global_union_type);
  
  /* 6. STRING TYPE - TYPE_STRING */
  /* In GCC, string type is typically char* */
  tree string_ptr_type = build_pointer_type(char_type_node);
  global_string_type = string_ptr_type;
  gt_ggc_mx(global_string_type);
  
  /* Also test const char* */
  tree const_char_type = build_qualified_type(char_type_node, TYPE_QUAL_CONST);
  tree const_string_type = build_pointer_type(const_char_type);
  gt_ggc_mx(const_string_type);
  
  /* 7. CALLBACK TYPES - TYPE_CALLBACK (function pointers) */
  tree void_type = void_type_node;
  tree func_type = build_function_type(void_type, NULL_TREE);
  tree func_ptr_type = build_pointer_type(func_type);
  global_callback_type = func_ptr_type;
  gt_ggc_mx(global_callback_type);
  
  /* Function type with parameters */
  tree arg_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
  tree func_with_args = build_function_type(integer_type_node, arg_list);
  tree func_ptr_with_args = build_pointer_type(func_with_args);
  gt_ggc_mx(func_ptr_with_args);
  
  /* 8. USER STRUCT TYPES - TYPE_USER_STRUCT */
  /* User structs are marked with special flags or language-specific data */
  tree user_struct = create_struct_with_fields(2);
  
#ifdef TYPE_LANG_FLAG_0
  TYPE_LANG_FLAG_0(user_struct) = 1;
#endif
  
  /* Try to mark as user struct through various mechanisms */
#ifdef TYPE_USER_STRUCT
  /* Use type flag if available */
  TYPE_USER_STRUCT(user_struct) = 1;
#endif
  
  global_user_struct_type = user_struct;
  gt_ggc_mx(global_user_struct_type);
  
  /* 9. LANG STRUCT TYPES - TYPE_LANG_STRUCT */
  /* Language-specific structs have TYPE_LANG_SPECIFIC set */
  tree lang_struct = create_struct_with_fields(3);
  
#ifdef TYPE_LANG_SPECIFIC
  /* Allocate and set language-specific data if supported */
  struct lang_type *lang_data = ggc_alloc<struct lang_type>();
  SET_TYPE_LANG_SPECIFIC(lang_struct, lang_data);
#endif
  
  global_lang_struct_type = lang_struct;
  gt_ggc_mx(global_lang_struct_type);
  
  /* 10. Test TYPE_UNDEFINED - might occur with incomplete types */
  tree incomplete_struct = make_node(RECORD_TYPE);
  /* Don't add fields or layout, keeping it incomplete */
  gt_ggc_mx(incomplete_struct);
  
  /* Test nested types */
  tree struct_with_nested = make_node(RECORD_TYPE);
  tree nested_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("nested"),
                                simple_struct);
  DECL_CONTEXT(nested_field) = struct_with_nested;
  TYPE_FIELDS(struct_with_nested) = nested_field;
  layout_type(struct_with_nested);
  gt_ggc_mx(struct_with_nested);
  
  /* Force processing of all globals through ggc_mark */
  gt_ggc_mx(global_scalar_type);
  gt_ggc_mx(global_pointer_type);
  gt_ggc_mx(global_array_type);
  gt_ggc_mx(global_struct_type);
  gt_ggc_mx(global_union_type);
  gt_ggc_mx(global_string_type);
  gt_ggc_mx(global_callback_type);
  gt_ggc_mx(global_user_struct_type);
  gt_ggc_mx(global_lang_struct_type);
}

/* Main entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC environment if needed */
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
