/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to ensure all type_enum
   categories are processed by the gengtype system. */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree __SCALAR_TYPE__ = integer_type_node;
static GTY(()) tree __ANOTHER_SCALAR__ = boolean_type_node;
static GTY(()) tree __CHAR_TYPE__ = char_type_node;

/* Pointer types */
static GTY(()) tree __POINTER_TO_INT__;
static GTY(()) tree __POINTER_TO_CHAR__;

/* Array types */
static GTY(()) tree __INT_ARRAY__;
static GTY(()) tree __MULTIDIM_ARRAY__;

/* Struct types */
static GTY(()) tree __TEST_STRUCT__;
static GTY(()) tree __USER_STRUCT__;

/* Union type */
static GTY(()) tree __TEST_UNION__;

/* String type (pointer to char) */
static GTY(()) tree __STRING_TYPE__;

/* Callback/function pointer type */
static GTY(()) tree __CALLBACK_TYPE__;

/* Lang struct type */
static GTY(()) tree __LANG_STRUCT__;

/* Helper function to create a struct with variable field count */
static tree
create_struct_with_fields(int field_count, const char *name, bool is_union)
{
  tree struct_type = make_node(is_union ? UNION_TYPE : RECORD_TYPE);
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
    
    if (field_list == NULL_TREE)
      field_list = field;
    else
      DECL_CHAIN(field_list) = field;
  }
  
  TYPE_FIELDS(struct_type) = field_list;
  layout_type(struct_type);
  
  return struct_type;
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* 1. Scalar types - already covered by global declarations
     but let's process them explicitly */
  gt_ggc_mx(integer_type_node);
  gt_ggc_mx(boolean_type_node);
  gt_ggc_mx(char_type_node);
  
  /* 2. Pointer types */
  __POINTER_TO_INT__ = build_pointer_type(integer_type_node);
  __POINTER_TO_CHAR__ = build_pointer_type(char_type_node);
  
  /* Force processing of pointer types */
  gt_ggc_mx(__POINTER_TO_INT__);
  gt_ggc_mx(__POINTER_TO_CHAR__);
  
  /* 3. Array types */
  tree index_type = build_index_type(size_int(__FIELD_COUNT__));
  __INT_ARRAY__ = build_array_type(integer_type_node, index_type);
  
  /* Multi-dimensional array */
  tree range_type = build_range_type(sizetype, size_zero_node, size_int(5));
  tree array_type_1d = build_array_type(integer_type_node, range_type);
  tree array_type_2d = build_array_type(array_type_1d, range_type);
  __MULTIDIM_ARRAY__ = array_type_2d;
  
  gt_ggc_mx(__INT_ARRAY__);
  gt_ggc_mx(__MULTIDIM_ARRAY__);
  
  /* 4. Struct types */
  __TEST_STRUCT__ = create_struct_with_fields(__FIELD_COUNT__, "TestStruct", false);
  gt_ggc_mx(__TEST_STRUCT__);
  
  /* 5. User struct - mark with TYPE_LANG_FLAG or similar */
  __USER_STRUCT__ = create_struct_with_fields(2, "UserStruct", false);
#ifdef TYPE_LANG_FLAG_1
  TYPE_LANG_FLAG_1(__USER_STRUCT__) = 1;
#endif
  gt_ggc_mx(__USER_STRUCT__);
  
  /* 6. Union type */
  __TEST_UNION__ = create_struct_with_fields(3, "TestUnion", true);
  gt_ggc_mx(__TEST_UNION__);
  
  /* 7. String type - TYPE_STRING is ptr_type_node to char */
  __STRING_TYPE__ = build_pointer_type(char_type_node);
  /* Mark it specially if needed - in practice TYPE_STRING detection
     happens in gengtype based on type structure */
  gt_ggc_mx(__STRING_TYPE__);
  
  /* 8. Callback/function pointer type */
  tree func_type = build_function_type_list(void_type_node, 
                                           integer_type_node, 
                                           NULL_TREE);
  __CALLBACK_TYPE__ = build_pointer_type(func_type);
  gt_ggc_mx(__CALLBACK_TYPE__);
  
  /* 9. Lang struct - use TYPE_LANG_SPECIFIC */
  __LANG_STRUCT__ = create_struct_with_fields(1, "LangStruct", false);
  
  /* Create and attach lang-specific data */
  struct lang_type *lang_data;
  lang_data = (struct lang_type *)ggc_alloc_cleared(sizeof(struct lang_type));
  SET_TYPE_LANG_SPECIFIC(__LANG_STRUCT__, lang_data);
  
  gt_ggc_mx(__LANG_STRUCT__);
  
  /* 10. Process undefined type if possible */
  tree undefined_type = make_node(ERROR_MARK);
  if (TREE_CODE(undefined_type) == ERROR_MARK) {
    /* This should trigger TYPE_UNDEFINED case */
    gt_ggc_mx(undefined_type);
  }
  
  /* Additional processing to ensure coverage */
  /* Process a NULL pointer type */
  gt_ggc_mx(ptr_type_node);
  
  /* Process void type */
  gt_ggc_mx(void_type_node);
  
  /* Process various tree nodes that might have different type categorizations */
  tree const_string = build_string(10, "teststring");
  gt_ggc_mx(const_string);
  
  /* Create and process a complex type */
  tree complex_type = build_complex_type(integer_type_node);
  gt_ggc_mx(complex_type);
  
  /* Vector type */
  tree vector_type = build_vector_type(integer_type_node, 4);
  gt_ggc_mx(vector_type);
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC's internal structures if needed */
  test_gengtype_categorization();
  return 0;
}
#endif

/* Alternative: Register as a plugin if compiling as GCC plugin */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Compilation test without main for in-tree compilation */
#ifndef STANDALONE_TEST
#ifndef PLUGIN_TEST
/* This function will be called from GCC's initialization */
void __attribute__((constructor))
init_gengtype_test(void)
{
  test_gengtype_categorization();
}
#endif
#endif
