/* test_gengtype_categorization.c - Comprehensive test for gengtype type categorization */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test_gengtype_categorization.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "stringpool.h"
#include "attribs.h"

/* Forward declarations for helper functions */
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
static GTY(()) tree global_string_type = NULL_TREE;
static GTY(()) tree global_callback_type = NULL_TREE;
static GTY(()) tree global_user_struct_type = NULL_TREE;
static GTY(()) tree global_lang_struct_type = NULL_TREE;

/* Variable struct with configurable field count */
static GTY(()) tree __FIELD_COUNT__struct = NULL_TREE;

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* TYPE_SCALAR - Basic scalar types */
  global_scalar_type = integer_type_node;      /* int */
  gt_ggc_mx (global_scalar_type);
  
  global_scalar_type = boolean_type_node;      /* bool */
  gt_ggc_mx (global_scalar_type);
  
  global_scalar_type = char_type_node;         /* char */
  gt_ggc_mx (global_scalar_type);
  
  global_scalar_type = size_type_node;         /* size_t */
  gt_ggc_mx (global_scalar_type);
  
  /* TYPE_POINTER */
  global_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx (global_pointer_type);
  
  /* Multiple pointer variations */
  tree double_ptr = build_pointer_type(global_pointer_type);
  gt_ggc_mx (double_ptr);
  
  /* TYPE_STRING - char* is treated as TYPE_STRING */
  global_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx (global_string_type);
  
  /* const char* */
  tree const_char_ptr = build_pointer_type(build_type_variant(char_type_node, 1, 0));
  gt_ggc_mx (const_char_ptr);
  
  /* TYPE_ARRAY */
  /* 1D array */
  global_array_type = build_array_type(integer_type_node, NULL_TREE);
  gt_ggc_mx (global_array_type);
  
  /* Fixed-size array */
  tree fixed_array = build_array_type_nelts(integer_type_node, 10);
  gt_ggc_mx (fixed_array);
  
  /* Multi-dimensional array */
  tree md_array = create_test_array(integer_type_node, __TYPE_KIND__);
  if (md_array)
    gt_ggc_mx (md_array);
  
  /* TYPE_STRUCT */
  global_struct_type = create_test_struct(3);  /* Struct with 3 fields */
  if (global_struct_type)
    gt_ggc_mx (global_struct_type);
  
  /* Variable field count struct */
  __FIELD_COUNT__struct = create_test_struct(__FIELD_COUNT__);
  if (__FIELD_COUNT__struct)
    gt_ggc_mx (__FIELD_COUNT__struct);
  
  /* Empty struct */
  tree empty_struct = make_node(RECORD_TYPE);
  TYPE_FIELDS(empty_struct) = NULL_TREE;
  gt_ggc_mx (empty_struct);
  
  /* TYPE_UNION */
  global_union_type = create_test_union(2);    /* Union with 2 fields */
  if (global_union_type)
    gt_ggc_mx (global_union_type);
  
  /* TYPE_CALLBACK - Function pointer types */
  global_callback_type = create_test_callback();
  if (global_callback_type)
    gt_ggc_mx (global_callback_type);
  
  /* Variadic function pointer */
  tree va_func_type = build_function_type_list(integer_type_node, 
                                               char_type_node, 
                                               NULL_TREE);
  tree va_func_ptr = build_pointer_type(va_func_type);
  gt_ggc_mx (va_func_ptr);
  
  /* TYPE_USER_STRUCT - Mark a struct as user-defined */
  global_user_struct_type = create_test_struct(2);
  if (global_user_struct_type)
  {
    /* Simulate user struct marking - actual mechanism may vary */
    TYPE_USER_ALIGN(global_user_struct_type) = 1;
    gt_ggc_mx (global_user_struct_type);
  }
  
  /* TYPE_LANG_STRUCT - Language-specific struct */
  global_lang_struct_type = create_test_struct(1);
  if (global_lang_struct_type)
  {
    /* Set language-specific flag if available */
    #ifdef TYPE_LANG_FLAG_0
    TYPE_LANG_FLAG_0(global_lang_struct_type) = 1;
    #endif
    
    /* Alternative: Use TYPE_LANG_SPECIFIC if defined */
    #ifdef TYPE_LANG_SPECIFIC
    if (!TYPE_LANG_SPECIFIC(global_lang_struct_type))
    {
      /* Allocate and set dummy lang-specific data */
      struct tree_type *type = (struct tree_type *)global_lang_struct_type;
      type->lang_specific = (struct lang_type *)xzalloc(sizeof(struct lang_type));
    }
    #endif
    
    gt_ggc_mx (global_lang_struct_type);
  }
  
  /* Complex type combinations */
  
  /* Struct containing pointers */
  tree struct_with_ptrs = make_node(RECORD_TYPE);
  tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL, 
                           get_identifier("data"), 
                           build_pointer_type(integer_type_node));
  tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("next"),
                           build_pointer_type(struct_with_ptrs));
  DECL_CHAIN(field1) = field2;
  TYPE_FIELDS(struct_with_ptrs) = field1;
  gt_ggc_mx (struct_with_ptrs);
  
  /* Array of structs */
  tree array_of_structs = build_array_type(global_struct_type, NULL_TREE);
  gt_ggc_mx (array_of_structs);
  
  /* Pointer to array */
  tree ptr_to_array = build_pointer_type(global_array_type);
  gt_ggc_mx (ptr_to_array);
  
  /* Union containing array */
  tree union_with_array = make_node(UNION_TYPE);
  tree ufield = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier("arr"),
                          fixed_array);
  TYPE_FIELDS(union_with_array) = ufield;
  gt_ggc_mx (union_with_array);
  
  /* Force processing of all created types through various mechanisms */
  
  /* Method 1: Direct gt_ggc_mx calls (already done above) */
  
  /* Method 2: Use in GTY-annotated local */
  {
    GTY(()) tree local_var = global_pointer_type;
    (void)local_var;  /* Suppress unused warning */
  }
  
  /* Method 3: Pass to a function that might trigger processing */
  debug_tree(global_struct_type);
  
  /* Ensure all counters would be incremented */
  asm volatile("" : : "r"(global_scalar_type), 
                       "r"(global_pointer_type),
                       "r"(global_array_type),
                       "r"(global_struct_type),
                       "r"(global_union_type),
                       "r"(global_string_type),
                       "r"(global_callback_type) : "memory");
}

