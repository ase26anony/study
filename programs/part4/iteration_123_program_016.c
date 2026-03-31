/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

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

/* Arrays of types for variability */
static GTY(()) tree scalar_types[__SCALAR_COUNT__];
static GTY(()) tree struct_types[__STRUCT_COUNT__];
static GTY(()) tree array_types[__ARRAY_COUNT__];

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  int i;
  
  /* 1. SCALAR TYPES - triggers TYPE_SCALAR case */
  scalar_types[0] = integer_type_node;
  scalar_types[1] = char_type_node;
  scalar_types[2] = boolean_type_node;
  scalar_types[3] = size_type_node;
  scalar_types[4] = ptrdiff_type_node;
  
  /* Process each scalar type */
  for (i = 0; i < __SCALAR_COUNT__ && i < 5; i++) {
    if (scalar_types[i]) {
      gt_ggc_mx(scalar_types[i]);
    }
  }
  
  /* 2. POINTER TYPES - triggers TYPE_POINTER case */
  global_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx(global_pointer_type);
  
  /* Additional pointer variations */
  tree ptr_to_char = build_pointer_type(char_type_node);
  tree ptr_to_ptr = build_pointer_type(global_pointer_type);
  gt_ggc_mx(ptr_to_char);
  gt_ggc_mx(ptr_to_ptr);
  
  /* 3. STRING TYPE - triggers TYPE_STRING case */
  /* In GCC, string type is typically pointer to char */
  global_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(global_string_type);
  
  /* 4. ARRAY TYPES - triggers TYPE_ARRAY case */
  /* Create arrays with different dimensions and element types */
  array_types[0] = build_array_type(integer_type_node, NULL_TREE);
  array_types[1] = build_array_type_nelts(char_type_node, 10);
  array_types[2] = create_test_array(integer_type_node, 2);
  
  for (i = 0; i < __ARRAY_COUNT__ && i < 3; i++) {
    if (array_types[i]) {
      gt_ggc_mx(array_types[i]);
    }
  }
  
  /* 5. STRUCT TYPES - triggers TYPE_STRUCT case */
  struct_types[0] = create_test_struct(__FIELD_COUNT__);
  struct_types[1] = create_test_struct(2);  /* Smaller struct */
  
  for (i = 0; i < __STRUCT_COUNT__ && i < 2; i++) {
    if (struct_types[i]) {
      gt_ggc_mx(struct_types[i]);
    }
  }
  
  /* 6. UNION TYPES - triggers TYPE_UNION case */
  global_union_type = create_test_union(3);
  gt_ggc_mx(global_union_type);
  
  /* 7. CALLBACK TYPES - triggers TYPE_CALLBACK case */
  global_callback_type = create_test_callback();
  gt_ggc_mx(global_callback_type);
  
  /* 8. USER STRUCT - triggers TYPE_USER_STRUCT case */
  /* Create a struct and mark it as user-defined */
  tree user_struct = create_test_struct(2);
#ifdef TYPE_LANG_FLAG_5
  TYPE_LANG_FLAG_5(user_struct) = 1;
#endif
  gt_ggc_mx(user_struct);
  
  /* 9. LANG STRUCT - triggers TYPE_LANG_STRUCT case */
  /* Create a language-specific struct */
  tree lang_struct = create_test_struct(2);
#ifdef TYPE_LANG_SPECIFIC
  if (!TYPE_LANG_SPECIFIC(lang_struct)) {
    TYPE_LANG_SPECIFIC(lang_struct) = (struct lang_type *) ggc_alloc_cleared(sizeof(struct lang_type));
  }
#endif
  gt_ggc_mx(lang_struct);
  
  /* 10. Complex type combinations */
  /* Struct containing pointers */
  tree complex_struct = create_test_struct(3);
  gt_ggc_mx(complex_struct);
  
  /* Array of structs */
  tree array_of_structs = build_array_type(struct_types[0], NULL_TREE);
  gt_ggc_mx(array_of_structs);
  
  /* Pointer to function returning struct */
  tree func_type = build_function_type(struct_types[0], NULL_TREE);
  tree ptr_to_func = build_pointer_type(func_type);
  gt_ggc_mx(ptr_to_func);
  
  /* Ensure all types are processed by walking the type graph */
  gt_ggc_mx(&global_scalar_type);
  gt_ggc_mx(&global_pointer_type);
  gt_ggc_mx(&global_array_type);
  gt_ggc_mx(&global_struct_type);
  gt_ggc_mx(&global_union_type);
  gt_ggc_mx(&global_callback_type);
  gt_ggc_mx(&global_string_type);
}

/* Helper function to create a test struct with variable field count */
static tree
create_test_struct(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  tree last_field = NULL_TREE;
  int i;
  
  /* Push struct into current binding level */
  pushdecl(build_decl(UNKNOWN_LOCATION, TYPE_DECL, 
                      get_identifier("test_struct"), struct_type));
  
  /* Create fields based on field_count */
  for (i = 0; i < field_count && i < 5; i++) {
    char field_name[20];
    sprintf(field_name, "field%d", i);
    
    tree field_type;
    switch (i % 4) {
      case 0: field_type = integer_type_node; break;
      case 1: field_type = char_type_node; break;
      case 2: field_type = build_pointer_type(integer_type_node); break;
      case 3: field_type = build_array_type_nelts(char_type_node, 10); break;
      default: field_type = integer_type_node;
    }
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name), field_type);
    
    if (last_field) {
      DECL_CHAIN(last_field) = field;
    } else {
      field_list = field;
    }
    last_field = field;
  }
  
  TYPE_FIELDS(struct_type) = field_list;
  layout_type(struct_type);
  
  return struct_type;
}

/* Helper function to create a test union */
static tree
create_test_union(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  tree last_field = NULL_TREE;
  int i;
  
  pushdecl(build_decl(UNKNOWN_LOCATION, TYPE_DECL,
                      get_identifier("test_union"), union_type));
  
  for (i = 0; i < field_count && i < 5; i++) {
    char field_name[20];
    sprintf(field_name, "u_field%d", i);
    
    tree field_type;
    switch (i % 3) {
      case 0: field_type = integer_type_node; break;
      case 1: field_type = char_type_node; break;
      case 2: field_type = build_pointer_type(char_type_node); break;
      default: field_type = integer_type_node;
    }
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name), field_type);
    
    if (last_field) {
      DECL_CHAIN(last_field) = field;
    } else {
      field_list = field;
    }
    last_field = field;
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  
  return union_type;
}

/* Helper function to create multi-dimensional arrays */
static tree
create_test_array(tree element_type, int dimensions)
{
  tree array_type = element_type;
  int i;
  
  for (i = 0; i < dimensions && i < 3; i++) {
    tree index_type = build_index_type(size_int(5 * (i + 1)));
    array_type = build_array_type(array_type, index_type);
  }
  
  return array_type;
}

/* Helper function to create a callback (function pointer) type */
static tree
create_test_callback(void)
{
  /* Create a function type taking an int and returning void */
  tree arg_type = integer_type_node;
  tree arg_list = build_tree_list(NULL_TREE, arg_type);
  tree func_type = build_function_type(void_type_node, arg_list);
  
  /* Create pointer to function */
  return build_pointer_type(func_type);
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC internal structures */
  gcc_init();
  
  /* Run the categorization test */
  test_gengtype_categorization();
  
  return 0;
}
#endif

/* Alternative: Plugin initialization */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
