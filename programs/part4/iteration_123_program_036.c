/* test_gengtype_categorization.c - Comprehensive test for gengtype type categorization */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
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

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree global_scalar_type;
static GTY(()) tree global_pointer_type;
static GTY(()) tree global_array_type;
static GTY(()) tree global_struct_type;
static GTY(()) tree global_union_type;
static GTY(()) tree global_string_type;
static GTY(()) tree global_callback_type;
static GTY(()) tree global_user_struct_type;
static GTY(()) tree global_lang_struct_type;

/* Array of types to ensure all are processed */
static GTY(()) tree all_types[10];

/* Helper to create a struct with variable field count */
static tree
create_test_struct(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier_with_length("field", 5),
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
create_test_union(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier_with_length("field", 5),
                           (i % 2 == 0) ? integer_type_node : char_type_node);
    DECL_CONTEXT(field) = union_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  return union_type;
}

/* Helper to create multi-dimensional array */
static tree
create_test_array(tree element_type, int dimensions)
{
  tree array_type = element_type;
  
  for (int i = 0; i < dimensions; i++) {
    tree index_type = build_index_type(size_int(10 + i));
    array_type = build_array_type(array_type, index_type);
  }
  
  return array_type;
}

/* Helper to create callback (function pointer) type */
static tree
create_test_callback(void)
{
  tree return_type = integer_type_node;
  tree arg_types = NULL_TREE;
  
  /* Create a function type with some arguments */
  for (int i = 0; i < 2; i++) {
    arg_types = chainon(arg_types,
                       tree_cons(NULL_TREE, integer_type_node, NULL_TREE));
  }
  
  tree func_type = build_function_type(return_type, arg_types);
  return build_pointer_type(func_type);
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  int type_index = 0;
  
  /* 1. SCALAR types */
  global_scalar_type = integer_type_node;
  all_types[type_index++] = integer_type_node;
  all_types[type_index++] = char_type_node;
  all_types[type_index++] = boolean_type_node;
  all_types[type_index++] = size_type_node;
  
  /* 2. POINTER types */
  global_pointer_type = build_pointer_type(integer_type_node);
  all_types[type_index++] = global_pointer_type;
  all_types[type_index++] = build_pointer_type(char_type_node);
  
  /* 3. ARRAY types - varying dimensions */
  global_array_type = create_test_array(integer_type_node, __ARRAY_DIMENSIONS__);
  all_types[type_index++] = global_array_type;
  all_types[type_index++] = create_test_array(char_type_node, 1);
  all_types[type_index++] = create_test_array(global_pointer_type, 2);
  
  /* 4. STRUCT types - varying field counts */
  global_struct_type = create_test_struct(__FIELD_COUNT__);
  all_types[type_index++] = global_struct_type;
  all_types[type_index++] = create_test_struct(1);
  all_types[type_index++] = create_test_struct(5);
  
  /* 5. UNION types - varying field counts */
  global_union_type = create_test_union(__FIELD_COUNT__);
  all_types[type_index++] = global_union_type;
  all_types[type_index++] = create_test_union(2);
  all_types[type_index++] = create_test_union(4);
  
  /* 6. STRING type (pointer to char) */
  global_string_type = build_pointer_type(char_type_node);
  all_types[type_index++] = global_string_type;
  
  /* 7. CALLBACK types (function pointers) */
  global_callback_type = create_test_callback();
  all_types[type_index++] = global_callback_type;
  all_types[type_index++] = build_pointer_type(
    build_function_type(void_type_node, NULL_TREE));
  
  /* 8. USER_STRUCT type - mark a struct with user flag */
  global_user_struct_type = create_test_struct(3);
  TYPE_USER_ALIGN(global_user_struct_type) = 1;
  all_types[type_index++] = global_user_struct_type;
  
  /* 9. LANG_STRUCT type - mark with language-specific flag */
  global_lang_struct_type = create_test_struct(2);
#ifdef TYPE_LANG_FLAG_0
  TYPE_LANG_FLAG_0(global_lang_struct_type) = 1;
#endif
  all_types[type_index++] = global_lang_struct_type;
  
  /* Force gengtype processing of all types */
  for (int i = 0; i < type_index; i++) {
    /* Use GTY macros to ensure types are processed */
    gt_ggc_mx(all_types[i]);
  }
  
  /* Additional processing through global variables */
  gt_ggc_mx(global_scalar_type);
  gt_ggc_mx(global_pointer_type);
  gt_ggc_mx(global_array_type);
  gt_ggc_mx(global_struct_type);
  gt_ggc_mx(global_union_type);
  gt_ggc_mx(global_string_type);
  gt_ggc_mx(global_callback_type);
  gt_ggc_mx(global_user_struct_type);
  gt_ggc_mx(global_lang_struct_type);
}

/* Main entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif
