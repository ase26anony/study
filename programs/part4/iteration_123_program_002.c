/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

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
create_test_union(int field_count, const char* union_name)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  if (union_name)
    TYPE_NAME(union_type) = get_identifier(union_name);
  
  for (int i = 0; i < field_count; i++) {
    tree field_type;
    
    /* Alternate between integer and pointer types for union fields */
    if (i % 2 == 0)
      field_type = integer_type_node;
    else
      field_type = build_pointer_type(integer_type_node);
    
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
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  /* Create a pointer to the function type */
  return build_pointer_type(func_type);
}

/* Create an array type with variable dimensions */
static tree
create_array_type(tree element_type, int dimensions)
{
  tree array_type = element_type;
  
  for (int i = 0; i < dimensions; i++) {
    /* Create array with 10 elements at each dimension */
    tree index_type = build_index_type(size_int(9));
    array_type = build_array_type(array_type, index_type);
  }
  
  return array_type;
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES - triggers TYPE_SCALAR case */
  test_scalar_types[0] = integer_type_node;      /* int */
  test_scalar_types[1] = char_type_node;         /* char */
  test_scalar_types[2] = boolean_type_node;      /* bool */
  
  /* Force processing of scalar types */
  for (int i = 0; i < 3; i++) {
    gt_ggc_mx(test_scalar_types[i]);
  }
  
  /* 2. POINTER TYPES - triggers TYPE_POINTER case */
  test_pointer_types[0] = build_pointer_type(integer_type_node);
  test_pointer_types[1] = build_pointer_type(char_type_node);
  
  for (int i = 0; i < 2; i++) {
    gt_ggc_mx(test_pointer_types[i]);
  }
  
  /* 3. ARRAY TYPES - triggers TYPE_ARRAY case */
  test_array_types[0] = create_array_type(integer_type_node, 1);  /* 1D array */
  test_array_types[1] = create_array_type(char_type_node, 2);     /* 2D array */
  
  for (int i = 0; i < 2; i++) {
    gt_ggc_mx(test_array_types[i]);
  }
  
  /* 4. STRUCT TYPES - triggers TYPE_STRUCT case */
  /* Create structs with different field counts for variability */
  test_struct_type = create_test_struct(__FIELD_COUNT__, "TestStruct");
  gt_ggc_mx(test_struct_type);
  
  /* 5. UNION TYPES - triggers TYPE_UNION case */
  test_union_type = create_test_union(__FIELD_COUNT__, "TestUnion");
  gt_ggc_mx(test_union_type);
  
  /* 6. STRING TYPE - triggers TYPE_STRING case */
  /* In GCC, string type is pointer to char */
  test_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(test_string_type);
  
  /* 7. CALLBACK TYPE - triggers TYPE_CALLBACK case */
  test_callback_type = create_callback_type();
  gt_ggc_mx(test_callback_type);
  
  /* 8. USER STRUCT TYPE - triggers TYPE_USER_STRUCT case */
  /* Create a struct and mark it as user-defined */
  test_user_struct_type = create_test_struct(2, "UserStruct");
  
  /* Mark as user struct - this depends on GCC internals */
  /* Using TYPE_LANG_FLAG as a marker (implementation specific) */
  TYPE_LANG_FLAG(test_user_struct_type) = 1;
  
  /* Alternative: Use TYPE_USER_ALIGN if available */
  #ifdef TYPE_USER_ALIGN
  TYPE_USER_ALIGN(test_user_struct_type) = 1;
  #endif
  
  gt_ggc_mx(test_user_struct_type);
  
  /* 9. LANG STRUCT TYPE - triggers TYPE_LANG_STRUCT case */
  /* Create a struct with language-specific info */
  test_lang_struct_type = create_test_struct(3, "LangStruct");
  
  /* Set language-specific data */
  #ifdef TYPE_LANG_SLOT_1
  TYPE_LANG_SLOT_1(test_lang_struct_type) = (tree)0x1;
  #endif
  
  /* Or use TYPE_LANG_SPECIFIC */
  #ifdef TYPE_LANG_SPECIFIC
  {
    struct lang_type *lt = ggc_alloc<struct lang_type>();
    TYPE_LANG_SPECIFIC(test_lang_struct_type) = lt;
  }
  #endif
  
  gt_ggc_mx(test_lang_struct_type);
  
  /* 10. Process all types through gengtype machinery */
  /* This ensures the counters are incremented */
  gt_ggc_mx(&test_scalar_types);
  gt_ggc_mx(&test_pointer_types);
  gt_ggc_mx(&test_array_types);
  
  /* Additional processing to ensure coverage */
  {
    /* Create and process TYPE_UNDEFINED if possible */
    tree undefined_type = make_node(ERROR_MARK);
    if (TREE_CODE(undefined_type) == ERROR_MARK) {
      /* ERROR_MARK might be categorized as TYPE_UNDEFINED */
      gt_ggc_mx(undefined_type);
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

/* GCC plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
