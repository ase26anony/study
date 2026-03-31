/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC type nodes to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "ggc.h"

/* Forward declarations */
static void create_and_process_types(void);
static tree create_test_struct(int field_count);
static tree create_test_union(int field_count);
static tree create_test_array(tree element_type, int dimensions);
static tree create_test_callback(void);

/* GTY-annotated globals to force gengtype processing */
static GTY(()) tree global_scalar_type = NULL_TREE;
static GTY(()) tree global_pointer_type = NULL_TREE;
static GTY(()) tree global_array_type = NULL_TREE;
static GTY(()) tree global_struct_type = NULL_TREE;
static GTY(()) tree global_union_type = NULL_TREE;
static GTY(()) tree global_callback_type = NULL_TREE;
static GTY(()) tree global_string_type = NULL_TREE;

/* Variable struct with different field counts */
static GTY(()) tree global_struct_variant1 = NULL_TREE;
static GTY(()) tree global_struct_variant2 = NULL_TREE;

/* Array variants */
static GTY(()) tree global_array_variant1 = NULL_TREE;
static GTY(()) tree global_array_variant2 = NULL_TREE;

/* Create a user-defined struct type */
static tree create_user_struct(void)
{
  tree type = make_node(RECORD_TYPE);
  tree field1, field2, field_list;
  
  /* Create fields */
  field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                     get_identifier("user_field1"),
                     integer_type_node);
  field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                     get_identifier("user_field2"),
                     char_type_node);
  
  /* Link fields */
  DECL_CHAIN(field1) = field2;
  TYPE_FIELDS(type) = field1;
  
  /* Mark as user struct - this may require special handling */
  /* TYPE_USER_STRUCT is typically set via GTY markers */
  
  return type;
}

/* Create a language-specific struct */
static tree create_lang_struct(void)
{
  tree type = make_node(RECORD_TYPE);
  tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                         get_identifier("lang_field"),
                         ptr_type_node);
  
  TYPE_FIELDS(type) = field;
  
  /* Set language-specific flag if available */
#ifdef TYPE_LANG_FLAG_0
  TYPE_LANG_FLAG_0(type) = 1;
#endif
  
  /* Try to set lang-specific data */
  if (TYPE_LANG_SPECIFIC(type) == NULL) {
    /* Allocate lang-specific structure if possible */
    /* This is architecture/language dependent */
  }
  
  return type;
}

/* Main test function */
void test_gengtype_categorization(void)
{
  /* Create and process each type category */
  
  /* 1. SCALAR types */
  global_scalar_type = integer_type_node;
  gt_ggc_mx(global_scalar_type);
  
  /* Also process other scalar types */
  gt_ggc_mx(char_type_node);
  gt_ggc_mx(boolean_type_node);
  gt_ggc_mx(void_type_node);
  
  /* 2. POINTER types */
  global_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx(global_pointer_type);
  
  /* Multiple pointer variants */
  gt_ggc_mx(build_pointer_type(char_type_node));
  gt_ggc_mx(build_pointer_type(void_type_node));
  
  /* 3. STRING type (char pointer) */
  global_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(global_string_type);
  
  /* 4. ARRAY types */
  global_array_type = build_array_type(integer_type_node, NULL_TREE);
  gt_ggc_mx(global_array_type);
  
  /* Array variants with different dimensions */
  global_array_variant1 = create_test_array(integer_type_node, 1);
  global_array_variant2 = create_test_array(char_type_node, 2);
  gt_ggc_mx(global_array_variant1);
  gt_ggc_mx(global_array_variant2);
  
  /* 5. STRUCT types */
  global_struct_type = create_test_struct(__FIELD_COUNT__);
  gt_ggc_mx(global_struct_type);
  
  /* Struct variants */
  global_struct_variant1 = create_test_struct(1);
  global_struct_variant2 = create_test_struct(3);
  gt_ggc_mx(global_struct_variant1);
  gt_ggc_mx(global_struct_variant2);
  
  /* 6. UNION types */
  global_union_type = create_test_union(__FIELD_COUNT__);
  gt_ggc_mx(global_union_type);
  
  /* 7. CALLBACK types (function pointers) */
  global_callback_type = create_test_callback();
  gt_ggc_mx(global_callback_type);
  
  /* 8. USER STRUCT types */
  tree user_struct = create_user_struct();
  gt_ggc_mx(user_struct);
  
  /* 9. LANG STRUCT types */
  tree lang_struct = create_lang_struct();
  gt_ggc_mx(lang_struct);
  
  /* Process complex nested types */
  tree nested_struct = create_test_struct(2);
  tree pointer_to_struct = build_pointer_type(nested_struct);
  tree array_of_pointers = create_test_array(pointer_to_struct, 1);
  gt_ggc_mx(array_of_pointers);
  
  /* Process a struct containing function pointers */
  tree callback_struct = make_node(RECORD_TYPE);
  tree callback_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("callback"),
                                  global_callback_type);
  TYPE_FIELDS(callback_struct) = callback_field;
  gt_ggc_mx(callback_struct);
  
  /* Force processing of type lists */
  ggc_mark_tree(global_scalar_type);
  ggc_mark_tree(global_pointer_type);
  ggc_mark_tree(global_array_type);
  ggc_mark_tree(global_struct_type);
  ggc_mark_tree(global_union_type);
  ggc_mark_tree(global_callback_type);
  ggc_mark_tree(global_string_type);
}

