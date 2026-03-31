/* test_gengtype_categorization.c - Comprehensive test for gengtype type categorization */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "ggc.h"

/* Forward declarations */
void test_gengtype_categorization(void);

/* GTY-annotated globals to force gengtype processing */
static GTY(()) tree scalar_types[3];
static GTY(()) tree pointer_types[2];
static GTY(()) tree array_types[2];
static GTY(()) tree struct_type;
static GTY(()) tree union_type;
static GTY(()) tree callback_type;
static GTY(()) tree string_type;
static GTY(()) tree lang_struct_type;
static GTY(()) tree user_struct_type;

/* Helper to create struct with variable field count */
static tree
create_struct_with_fields(int field_count)
{
  tree struct_node = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("field"),
                           integer_type_node);
    DECL_CONTEXT(field) = struct_node;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(struct_node) = field_list;
  layout_type(struct_node);
  return struct_node;
}

/* Helper to create union with variable field count */
static tree
create_union_with_fields(int field_count)
{
  tree union_node = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("field"),
                           integer_type_node);
    DECL_CONTEXT(field) = union_node;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_node) = field_list;
  layout_type(union_node);
  return union_node;
}

/* Main test function */
void
test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES (TYPE_SCALAR) */
  scalar_types[0] = integer_type_node;      /* int */
  scalar_types[1] = char_type_node;         /* char */
  scalar_types[2] = boolean_type_node;      /* bool */
  
  /* Force processing of scalar types */
  gt_ggc_mx (scalar_types);
  
  /* 2. POINTER TYPES (TYPE_POINTER) */
  pointer_types[0] = build_pointer_type(integer_type_node);
  pointer_types[1] = build_pointer_type(char_type_node);
  
  /* 3. ARRAY TYPES (TYPE_ARRAY) */
  tree index_type = build_index_type(size_int(10));
  array_types[0] = build_array_type(integer_type_node, index_type);
  
  /* Variable length array */
  tree vla_index = build_index_type(size_int(__FIELD_COUNT__));
  array_types[1] = build_array_type(char_type_node, vla_index);
  
  /* 4. STRUCT TYPES (TYPE_STRUCT) */
  struct_type = create_struct_with_fields(__FIELD_COUNT__);
  
  /* 5. UNION TYPES (TYPE_UNION) */
  union_type = create_union_with_fields(__FIELD_COUNT__);
  
  /* 6. STRING TYPE (TYPE_STRING) - char* */
  string_type = build_pointer_type(char_type_node);
  
  /* 7. CALLBACK TYPE (TYPE_CALLBACK) - function pointer */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  callback_type = build_pointer_type(func_type);
  
  /* 8. LANG STRUCT TYPE (TYPE_LANG_STRUCT) */
  lang_struct_type = create_struct_with_fields(2);
#ifdef TYPE_LANG_SPECIFIC
  TYPE_LANG_SPECIFIC(lang_struct_type) = (struct lang_type *)1;
#endif
  
  /* 9. USER STRUCT TYPE (TYPE_USER_STRUCT) */
  user_struct_type = create_struct_with_fields(2);
#ifdef TYPE_USER_STRUCT
  /* Mark as user struct if the macro is defined */
  TYPE_USER_STRUCT(user_struct_type) = 1;
#endif
  
  /* Force processing of all types through GTY system */
  gt_ggc_mx (pointer_types);
  gt_ggc_mx (array_types);
  gt_ggc_mx (&struct_type);
  gt_ggc_mx (&union_type);
  gt_ggc_mx (&string_type);
  gt_ggc_mx (&callback_type);
  gt_ggc_mx (&lang_struct_type);
  gt_ggc_mx (&user_struct_type);
  
  /* Create additional type variations to ensure coverage */
  
  /* Nested struct with pointer field */
  tree nested_struct = make_node(RECORD_TYPE);
  tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("ptr_field"),
                             pointer_types[0]);
  DECL_CONTEXT(ptr_field) = nested_struct;
  TYPE_FIELDS(nested_struct) = ptr_field;
  layout_type(nested_struct);
  gt_ggc_mx (&nested_struct);
  
  /* Array of structs */
  tree array_of_structs = build_array_type(struct_type, index_type);
  gt_ggc_mx (&array_of_structs);
  
  /* Pointer to array */
  tree ptr_to_array = build_pointer_type(array_types[0]);
  gt_ggc_mx (&ptr_to_array);
  
  /* Struct containing array */
  tree struct_with_array = make_node(RECORD_TYPE);
  tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("array_field"),
                               array_types[0]);
  DECL_CONTEXT(array_field) = struct_with_array;
  TYPE_FIELDS(struct_with_array) = array_field;
  layout_type(struct_with_array);
  gt_ggc_mx (&struct_with_array);
  
  /* Union with different field types */
  tree mixed_union = make_node(UNION_TYPE);
  tree union_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("int_field"),
                                integer_type_node);
  tree union_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("ptr_field"),
                                pointer_types[0]);
  DECL_CONTEXT(union_field1) = mixed_union;
  DECL_CONTEXT(union_field2) = mixed_union;
  TYPE_FIELDS(mixed_union) = chainon(union_field1, union_field2);
  layout_type(mixed_union);
  gt_ggc_mx (&mixed_union);
  
  /* Complex callback with parameters */
  tree param_list = NULL_TREE;
  tree param1 = build_decl(UNKNOWN_LOCATION, PARM_DECL,
                          get_identifier("arg1"),
                          integer_type_node);
  tree param2 = build_decl(UNKNOWN_LOCATION, PARM_DECL,
                          get_identifier("arg2"),
                          char_type_node);
  param_list = chainon(param_list, param1);
  param_list = chainon(param_list, param2);
  
  tree func_type_with_params = build_function_type(integer_type_node, param_list);
  tree complex_callback = build_pointer_type(func_type_with_params);
  gt_ggc_mx (&complex_callback);
}

/* Main entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* Plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
