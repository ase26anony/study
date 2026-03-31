/* Test for gengtype type categorization coverage */
/* This test creates various GCC type nodes to trigger all type_enum cases */

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

/* Test function that creates and processes all type categories */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* TYPE_SCALAR: Basic scalar types */
  global_scalar_type = integer_type_node;
  gt_ggc_mx(global_scalar_type);
  
  /* Also test other scalar types */
  gt_ggc_mx(char_type_node);
  gt_ggc_mx(boolean_type_node);
  gt_ggc_mx(size_type_node);
  
  /* TYPE_POINTER: Pointer types */
  global_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx(global_pointer_type);
  
  /* TYPE_STRING: String type (pointer to char) */
  global_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(global_string_type);
  
  /* TYPE_ARRAY: Array types with different dimensions */
  tree array_1d = create_test_array(integer_type_node, 1);
  gt_ggc_mx(array_1d);
  
  tree array_2d = create_test_array(integer_type_node, 2);
  gt_ggc_mx(array_2d);
  
  global_array_type = create_test_array(char_type_node, __FIELD_COUNT__);
  gt_ggc_mx(global_array_type);
  
  /* TYPE_STRUCT: Structure types */
  global_struct_type = create_test_struct(__FIELD_COUNT__);
  gt_ggc_mx(global_struct_type);
  
  /* Create structs with different field counts for variability */
  tree small_struct = create_test_struct(1);
  gt_ggc_mx(small_struct);
  
  tree large_struct = create_test_struct(10);
  gt_ggc_mx(large_struct);
  
  /* TYPE_UNION: Union types */
  global_union_type = create_test_union(__FIELD_COUNT__);
  gt_ggc_mx(global_union_type);
  
  /* TYPE_CALLBACK: Function pointer types */
  global_callback_type = create_test_callback();
  gt_ggc_mx(global_callback_type);
  
  /* TYPE_USER_STRUCT: User-defined struct with lang-specific flag */
  tree user_struct = create_test_struct(3);
#ifdef TYPE_LANG_FLAG_0
  TYPE_LANG_FLAG_0(user_struct) = 1;
#endif
  /* Alternative method for user struct marking */
  SET_TYPE_ALIGN(user_struct, TYPE_ALIGN(user_struct) | 1);
  gt_ggc_mx(user_struct);
  
  /* TYPE_LANG_STRUCT: Language-specific struct */
  tree lang_struct = create_test_struct(2);
#ifdef TYPE_LANG_SPECIFIC
  if (TYPE_LANG_SPECIFIC(lang_struct) == NULL) {
    /* Allocate lang-specific data if available */
    TYPE_LANG_SPECIFIC(lang_struct) = (struct lang_type *) 
      ggc_alloc_cleared(sizeof(struct lang_type));
  }
#endif
  gt_ggc_mx(lang_struct);
  
  /* Complex type combinations */
  tree struct_with_pointers = create_test_struct(2);
  tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL, 
                          get_identifier("ptr_field"),
                          build_pointer_type(integer_type_node));
  tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier("array_field"),
                          create_test_array(char_type_node, 1));
  TYPE_FIELDS(struct_with_pointers) = chainon(field1, field2);
  gt_ggc_mx(struct_with_pointers);
  
  /* Nested structures */
  tree inner_struct = create_test_struct(2);
  tree outer_struct = make_node(RECORD_TYPE);
  tree nested_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("inner"),
                                inner_struct);
  TYPE_FIELDS(outer_struct) = nested_field;
  gt_ggc_mx(outer_struct);
  gt_ggc_mx(inner_struct);
  
  /* Void pointer type */
  tree void_ptr_type = build_pointer_type(void_type_node);
  gt_ggc_mx(void_ptr_type);
  
  /* Const-qualified types */
  tree const_int_type = build_qualified_type(integer_type_node, TYPE_QUAL_CONST);
  gt_ggc_mx(const_int_type);
  
  /* Volatile-qualified types */
  tree volatile_int_type = build_qualified_type(integer_type_node, TYPE_QUAL_VOLATILE);
  gt_ggc_mx(volatile_int_type);
  
  /* Force processing of type size computations */
  tree size = size_in_bytes(global_struct_type);
  gt_ggc_mx(size);
}

/* Helper function to create a test structure with specified number of fields */
static tree
create_test_struct(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    char field_name[32];
    snprintf(field_name, sizeof(field_name), "field_%d", i);
    
    tree field_type;
    switch (i % 4) {
      case 0: field_type = integer_type_node; break;
      case 1: field_type = char_type_node; break;
      case 2: field_type = build_pointer_type(integer_type_node); break;
      case 3: field_type = boolean_type_node; break;
      default: field_type = integer_type_node; break;
    }
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name),
                           field_type);
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(struct_type) = field_list;
  return struct_type;
}

/* Helper function to create a test union with specified number of fields */
static tree
create_test_union(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    char field_name[32];
    snprintf(field_name, sizeof(field_name), "union_field_%d", i);
    
    tree field_type;
    switch (i % 3) {
      case 0: field_type = integer_type_node; break;
      case 1: field_type = char_type_node; break;
      case 2: field_type = build_pointer_type(void_type_node); break;
      default: field_type = integer_type_node; break;
    }
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name),
                           field_type);
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  return union_type;
}

/* Helper function to create a test array type */
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

/* Helper function to create a callback (function pointer) type */
static tree
create_test_callback(void)
{
  /* Create a function type taking an int and returning an int */
  tree arg_type = integer_type_node;
  tree return_type = integer_type_node;
  
  tree function_type = build_function_type(return_type, 
                                          tree_cons(NULL_TREE, arg_type, NULL_TREE));
  
  /* Create pointer to function type */
  return build_pointer_type(function_type);
}

/* Main test entry point */
int
main(void)
{
  /* Initialize GCC's internal structures if needed */
#ifdef GCC_INITIALIZED
  test_gengtype_categorization();
#endif
  
  return 0;
}

/* Alternative: Plugin initialization if compiled as GCC plugin */
#ifdef PLUGIN_INIT_FUNCTION
int
plugin_init(struct plugin_name_args *plugin_info,
            struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
