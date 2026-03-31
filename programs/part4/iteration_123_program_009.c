/* Test for gengtype type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Global variables to hold our test types */
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
static tree
create_test_struct(int field_count, const char* struct_name)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  /* Set the name for debugging */
  if (struct_name)
    TYPE_NAME(struct_type) = get_identifier(struct_name);
  
  /* Create fields based on count */
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
static tree
create_test_union(int field_count, const char* union_name)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  if (union_name)
    TYPE_NAME(union_type) = get_identifier(union_name);
  
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

/* Create a function pointer type (callback) */
static tree
create_callback_type(void)
{
  /* Build a function type returning int with no arguments */
  tree ret_type = integer_type_node;
  tree arg_types = NULL_TREE;
  tree func_type = build_function_type(ret_type, arg_types);
  
  /* Create pointer to function type */
  return build_pointer_type(func_type);
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES - TYPE_SCALAR */
  test_scalar_type = integer_type_node;
  gt_ggc_mx(tree, &test_scalar_type);
  
  test_scalar_type = char_type_node;
  gt_ggc_mx(tree, &test_scalar_type);
  
  test_scalar_type = boolean_type_node;
  gt_ggc_mx(tree, &test_scalar_type);
  
  /* 2. POINTER TYPE - TYPE_POINTER */
  test_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx(tree, &test_pointer_type);
  
  /* 3. ARRAY TYPE - TYPE_ARRAY */
  /* Create array of 10 integers */
  tree index_type = build_index_type(build_int_cst(integer_type_node, 9));
  test_array_type = build_array_type(integer_type_node, index_type);
  gt_ggc_mx(tree, &test_array_type);
  
  /* Multi-dimensional array */
  tree inner_array = build_array_type(char_type_node, index_type);
  tree outer_array = build_array_type(inner_array, index_type);
  gt_ggc_mx(tree, &outer_array);
  
  /* 4. STRUCT TYPE - TYPE_STRUCT */
  /* Create structs with varying field counts */
  test_struct_type = create_test_struct(__FIELD_COUNT__, "TestStruct");
  gt_ggc_mx(tree, &test_struct_type);
  
  /* Another struct with different layout */
  tree another_struct = create_test_struct(__FIELD_COUNT__ + 1, "AnotherStruct");
  gt_ggc_mx(tree, &another_struct);
  
  /* 5. UNION TYPE - TYPE_UNION */
  test_union_type = create_test_union(__FIELD_COUNT__, "TestUnion");
  gt_ggc_mx(tree, &test_union_type);
  
  /* 6. STRING TYPE - TYPE_STRING */
  /* char* is treated as TYPE_STRING in gengtype */
  test_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(tree, &test_string_type);
  
  /* const char* should also work */
  tree const_char_ptr = build_pointer_type(build_qualified_type(
    char_type_node, TYPE_QUAL_CONST));
  gt_ggc_mx(tree, &const_char_ptr);
  
  /* 7. CALLBACK TYPE - TYPE_CALLBACK */
  test_callback_type = create_callback_type();
  gt_ggc_mx(tree, &test_callback_type);
  
  /* Another callback with arguments */
  tree arg_type_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
  tree func_with_args = build_function_type(integer_type_node, arg_type_list);
  tree callback_with_args = build_pointer_type(func_with_args);
  gt_ggc_mx(tree, &callback_with_args);
  
  /* 8. USER STRUCT - TYPE_USER_STRUCT */
  /* Mark a struct as user-defined */
  test_user_struct_type = create_test_struct(2, "UserStruct");
  
  /* Simulate user struct marking - this depends on GCC internals */
  /* In practice, this might be set via language-specific hooks */
#ifdef TYPE_LANG_FLAG_0
  TYPE_LANG_FLAG_0(test_user_struct_type) = 1;
#endif
  gt_ggc_mx(tree, &test_user_struct_type);
  
  /* 9. LANG STRUCT - TYPE_LANG_STRUCT */
  /* Create a struct with language-specific info */
  test_lang_struct_type = create_test_struct(3, "LangStruct");
  
  /* Simulate language-specific struct */
  /* This would normally be set by front-end language code */
#ifdef TYPE_LANG_SPECIFIC
  {
    struct lang_type *lt = ggc_alloc<struct lang_type>();
    TYPE_LANG_SPECIFIC(test_lang_struct_type) = lt;
  }
#endif
  gt_ggc_mx(tree, &test_lang_struct_type);
  
  /* 10. Test TYPE_UNDEFINED path */
  /* Create an incomplete type */
  tree incomplete_struct = make_node(RECORD_TYPE);
  /* Don't add fields or layout - keep it incomplete */
  gt_ggc_mx(tree, &incomplete_struct);
  
  /* Force processing of all created types through various GTY mechanisms */
  
  /* Process through gt_pch_nx */
  gt_pch_nx(&test_scalar_type);
  gt_pch_nx(&test_pointer_type);
  gt_pch_nx(&test_array_type);
  gt_pch_nx(&test_struct_type);
  gt_pch_nx(&test_union_type);
  gt_pch_nx(&test_string_type);
  gt_pch_nx(&test_callback_type);
  gt_pch_nx(&test_user_struct_type);
  gt_pch_nx(&test_lang_struct_type);
  
  /* Also test with gt_ggc_e for edge cases */
  gt_ggc_e(test_scalar_type);
  gt_ggc_e(test_pointer_type);
  
  /* Create a complex type graph to test deep traversal */
  tree complex_struct = create_test_struct(3, "ComplexStruct");
  tree struct_ptr = build_pointer_type(complex_struct);
  tree array_of_ptrs = build_array_type(struct_ptr, index_type);
  gt_ggc_mx(tree, &array_of_ptrs);
  
  /* Mix types in a union to test nested categorization */
  tree mixed_union = make_node(UNION_TYPE);
  tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier("as_int"),
                          integer_type_node);
  tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier("as_ptr"),
                          build_pointer_type(char_type_node));
  DECL_CONTEXT(field1) = mixed_union;
  DECL_CONTEXT(field2) = mixed_union;
  TYPE_FIELDS(mixed_union) = chainon(field1, field2);
  layout_type(mixed_union);
  gt_ggc_mx(tree, &mixed_union);
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC's type system if needed */
  test_gengtype_categorization();
  return 0;
}
#endif

/* Alternative: Register as a plugin if compiled as part of GCC */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Compilation test without execution */
#ifdef COMPILE_ONLY
void dummy(void)
{
  test_gengtype_categorization();
}
#endif