/* Helper to create struct with variable field count */
static tree create_test_struct(int field_count)
{
  tree type = make_node(RECORD_TYPE);
  tree first_field = NULL_TREE;
  tree prev_field = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field;
    char field_name[32];
    
    snprintf(field_name, sizeof(field_name), "field%d", i);
    
    /* Alternate field types for variety */
    if (i % 3 == 0) {
      field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                        get_identifier(field_name),
                        integer_type_node);
    } else if (i % 3 == 1) {
      field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                        get_identifier(field_name),
                        char_type_node);
    } else {
      field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                        get_identifier(field_name),
                        build_pointer_type(integer_type_node));
    }
    
    if (first_field == NULL_TREE) {
      first_field = field;
    } else {
      DECL_CHAIN(prev_field) = field;
    }
    prev_field = field;
  }
  
  TYPE_FIELDS(type) = first_field;
  return type;
}

/* Helper to create union with variable field count */
static tree create_test_union(int field_count)
{
  tree type = make_node(UNION_TYPE);
  tree first_field = NULL_TREE;
  tree prev_field = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field;
    char field_name[32];
    
    snprintf(field_name, sizeof(field_name), "union_field%d", i);
    
    /* Use different types for union fields */
    switch (i % 4) {
      case 0:
        field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier(field_name),
                          integer_type_node);
        break;
      case 1:
        field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier(field_name),
                          char_type_node);
        break;
      case 2:
        field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier(field_name),
                          build_pointer_type(char_type_node));
        break;
      case 3:
        field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier(field_name),
                          build_array_type(char_type_node, NULL_TREE));
        break;
    }
    
    if (first_field == NULL_TREE) {
      first_field = field;
    } else {
      DECL_CHAIN(prev_field) = field;
    }
    prev_field = field;
  }
  
  TYPE_FIELDS(type) = first_field;
  return type;
}

/* Helper to create multi-dimensional array */
static tree create_test_array(tree element_type, int dimensions)
{
  tree type = element_type;
  
  for (int i = 0; i < dimensions; i++) {
    /* Create array type with unspecified bounds */
    type = build_array_type(type, NULL_TREE);
  }
  
  return type;
}

/* Helper to create callback (function pointer) type */
static tree create_test_callback(void)
{
  /* Create a simple function type: int(void) */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  
  /* Create pointer to function type */
  return build_pointer_type(func_type);
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

/* Alternative: Plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