/* Helper function to create a test struct with variable field count */
static tree
create_test_struct(int field_count)
{
  if (field_count <= 0)
    return NULL_TREE;
  
  tree struct_type = make_node(RECORD_TYPE);
  tree first_field = NULL_TREE;
  tree prev_field = NULL_TREE;
  
  for (int i = 0; i < field_count; i++)
  {
    tree field_type;
    
    /* Vary field types to create different structures */
    switch (i % 4)
    {
      case 0:
        field_type = integer_type_node;
        break;
      case 1:
        field_type = char_type_node;
        break;
      case 2:
        field_type = build_pointer_type(integer_type_node);
        break;
      case 3:
        field_type = build_array_type_nelts(char_type_node, 16);
        break;
      default:
        field_type = integer_type_node;
    }
    
    char field_name[32];
    snprintf(field_name, sizeof(field_name), "field%d", i);
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name),
                           field_type);
    
    if (!first_field)
      first_field = field;
    
    if (prev_field)
      DECL_CHAIN(prev_field) = field;
    
    prev_field = field;
  }
  
  TYPE_FIELDS(struct_type) = first_field;
  return struct_type;
}

/* Helper function to create a test union with variable field count */
static tree
create_test_union(int field_count)
{
  if (field_count <= 0)
    return NULL_TREE;
  
  tree union_type = make_node(UNION_TYPE);
  tree first_field = NULL_TREE;
  tree prev_field = NULL_TREE;
  
  for (int i = 0; i < field_count; i++)
  {
    tree field_type;
    
    switch (i % 3)
    {
      case 0:
        field_type = integer_type_node;
        break;
      case 1:
        field_type = char_type_node;
        break;
      case 2:
        field_type = build_pointer_type(void_type_node);
        break;
      default:
        field_type = integer_type_node;
    }
    
    char field_name[32];
    snprintf(field_name, sizeof(field_name), "ufield%d", i);
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name),
                           field_type);
    
    if (!first_field)
      first_field = field;
    
    if (prev_field)
      DECL_CHAIN(prev_field) = field;
    
    prev_field = field;
  }
  
  TYPE_FIELDS(union_type) = first_field;
  return union_type;
}

/* Helper function to create multi-dimensional arrays */
static tree
create_test_array(tree element_type, int dimensions)
{
  if (dimensions <= 0 || !element_type)
    return NULL_TREE;
  
  tree array_type = element_type;
  
  for (int i = 0; i < dimensions; i++)
  {
    /* Create dimension index type */
    tree index_type = build_index_type(build_int_cst(integer_type_node, 5 * (i + 1)));
    array_type = build_array_type(array_type, index_type);
  }
  
  return array_type;
}

/* Helper function to create callback (function pointer) types */
static tree
create_test_callback(void)
{
  /* Simple function: int(void) */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  tree func_ptr = build_pointer_type(func_type);
  
  /* Function with parameters: int(int, char*) */
  tree param_list = NULL_TREE;
  tree param_node;
  
  param_node = build_tree_list(NULL_TREE, integer_type_node);
  param_list = param_node;
  
  tree char_ptr_type = build_pointer_type(char_type_node);
  param_node = build_tree_list(param_node, char_ptr_type);
  
  tree func_type2 = build_function_type(integer_type_node, param_list);
  tree func_ptr2 = build_pointer_type(func_type2);
  
  /* Process both to ensure coverage */
  gt_ggc_mx (func_ptr2);
  
  return func_ptr;
}

/* Optional: Main function for standalone testing */
#ifdef TEST_STANDALONE
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif
