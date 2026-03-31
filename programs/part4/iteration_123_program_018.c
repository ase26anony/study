/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* This test creates various GCC type nodes to trigger all type_enum classification cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

/* Forward declarations for helper functions */
static tree create_test_struct(int field_count);
static tree create_test_union(int field_count);
static tree create_test_array(tree element_type, int dimensions);
static tree create_test_callback(void);

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree global_scalar_type = NULL_TREE;
static GTY(()) tree global_pointer_type = NULL_TREE;
static GTY(()) tree global_array_type = NULL_TREE;
static GTY(()) tree global_struct_type = NULL_TREE;
static GTY(()) tree global_union_type = NULL_TREE;
static GTY(()) tree global_callback_type = NULL_TREE;
static GTY(()) tree global_string_type = NULL_TREE;

/* Variable struct with different field counts */
static GTY(()) tree global_struct_variant_1 = NULL_TREE;
static GTY(()) tree global_struct_variant_2 = NULL_TREE;
static GTY(()) tree global_struct_variant_3 = NULL_TREE;

/* Array variants */
static GTY(()) tree global_array_variant_1 = NULL_TREE;
static GTY(()) tree global_array_variant_2 = NULL_TREE;

/* Callback variants */
static GTY(()) tree global_callback_variant_1 = NULL_TREE;
static GTY(()) tree global_callback_variant_2 = NULL_TREE;

/* Language-specific struct simulation */
#ifdef TYPE_LANG_STRUCT
static GTY(()) tree global_lang_struct = NULL_TREE;
#endif

/* User struct simulation */
#ifdef TYPE_USER_STRUCT
static GTY(()) tree global_user_struct = NULL_TREE;
#endif

/* Helper to create a struct with specified number of fields */
static tree
create_test_struct(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  /* Set up basic type attributes */
  TYPE_NAME(struct_type) = get_identifier("test_struct");
  TYPE_PACKED(struct_type) = 0;
  TYPE_ALIGN(struct_type) = BITS_PER_UNIT;
  
  /* Create fields based on field_count parameter */
  for (int i = 0; i < field_count; i++) {
    tree field_type;
    
    /* Alternate field types for variety */
    switch (i % 4) {
      case 0:
        field_type = integer_type_node;
        break;
      case 1:
        field_type = char_type_node;
        break;
      case 2:
        field_type = boolean_type_node;
        break;
      case 3:
        field_type = build_pointer_type(integer_type_node);
        break;
      default:
        field_type = integer_type_node;
    }
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(ASM_GENERATE_INTERNAL_LABEL("field", i, 0)),
                           field_type);
    
    DECL_CONTEXT(field) = struct_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(struct_type) = field_list;
  layout_type(struct_type);
  
  return struct_type;
}

/* Helper to create a union with specified number of fields */
static tree
create_test_union(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  TYPE_NAME(union_type) = get_identifier("test_union");
  TYPE_ALIGN(union_type) = BITS_PER_UNIT;
  
  for (int i = 0; i < field_count; i++) {
    tree field_type;
    
    switch (i % 3) {
      case 0:
        field_type = integer_type_node;
        break;
      case 1:
        field_type = char_type_node;
        break;
      case 2:
        field_type = build_pointer_type(char_type_node);
        break;
      default:
        field_type = integer_type_node;
    }
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(ASM_GENERATE_INTERNAL_LABEL("u_field", i, 0)),
                           field_type);
    
    DECL_CONTEXT(field) = union_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  
  return union_type;
}

/* Helper to create multi-dimensional arrays */
static tree
create_test_array(tree element_type, int dimensions)
{
  tree array_type = element_type;
  
  for (int i = 0; i < dimensions; i++) {
    /* Create array with 5 elements for each dimension */
    tree index_type = build_index_type(size_int(4));
    array_type = build_array_type(array_type, index_type);
  }
  
  return array_type;
}

/* Helper to create callback (function pointer) types */
static tree
create_test_callback(void)
{
  /* Create a function type returning int with varying parameters */
  tree return_type = integer_type_node;
  tree arg_types = NULL_TREE;
  
  /* Add some parameters */
  tree arg1 = build_pointer_type(char_type_node);
  tree arg2 = integer_type_node;
  
  arg_types = tree_cons(NULL_TREE, arg1, arg_types);
  arg_types = tree_cons(NULL_TREE, arg2, arg_types);
  
  tree func_type = build_function_type(return_type, arg_types);
  tree func_ptr_type = build_pointer_type(func_type);
  
  return func_ptr_type;
}

