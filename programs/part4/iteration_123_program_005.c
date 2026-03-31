/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* This test creates various GCC type nodes to ensure all type_enum categories are exercised */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

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

/* Array of types to ensure all categories are covered */
static GTY(()) tree type_collection[10];

/* Main test function */
void
test_gengtype_categorization (void)
{
  int i = 0;
  
  /* TYPE_SCALAR - Basic scalar types */
  global_scalar_type = integer_type_node;  /* int */
  type_collection[i++] = global_scalar_type;
  
  type_collection[i++] = boolean_type_node;    /* bool */
  type_collection[i++] = char_type_node;       /* char */
  type_collection[i++] = short_integer_type_node;  /* short */
  type_collection[i++] = long_integer_type_node;   /* long */
  
  /* TYPE_POINTER - Pointer types */
  global_pointer_type = build_pointer_type (integer_type_node);
  type_collection[i++] = global_pointer_type;
  
  /* TYPE_STRING - String type (pointer to char) */
  global_string_type = build_pointer_type (char_type_node);
  type_collection[i++] = global_string_type;
  
  /* TYPE_ARRAY - Array types with different dimensions */
  tree int_array_1d = create_test_array (integer_type_node, 1);
  tree int_array_2d = create_test_array (integer_type_node, 2);
  tree char_array = create_test_array (char_type_node, 1);
  
  global_array_type = int_array_1d;
  type_collection[i++] = global_array_type;
  type_collection[i++] = int_array_2d;
  type_collection[i++] = char_array;
  
  /* TYPE_STRUCT - Structure types with varying field counts */
  tree struct_2_fields = create_test_struct (2);
  tree struct_5_fields = create_test_struct (5);
  
  global_struct_type = struct_2_fields;
  type_collection[i++] = global_struct_type;
  type_collection[i++] = struct_5_fields;
  
  /* TYPE_UNION - Union types */
  tree union_3_fields = create_test_union (3);
  
  global_union_type = union_3_fields;
  type_collection[i++] = global_union_type;
  
  /* TYPE_CALLBACK - Function pointer types */
  tree func_ptr = create_test_callback ();
  
  global_callback_type = func_ptr;
  type_collection[i++] = global_callback_type;
  
  /* TYPE_USER_STRUCT and TYPE_LANG_STRUCT - Language-specific types */
  /* Create a struct and mark it as language-specific */
  tree lang_struct = create_test_struct (3);
  
#ifdef TYPE_LANG_FLAG_0
  TYPE_LANG_FLAG_0 (lang_struct) = 1;
#endif
  
  /* Try to set language-specific info if available */
#ifdef SET_TYPE_LANG_SPECIFIC
  {
    struct lang_type *lt = ggc_alloc<struct lang_type> ();
    SET_TYPE_LANG_SPECIFIC (lang_struct, lt);
  }
#endif
  
  type_collection[i++] = lang_struct;
  
  /* Process all types through gengtype machinery */
  /* Force GC marking of all types to ensure they're processed */
  for (int j = 0; j < i; j++)
    {
      /* Use GTY macros to force gengtype processing */
      gt_ggc_mx (type_collection[j]);
      
      /* Alternative: Use ggc_test_and_mark if available */
#ifdef ggc_test_and_mark
      ggc_test_and_mark (type_collection[j]);
#endif
    }
  
  /* Create some complex nested types to test edge cases */
  tree complex_type = create_test_struct (__FIELD_COUNT__);
  tree pointer_to_struct = build_pointer_type (complex_type);
  tree array_of_pointers = create_test_array (pointer_to_struct, 1);
  
  /* Process these as well */
  gt_ggc_mx (complex_type);
  gt_ggc_mx (pointer_to_struct);
  gt_ggc_mx (array_of_pointers);
  
  /* Create a union with struct members */
  tree nested_union = make_node (UNION_TYPE);
  tree field1 = build_decl (UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier ("as_struct"),
                           struct_2_fields);
  tree field2 = build_decl (UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier ("as_int"),
                           integer_type_node);
  DECL_CHAIN (field1) = field2;
  TYPE_FIELDS (nested_union) = field1;
  layout_type (nested_union);
  
  gt_ggc_mx (nested_union);
}

