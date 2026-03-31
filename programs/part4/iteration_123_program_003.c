/* test_gengtype_categorization.c - Comprehensive test for gengtype type classification */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Guard for version-specific features */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 4
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 9
#endif

/* Global variables with GTY annotations to force gengtype processing */
/* These will be processed during gengtype's type analysis */

/* Scalar types */
static GTY(()) tree scalar_var = NULL_TREE;

/* Pointer types */
static GTY(()) tree pointer_var = NULL_TREE;

/* Array types */
static GTY(()) tree array_var = NULL_TREE;

/* Struct types */
static GTY(()) tree struct_var = NULL_TREE;

/* Union types */
static GTY(()) tree union_var = NULL_TREE;

/* String type (char pointer) */
static GTY(()) tree string_var = NULL_TREE;

/* Callback type (function pointer) */
static GTY(()) tree callback_var = NULL_TREE;

/* User struct type */
static GTY(()) tree user_struct_var = NULL_TREE;

/* Lang struct type */
static GTY(()) tree lang_struct_var = NULL_TREE;

/* Helper function to create a struct with variable field count */
static tree
create_struct_with_fields(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++)
  {
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
create_union_with_fields(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++)
  {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("field"),
                           integer_type_node);
    DECL_CONTEXT(field) = union_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  
  return union_type;
}

/* Main test function */
void
test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES - TYPE_SCALAR */
  scalar_var = integer_type_node;      /* int */
  gt_ggc_mx(scalar_var);
  
  scalar_var = char_type_node;         /* char */
  gt_ggc_mx(scalar_var);
  
  scalar_var = boolean_type_node;      /* bool */
  gt_ggc_mx(scalar_var);
  
  /* 2. POINTER TYPES - TYPE_POINTER */
  pointer_var = build_pointer_type(integer_type_node);
  gt_ggc_mx(pointer_var);
  
  /* 3. ARRAY TYPES - TYPE_ARRAY */
  /* Create array of integers with __FIELD_COUNT__ elements */
  array_var = build_array_type_nelts(integer_type_node, __FIELD_COUNT__);
  gt_ggc_mx(array_var);
  
  /* Multi-dimensional array */
  tree inner_array = build_array_type_nelts(integer_type_node, 5);
  tree outer_array = build_array_type_nelts(inner_array, 3);
  gt_ggc_mx(outer_array);
  
  /* 4. STRUCT TYPES - TYPE_STRUCT */
  /* Create struct with __FIELD_COUNT__ fields */
  struct_var = create_struct_with_fields(__FIELD_COUNT__);
  gt_ggc_mx(struct_var);
  
  /* Another struct with different field types */
  tree complex_struct = make_node(RECORD_TYPE);
  tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier("int_field"),
                          integer_type_node);
  tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                          get_identifier("ptr_field"),
                          build_pointer_type(char_type_node));
  DECL_CONTEXT(field1) = complex_struct;
  DECL_CONTEXT(field2) = complex_struct;
  TYPE_FIELDS(complex_struct) = chainon(field1, field2);
  layout_type(complex_struct);
  gt_ggc_mx(complex_struct);
  
  /* 5. UNION TYPES - TYPE_UNION */
  /* Create union with __FIELD_COUNT__ fields */
  union_var = create_union_with_fields(__FIELD_COUNT__);
  gt_ggc_mx(union_var);
  
  /* 6. STRING TYPE - TYPE_STRING */
  /* char* is treated as TYPE_STRING in gengtype */
  string_var = build_pointer_type(char_type_node);
  gt_ggc_mx(string_var);
  
  /* 7. CALLBACK TYPE - TYPE_CALLBACK */
  /* Function pointer type */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  callback_var = build_pointer_type(func_type);
  gt_ggc_mx(callback_var);
  
  /* More complex function pointer */
  tree arg_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
  arg_list = tree_cons(NULL_TREE, char_type_node, arg_list);
  tree complex_func_type = build_function_type(void_type_node, arg_list);
  tree complex_callback = build_pointer_type(complex_func_type);
  gt_ggc_mx(complex_callback);
  
  /* 8. USER STRUCT TYPE - TYPE_USER_STRUCT */
  /* Create a struct and mark it as user-defined */
  user_struct_var = create_struct_with_fields(2);
  /* Mark as user struct - this depends on gengtype's classification */
  /* In practice, this might require special GTY options or attributes */
  gt_ggc_mx(user_struct_var);
  
  /* 9. LANG STRUCT TYPE - TYPE_LANG_STRUCT */
  /* Create a struct with language-specific information */
  lang_struct_var = create_struct_with_fields(2);
  
  /* Try to mark as language-specific struct */
  /* This typically requires TYPE_LANG_SPECIFIC to be set */
#ifdef SET_TYPE_LANG_SPECIFIC
  SET_TYPE_LANG_SPECIFIC(lang_struct_var, (struct lang_type *)1);
#endif
  gt_ggc_mx(lang_struct_var);
  
  /* Process all types through gengtype machinery */
  /* Force categorization by using gt_pch_nx */
  gt_pch_nx(&scalar_var);
  gt_pch_nx(&pointer_var);
  gt_pch_nx(&array_var);
  gt_pch_nx(&struct_var);
  gt_pch_nx(&union_var);
  gt_pch_nx(&string_var);
  gt_pch_nx(&callback_var);
  gt_pch_nx(&user_struct_var);
  gt_pch_nx(&lang_struct_var);
  
  /* Additional type variations for comprehensive coverage */
  
  /* Void pointer */
  tree void_ptr = build_pointer_type(void_type_node);
  gt_ggc_mx(void_ptr);
  
  /* Const pointer */
  tree const_ptr = build_pointer_type(build_qualified_type(integer_type_node, TYPE_QUAL_CONST));
  gt_ggc_mx(const_ptr);
  
  /* Array of pointers */
  tree ptr_array = build_array_type_nelts(build_pointer_type(integer_type_node), 10);
  gt_ggc_mx(ptr_array);
  
  /* Struct containing array */
  tree struct_with_array = make_node(RECORD_TYPE);
  tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("data"),
                               build_array_type_nelts(char_type_node, 100));
  DECL_CONTEXT(array_field) = struct_with_array;
  TYPE_FIELDS(struct_with_array) = array_field;
  layout_type(struct_with_array);
  gt_ggc_mx(struct_with_array);
  
  /* Union with different field types */
  tree mixed_union = make_node(UNION_TYPE);
  tree ufield1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("as_int"),
                           integer_type_node);
  tree ufield2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("as_ptr"),
                           build_pointer_type(void_type_node));
  DECL_CONTEXT(ufield1) = mixed_union;
  DECL_CONTEXT(ufield2) = mixed_union;
  TYPE_FIELDS(mixed_union) = chainon(ufield1, ufield2);
  layout_type(mixed_union);
  gt_ggc_mx(mixed_union);
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Plugin entry point for GCC plugin compilation */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
