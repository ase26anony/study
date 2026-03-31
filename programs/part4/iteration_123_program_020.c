/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* This test creates various GCC type nodes to trigger all categorization cases in gengtype.cc */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree test_scalar_types[3];
static GTY(()) tree test_pointer_types[2];
static GTY(()) tree test_array_types[2];
static GTY(()) tree test_struct_type;
static GTY(()) tree test_union_type;
static GTY(()) tree test_callback_type;
static GTY(()) tree test_string_type;
static GTY(()) tree test_user_struct_type;
static GTY(()) tree test_lang_struct_type;

/* Helper function to create a struct with variable field count */
static tree
create_test_struct(int field_count, const char *struct_name)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  /* Set the name for debugging */
  if (struct_name)
    TYPE_NAME(struct_type) = get_identifier(struct_name);
  
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

/* Helper function to create a union with variable field count */
static tree
create_test_union(int field_count, const char *union_name)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  if (union_name)
    TYPE_NAME(union_type) = get_identifier(union_name);
  
  for (int i = 0; i < field_count; i++) {
    tree field_type;
    
    /* Use different types for union fields */
    switch (i % 3) {
      case 0: field_type = integer_type_node; break;
      case 1: field_type = char_type_node; break;
      case 2: field_type = boolean_type_node; break;
      default: field_type = integer_type_node;
    }
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("u_field"),
                           field_type);
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
  /* Create a function type returning int with no arguments */
  tree return_type = integer_type_node;
  tree arg_types = NULL_TREE;
  tree func_type = build_function_type(return_type, arg_types);
  
  /* Create pointer to function */
  return build_pointer_type(func_type);
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES - TYPE_SCALAR */
  test_scalar_types[0] = integer_type_node;
  test_scalar_types[1] = char_type_node;
  test_scalar_types[2] = boolean_type_node;
  
  /* Force processing of scalar types */
  for (int i = 0; i < 3; i++) {
    gt_ggc_mx(test_scalar_types[i]);
  }
  
  /* 2. POINTER TYPES - TYPE_POINTER */
  test_pointer_types[0] = build_pointer_type(integer_type_node);
  test_pointer_types[1] = build_pointer_type(char_type_node);
  
  for (int i = 0; i < 2; i++) {
    gt_ggc_mx(test_pointer_types[i]);
  }
  
  /* 3. ARRAY TYPES - TYPE_ARRAY */
  /* Create array of 10 integers */
  tree index_type = build_index_type(build_int_cst(integer_type_node, 9));
  test_array_types[0] = build_array_type(integer_type_node, index_type);
  
  /* Create multi-dimensional array */
  tree inner_array = build_array_type(char_type_node, index_type);
  test_array_types[1] = build_array_type(inner_array, index_type);
  
  for (int i = 0; i < 2; i++) {
    gt_ggc_mx(test_array_types[i]);
  }
  
  /* 4. STRUCT TYPES - TYPE_STRUCT */
  /* Create struct with __FIELD_COUNT__ fields (placeholder for mutation) */
  int field_count = __FIELD_COUNT__;
  if (field_count < 1) field_count = 3; /* Default */
  if (field_count > 10) field_count = 10; /* Limit */
  
  test_struct_type = create_test_struct(field_count, "test_struct");
  gt_ggc_mx(test_struct_type);
  
  /* 5. UNION TYPES - TYPE_UNION */
  test_union_type = create_test_union(field_count, "test_union");
  gt_ggc_mx(test_union_type);
  
  /* 6. STRING TYPE - TYPE_STRING */
  /* String type is pointer to char */
  test_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(test_string_type);
  
  /* 7. CALLBACK TYPE - TYPE_CALLBACK */
  test_callback_type = create_callback_type();
  gt_ggc_mx(test_callback_type);
  
  /* 8. USER STRUCT - TYPE_USER_STRUCT */
  /* Create a struct and mark it as user-defined */
  test_user_struct_type = create_test_struct(2, "user_struct");
  
  /* Mark as user struct - using TYPE_LANG_FLAG_5 as a marker */
  /* This is implementation-dependent and may need adjustment */
  TYPE_LANG_FLAG_5(test_user_struct_type) = 1;
  gt_ggc_mx(test_user_struct_type);
  
  /* 9. LANG STRUCT - TYPE_LANG_STRUCT */
  /* Create a struct with language-specific data */
  test_lang_struct_type = create_test_struct(2, "lang_struct");
  
  /* Simulate language-specific type by setting lang-specific info */
  /* This would normally be set by language frontends */
#ifdef TYPE_LANG_STRUCT
  /* Use implementation-specific method to mark as lang struct */
  TYPE_LANG_SPECIFIC(test_lang_struct_type) = (struct lang_type *)1;
#endif
  gt_ggc_mx(test_lang_struct_type);
  
  /* 10. Process all types again in different order to ensure coverage */
  /* This helps catch any edge cases in the switch statement */
  tree all_types[] = {
    test_scalar_types[0],
    test_pointer_types[0],
    test_array_types[0],
    test_struct_type,
    test_union_type,
    test_string_type,
    test_callback_type,
    test_user_struct_type,
    test_lang_struct_type
  };
  
  for (size_t i = 0; i < sizeof(all_types)/sizeof(all_types[0]); i++) {
    if (all_types[i]) {
      gt_pch_nx(all_types[i]);
    }
  }
  
  /* Create some complex nested types to test edge cases */
  if (__TYPE_KIND__ == 1) {
    /* Nested struct with pointer to array */
    tree nested_struct = create_test_struct(2, "nested");
    tree ptr_to_array = build_pointer_type(test_array_types[0]);
    gt_ggc_mx(nested_struct);
    gt_ggc_mx(ptr_to_array);
  } else if (__TYPE_KIND__ == 2) {
    /* Union containing struct */
    tree inner_struct = create_test_struct(2, "inner");
    tree complex_union = create_test_union(1, "complex_union");
    gt_ggc_mx(inner_struct);
    gt_ggc_mx(complex_union);
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

/* Alternative: Register as a plugin if compiled as such */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