/* Helper function to create test struct with specified number of fields */
static tree
create_test_struct (int field_count)
{
  tree struct_type = make_node (RECORD_TYPE);
  tree last_field = NULL_TREE;
  
  for (int i = 0; i < field_count; i++)
    {
      char field_name[32];
      sprintf (field_name, "field_%d", i);
      
      tree field_type;
      /* Vary field types for more comprehensive testing */
      switch (i % 4)
        {
        case 0:
          field_type = integer_type_node;
          break;
        case 1:
          field_type = build_pointer_type (char_type_node);
          break;
        case 2:
          field_type = boolean_type_node;
          break;
        case 3:
          field_type = create_test_array (integer_type_node, 1);
          break;
        default:
          field_type = integer_type_node;
        }
      
      tree field = build_decl (UNKNOWN_LOCATION, FIELD_DECL,
                              get_identifier (field_name),
                              field_type);
      
      if (last_field == NULL_TREE)
        TYPE_FIELDS (struct_type) = field;
      else
        DECL_CHAIN (last_field) = field;
      
      last_field = field;
    }
  
  /* Complete the struct definition */
  layout_type (struct_type);
  return struct_type;
}

/* Helper function to create test union with specified number of fields */
static tree
create_test_union (int field_count)
{
  tree union_type = make_node (UNION_TYPE);
  tree last_field = NULL_TREE;
  
  for (int i = 0; i < field_count; i++)
    {
      char field_name[32];
      sprintf (field_name, "u_field_%d", i);
      
      tree field_type;
      switch (i % 3)
        {
        case 0:
          field_type = integer_type_node;
          break;
        case 1:
          field_type = long_integer_type_node;
          break;
        case 2:
          field_type = build_pointer_type (void_type_node);
          break;
        default:
          field_type = integer_type_node;
        }
      
      tree field = build_decl (UNKNOWN_LOCATION, FIELD_DECL,
                              get_identifier (field_name),
                              field_type);
      
      if (last_field == NULL_TREE)
        TYPE_FIELDS (union_type) = field;
      else
        DECL_CHAIN (last_field) = field;
      
      last_field = field;
    }
  
  layout_type (union_type);
  return union_type;
}

/* Helper function to create array type */
static tree
create_test_array (tree element_type, int dimensions)
{
  tree array_type = element_type;
  
  for (int i = 0; i < dimensions; i++)
    {
      /* Create array with 10 elements for each dimension */
      tree index_type = build_index_type (size_int (9));
      array_type = build_array_type (element_type, index_type);
      element_type = array_type; /* For multi-dimensional arrays */
    }
  
  return array_type;
}

/* Helper function to create callback (function pointer) type */
static tree
create_test_callback (void)
{
  /* Create a function type: int (*)(int, char *) */
  tree arg_types = NULL_TREE;
  tree last_arg = NULL_TREE;
  
  /* First argument: int */
  tree arg1 = integer_type_node;
  arg_types = tree_cons (NULL_TREE, arg1, NULL_TREE);
  last_arg = arg_types;
  
  /* Second argument: char * */
  tree arg2 = build_pointer_type (char_type_node);
  arg_types = tree_cons (NULL_TREE, arg2, arg_types);
  
  /* Build function type */
  tree func_type = build_function_type (integer_type_node, arg_types);
  
  /* Create pointer to function */
  return build_pointer_type (func_type);
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int
main (void)
{
  /* Initialize GCC internal structures if needed */
#ifdef GGC_INITIALIZE
  GGC_INITIALIZE ();
#endif
  
  test_gengtype_categorization ();
  
  /* Print some diagnostics */
  printf ("Gengtype categorization test completed.\n");
  
  return 0;
}
#endif

/* Plugin entry point for GCC plugin testing */
#ifdef PLUGIN_TEST
int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  test_gengtype_categorization ();
  return 0;
}
#endif
