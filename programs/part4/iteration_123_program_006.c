/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Guard for GCC version compatibility */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 4
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 9
#endif

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
create_test_struct(const char *name, int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  /* Set the name for debugging */
  if (name)
    TYPE_NAME(struct_type) = get_identifier(name);
  
  /* Create fields based on field_count parameter */
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
create_test_union(const char *name, int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  if (name)
    TYPE_NAME(union_type) = get_identifier(name);
  
  for (int i = 0; i < field_count; i++) {
    tree field_type = (i % 2 == 0) ? integer_type_node : char_type_node;
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("union_field"),
                           field_type);
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
  /* 1. SCALAR types - TYPE_SCALAR */
  test_scalar_type = integer_type_node;  /* int */
  gt_ggc_mx(tree, &test_scalar_type);
  
  test_scalar_type = boolean_type_node;  /* bool */
  gt_ggc_mx(tree, &test_scalar_type);
  
  test_scalar_type = char_type_node;     /* char */
  gt_ggc_mx(tree, &test_scalar_type);
  
  /* 2. POINTER types - TYPE_POINTER */
  test_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx(tree, &test_pointer_type);
  
  /* 3. ARRAY types - TYPE_ARRAY */
  /* Create array with __FIELD_COUNT__ elements (placeholder) */
  int array_size = __FIELD_COUNT__ > 0 ? __FIELD_COUNT__ : 10;
  tree index_type = build_index_type(size_int(array_size - 1));
  test_array_type = build_array_type(integer_type_node, index_type);
  gt_ggc_mx(tree, &test_array_type);
  
  /* Multi-dimensional array */
  tree inner_array = build_array_type(char_type_node, index_type);
  tree outer_array = build_array_type(inner_array, index_type);
  gt_ggc_mx(tree, &outer_array);
  
  /* 4. STRUCT types - TYPE_STRUCT */
  test_struct_type = create_test_struct("TestStruct", __FIELD_COUNT__);
  gt_ggc_mx(tree, &test_struct_type);
  
  /* Nested struct */
  tree inner_struct = create_test_struct("InnerStruct", 2);
  tree outer_struct = create_test_struct("OuterStruct", 3);
  tree nested_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("nested"),
                                inner_struct);
  DECL_CONTEXT(nested_field) = outer_struct;
  TYPE_FIELDS(outer_struct) = chainon(TYPE_FIELDS(outer_struct), nested_field);
  layout_type(outer_struct);
  gt_ggc_mx(tree, &outer_struct);
  
  /* 5. UNION types - TYPE_UNION */
  test_union_type = create_test_union("TestUnion", __FIELD_COUNT__);
  gt_ggc_mx(tree, &test_union_type);
  
  /* 6. STRING type - TYPE_STRING */
  /* In GCC, string type is pointer to char */
  test_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(tree, &test_string_type);
  
  /* Also test const string */
  tree const_char_type = build_qualified_type(char_type_node, TYPE_QUAL_CONST);
  tree const_string_type = build_pointer_type(const_char_type);
  gt_ggc_mx(tree, &const_string_type);
  
  /* 7. CALLBACK type - TYPE_CALLBACK (function pointer) */
  /* Create function type returning int with no arguments */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  test_callback_type = build_pointer_type(func_type);
  gt_ggc_mx(tree, &test_callback_type);
  
  /* Function with parameters */
  tree param_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
  param_list = tree_cons(NULL_TREE, char_type_node, param_list);
  tree func_with_params = build_function_type(integer_type_node, param_list);
  tree func_ptr_with_params = build_pointer_type(func_with_params);
  gt_ggc_mx(tree, &func_ptr_with_params);
  
  /* 8. USER STRUCT type - TYPE_USER_STRUCT */
  /* Create a struct and mark it as user-defined */
  test_user_struct_type = create_test_struct("UserStruct", 2);
  
  /* Mark as user struct - implementation depends on GCC version */
  #ifdef TYPE_LANG_FLAG
  TYPE_LANG_FLAG(test_user_struct_type) = 1;
  #endif
  
  /* Alternative: Use GTY markers */
  struct GTY(()) user_defined_struct {
    int field1;
    char field2;
  };
  
  static GTY(()) struct user_defined_struct user_struct_instance;
  user_struct_instance.field1 = 42;
  user_struct_instance.field2 = 'A';
  
  gt_ggc_mx(user_defined_struct, &user_struct_instance);
  gt_ggc_mx(tree, &test_user_struct_type);
  
  /* 9. LANG STRUCT type - TYPE_LANG_STRUCT */
  /* Create a language-specific struct type */
  test_lang_struct_type = create_test_struct("LangStruct", 3);
  
  /* Mark as language-specific struct */
  #ifdef SET_TYPE_LANG_SPECIFIC
  SET_TYPE_LANG_SPECIFIC(test_lang_struct_type);
  #endif
  
  /* Alternative approach: create a type with language-specific attributes */
  tree lang_struct = make_node(RECORD_TYPE);
  TYPE_NAME(lang_struct) = get_identifier("LanguageSpecificType");
  
  /* Add some language-specific marker */
  #ifdef TYPE_LANG_SLOT_1
  TYPE_LANG_SLOT_1(lang_struct) = integer_type_node;
  #endif
  
  layout_type(lang_struct);
  gt_ggc_mx(tree, &lang_struct);
  gt_ggc_mx(tree, &test_lang_struct_type);
  
  /* Process all types through gengtype system */
  /* Force GC marking of all test variables */
  gt_ggc_mx(tree, &test_scalar_type);
  gt_ggc_mx(tree, &test_pointer_type);
  gt_ggc_mx(tree, &test_array_type);
  gt_ggc_mx(tree, &test_struct_type);
  gt_ggc_mx(tree, &test_union_type);
  gt_ggc_mx(tree, &test_string_type);
  gt_ggc_mx(tree, &test_callback_type);
  gt_ggc_mx(tree, &test_user_struct_type);
  gt_ggc_mx(tree, &test_lang_struct_type);
  
  /* Also test with NULL types */
  tree null_tree = NULL_TREE;
  gt_ggc_mx(tree, &null_tree);
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC environment if needed */
  #ifdef GCC_INITIALIZE
  gcc_init();
  #endif
  
  test_gengtype_categorization();
  
  /* Force garbage collection to trigger type processing */
  #ifdef GGC_COLLECT
  ggc_collect();
  #endif
  
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

/* Compile-time variations for mutation testing */
#if __TYPE_KIND__ == 1
/* Focus on scalar types */
void test_scalar_variants(void)
{
  tree types[] = {
    integer_type_node,
    char_type_node,
    boolean_type_node,
    void_type_node,
    size_type_node,
    ptrdiff_type_node
  };
  
  for (unsigned i = 0; i < ARRAY_SIZE(types); i++) {
    gt_ggc_mx(tree, &types[i]);
  }
}
#endif

#if __TYPE_KIND__ == 2
/* Focus on aggregate types */
void test_aggregate_variants(void)
{
  /* Create structs with different field counts */
  for (int i = 1; i <= 10; i++) {
    tree s = create_test_struct("VarStruct", i);
    gt_ggc_mx(tree, &s);
    
    tree u = create_test_union("VarUnion", i);
    gt_ggc_mx(tree, &u);
  }
}
#endif
