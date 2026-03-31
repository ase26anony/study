/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to ensure all type_enum
   categories are processed by the gengtype system. */

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

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree scalar_types[5];
static GTY(()) tree pointer_types[3];
static GTY(()) tree array_types[4];
static GTY(()) tree struct_types[3];
static GTY(()) tree union_types[2];
static GTY(()) tree callback_types[2];
static GTY(()) tree string_type;
static GTY(()) tree lang_struct_type;
static GTY(()) tree user_struct_type;

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  int i;
  
  /* 1. SCALAR TYPES (TYPE_SCALAR) */
  scalar_types[0] = integer_type_node;
  scalar_types[1] = char_type_node;
  scalar_types[2] = boolean_type_node;
  scalar_types[3] = size_type_node;
  scalar_types[4] = ptrdiff_type_node;
  
  /* Force processing of scalar types */
  for (i = 0; i < 5; i++) {
    if (scalar_types[i])
      gt_ggc_mx (scalar_types[i]);
  }
  
  /* 2. POINTER TYPES (TYPE_POINTER) */
  pointer_types[0] = build_pointer_type(integer_type_node);
  pointer_types[1] = build_pointer_type(char_type_node);
  pointer_types[2] = build_pointer_type(build_pointer_type(integer_type_node));
  
  for (i = 0; i < 3; i++) {
    if (pointer_types[i])
      gt_ggc_mx (pointer_types[i]);
  }
  
  /* 3. ARRAY TYPES (TYPE_ARRAY) */
  /* Single-dimensional array */
  array_types[0] = build_array_type(integer_type_node, 
                                   build_index_type(size_int(__FIELD_COUNT__)));
  
  /* Multi-dimensional array */
  tree index_type = build_index_type(size_int(5));
  tree array1 = build_array_type(char_type_node, index_type);
  array_types[1] = build_array_type(array1, index_type);
  
  /* Variable length array */
  array_types[2] = build_array_type(integer_type_node, NULL_TREE);
  
  /* Array of pointers */
  array_types[3] = build_array_type(pointer_types[0], index_type);
  
  for (i = 0; i < 4; i++) {
    if (array_types[i])
      gt_ggc_mx (array_types[i]);
  }
  
  /* 4. STRUCT TYPES (TYPE_STRUCT) */
  struct_types[0] = create_test_struct(1);  /* Small struct */
  struct_types[1] = create_test_struct(__FIELD_COUNT__);  /* Variable size struct */
  struct_types[2] = create_test_struct(3);  /* Medium struct */
  
  for (i = 0; i < 3; i++) {
    if (struct_types[i])
      gt_ggc_mx (struct_types[i]);
  }
  
  /* 5. UNION TYPES (TYPE_UNION) */
  union_types[0] = create_test_union(2);
  union_types[1] = create_test_union(__FIELD_COUNT__);
  
  for (i = 0; i < 2; i++) {
    if (union_types[i])
      gt_ggc_mx (union_types[i]);
  }
  
  /* 6. STRING TYPE (TYPE_STRING) */
  /* In GCC, string type is pointer to char */
  string_type = build_pointer_type(char_type_node);
  if (string_type)
    gt_ggc_mx (string_type);
  
  /* 7. CALLBACK TYPES (TYPE_CALLBACK) */
  callback_types[0] = create_test_callback();
  /* Function pointer type */
  tree return_type = integer_type_node;
  tree arg_types = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
  tree func_type = build_function_type(return_type, arg_types);
  callback_types[1] = build_pointer_type(func_type);
  
  for (i = 0; i < 2; i++) {
    if (callback_types[i])
      gt_ggc_mx (callback_types[i]);
  }
  
  /* 8. LANG_STRUCT TYPE (TYPE_LANG_STRUCT) */
  /* Create a struct and mark it as language-specific */
  lang_struct_type = create_test_struct(2);
  if (lang_struct_type) {
    /* Mark as language-specific structure */
    struct lang_type *lt = ggc_alloc<struct lang_type>();
    TYPE_LANG_SPECIFIC(lang_struct_type) = lt;
    gt_ggc_mx (lang_struct_type);
  }
  
  /* 9. USER_STRUCT TYPE (TYPE_USER_STRUCT) */
  /* Create a user-defined struct type */
  user_struct_type = create_test_struct(2);
  if (user_struct_type) {
    /* Mark with user struct flag if available */
    #ifdef TYPE_USER_STRUCT
    TYPE_USER_STRUCT(user_struct_type) = 1;
    #endif
    gt_ggc_mx (user_struct_type);
  }
  
  /* 10. Process all types through alternative path */
  /* This ensures types are processed even if not in GTY vars */
  gt_types_enum_last = gt_types_get_last ();
  
  /* Additional processing to ensure coverage */
  process_all_types();
}