/* Main test function */
void
test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES - TYPE_SCALAR */
  global_scalar_type = integer_type_node;
  gt_ggc_mx(global_scalar_type);
  
  /* Also process other scalar types */
  gt_ggc_mx(char_type_node);
  gt_ggc_mx(boolean_type_node);
  gt_ggc_mx(size_type_node);
  
  /* 2. POINTER TYPES - TYPE_POINTER */
  global_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx(global_pointer_type);
  
  /* Additional pointer variants */
  tree char_ptr = build_pointer_type(char_type_node);
  gt_ggc_mx(char_ptr);
  
  /* 3. STRING TYPE - TYPE_STRING */
  /* In GCC, string type is typically char* */
  global_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(global_string_type);
  
  /* 4. ARRAY TYPES - TYPE_ARRAY */
  /* Single-dimensional array */
  global_array_type = create_test_array(integer_type_node, 1);
  gt_ggc_mx(global_array_type);
  
  /* Multi-dimensional array variants */
  global_array_variant_1 = create_test_array(char_type_node, 2);
  gt_ggc_mx(global_array_variant_1);
  
  global_array_variant_2 = create_test_array(build_pointer_type(integer_type_node), 3);
  gt_ggc_mx(global_array_variant_2);
  
  /* 5. STRUCT TYPES - TYPE_STRUCT */
  /* Create structs with different field counts */
  global_struct_type = create_test_struct(__FIELD_COUNT__);
  gt_ggc_mx(global_struct_type);
  
  global_struct_variant_1 = create_test_struct(1);
  gt_ggc_mx(global_struct_variant_1);
  
  global_struct_variant_2 = create_test_struct(5);
  gt_ggc_mx(global_struct_variant_2);
  
  global_struct_variant_3 = create_test_struct(10);
  gt_ggc_mx(global_struct_variant_3);
  
  /* 6. UNION TYPES - TYPE_UNION */
  global_union_type = create_test_union(__FIELD_COUNT__);
  gt_ggc_mx(global_union_type);
  
  /* Additional union variant */
  tree union_variant = create_test_union(3);
  gt_ggc_mx(union_variant);
  
  /* 7. CALLBACK TYPES - TYPE_CALLBACK */
  global_callback_type = create_test_callback();
  gt_ggc_mx(global_callback_type);
  
  global_callback_variant_1 = create_test_callback();
  gt_ggc_mx(global_callback_variant_1);
  
  /* Another callback variant with different signature */
  tree simple_callback = build_pointer_type(
    build_function_type(integer_type_node, NULL_TREE));
  gt_ggc_mx(simple_callback);
  
  /* 8. LANGUAGE-SPECIFIC STRUCT - TYPE_LANG_STRUCT */
#ifdef TYPE_LANG_STRUCT
  global_lang_struct = create_test_struct(3);
  /* Mark as language-specific */
  SET_TYPE_LANG_SPECIFIC(global_lang_struct);
  gt_ggc_mx(global_lang_struct);
#endif
  
  /* 9. USER STRUCT - TYPE_USER_STRUCT */
#ifdef TYPE_USER_STRUCT
  global_user_struct = create_test_struct(2);
  /* Mark as user struct - implementation depends on GCC version */
  TYPE_USER_ALIGN(global_user_struct) = 1;
  gt_ggc_mx(global_user_struct);
#endif
  
  /* 10. Process undefined type placeholder */
  /* TYPE_UNDEFINED might be triggered by error_mark_node */
  gt_ggc_mx(error_mark_node);
  
  /* Force processing of all created types through multiple passes */
  for (int i = 0; i < 3; i++) {
    gt_pch_nx(global_scalar_type);
    gt_pch_nx(global_pointer_type);
    gt_pch_nx(global_array_type);
    gt_pch_nx(global_struct_type);
    gt_pch_nx(global_union_type);
    gt_pch_nx(global_callback_type);
    gt_pch_nx(global_string_type);
  }
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