/* Helper function to create a test struct with specified number of fields */
static tree
create_test_struct(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  tree last_field = NULL_TREE;
  int i;
  
  /* Push struct into current binding level */
  pushdecl (build_decl (UNKNOWN_LOCATION, TYPE_DECL, 
                       get_identifier ("test_struct"), struct_type));
  
  /* Create fields */
  for (i = 0; i < field_count; i++) {
    char field_name[32];
    snprintf(field_name, sizeof(field_name), "field%d", i);
    
    tree field_type;
    switch (i % 4) {
      case 0: field_type = integer_type_node; break;
      case 1: field_type = char_type_node; break;
      case 2: field_type = build_pointer_type(integer_type_node); break;
      case 3: field_type = build_array_type(char_type_node, 
                                           build_index_type(size_int(10))); break;
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
  
  pushdecl (build_decl (UNKNOWN_LOCATION, TYPE_DECL,
                       get_identifier ("test_union"), union_type));
  
  for (i = 0; i < field_count; i++) {
    char field_name[32];
    snprintf(field_name, sizeof(field_name), "u_field%d", i);
    
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

/* Helper function to create multi-dimensional array */
static tree
create_test_array(tree element_type, int dimensions)
{
  tree array_type = element_type;
  int i;
  
  for (i = 0; i < dimensions; i++) {
    tree index = build_index_type(size_int(5 + i));
    array_type = build_array_type(array_type, index);
  }
  
  return array_type;
}

/* Helper function to create a callback (function pointer) type */
static tree
create_test_callback(void)
{
  /* Create function type with mixed parameters */
  tree return_type = void_type_node;
  
  /* Build parameter list */
  tree param_list = NULL_TREE;
  tree last_param = NULL_TREE;
  
  /* Add some parameters */
  tree param1 = build_decl(UNKNOWN_LOCATION, PARM_DECL,
                          get_identifier("arg1"), integer_type_node);
  tree param2 = build_decl(UNKNOWN_LOCATION, PARM_DECL,
                          get_identifier("arg2"), build_pointer_type(char_type_node));
  
  param_list = chainon(param1, param2);
  
  /* Create function type */
  tree func_type = build_function_type(return_type, param_list);
  
  /* Create pointer to function */
  return build_pointer_type(func_type);
}

/* Function to process all created types through various gengtype paths */
static void
process_all_types(void)
{
  /* Process through gt_pch_nx path */
  gt_pch_nx (&scalar_types[0]);
  gt_pch_nx (&pointer_types[0]);
  gt_pch_nx (&struct_types[0]);
  
  /* Process individual types */
  if (string_type)
    gt_pch_nx (&string_type);
  
  /* Mark types for GC */
  ggc_mark (scalar_types[0]);
  ggc_mark (pointer_types[0]);
  ggc_mark (struct_types[0]);
  ggc_mark (union_types[0]);
  ggc_mark (callback_types[0]);
  
  /* Ensure lang_struct gets marked if it exists */
  if (lang_struct_type) {
    ggc_mark (lang_struct_type);
    if (TYPE_LANG_SPECIFIC(lang_struct_type)) {
      ggc_mark (TYPE_LANG_SPECIFIC(lang_struct_type));
    }
  }
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC internal structures */
  init_tree();
  
  test_gengtype_categorization();
  
  return 0;
}
#endif

/* Plugin entry point if compiled as plugin */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
