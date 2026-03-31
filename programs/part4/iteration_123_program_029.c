/* test_gengtype_categorization.c - Comprehensive test for gengtype type categorization */
/* This test must be compiled within the GCC build environment */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "stringpool.h"
#include "attribs.h"
#include "stor-layout.h"

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

/* Helper to create a struct with variable field count */
static tree
create_struct_with_fields(int field_count)
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
create_union_with_fields(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier_with_length("field", 5),
                           integer_type_node);
    DECL_CONTEXT(field) = union_type;
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  return union_type;
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES - TYPE_SCALAR */
  global_scalar_type = integer_type_node;  /* This will increment nb_scalar */
  gt_ggc_mx(global_scalar_type);
  
  /* Also test other scalar types */
  gt_ggc_mx(char_type_node);
  gt_ggc_mx(boolean_type_node);
  gt_ggc_mx(size_type_node);
  
  /* 2. POINTER TYPES - TYPE_POINTER */
  tree pointer_to_int = build_pointer_type(integer_type_node);
  global_pointer_type = pointer_to_int;
  gt_ggc_mx(global_pointer_type);
  
  /* Multiple pointer variations */
  gt_ggc_mx(build_pointer_type(char_type_node));
  gt_ggc_mx(build_pointer_type(build_pointer_type(integer_type_node)));
  
  /* 3. ARRAY TYPES - TYPE_ARRAY */
  tree array_type = build_array_type(integer_type_node, NULL_TREE);
  global_array_type = array_type;
  gt_ggc_mx(global_array_type);
  
  /* Array with bounds */
  tree index_type = build_index_type(size_int(10));
  tree bounded_array = build_array_type(char_type_node, index_type);
  gt_ggc_mx(bounded_array);
  
  /* 4. STRUCT TYPES - TYPE_STRUCT */
  tree simple_struct = create_struct_with_fields(__FIELD_COUNT__);
  global_struct_type = simple_struct;
  gt_ggc_mx(global_struct_type);
  
  /* Struct with pointer field */
  tree struct_with_ptr = make_node(RECORD_TYPE);
  tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("ptr_field"),
                             build_pointer_type(integer_type_node));
  DECL_CONTEXT(ptr_field) = struct_with_ptr;
  TYPE_FIELDS(struct_with_ptr) = ptr_field;
  layout_type(struct_with_ptr);
  gt_ggc_mx(struct_with_ptr);
  
  /* 5. UNION TYPES - TYPE_UNION */
  tree simple_union = create_union_with_fields(__FIELD_COUNT__);
  global_union_type = simple_union;
  gt_ggc_mx(global_union_type);
  
  /* 6. STRING TYPE - TYPE_STRING */
  /* In GCC, string type is pointer to char */
  tree string_ptr = build_pointer_type(char_type_node);
  global_string_type = string_ptr;
  gt_ggc_mx(global_string_type);
  
  /* 7. CALLBACK TYPE - TYPE_CALLBACK */
  /* Function pointer type */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  tree func_ptr_type = build_pointer_type(func_type);
  global_callback_type = func_ptr_type;
  gt_ggc_mx(global_callback_type);
  
  /* Function type with arguments */
  tree arg_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
  arg_list = tree_cons(NULL_TREE, char_type_node, arg_list);
  tree func_with_args = build_function_type(integer_type_node, arg_list);
  gt_ggc_mx(build_pointer_type(func_with_args));
  
  /* 8. USER STRUCT TYPE - TYPE_USER_STRUCT */
  /* Mark a struct as user-defined */
  tree user_struct = create_struct_with_fields(2);
  /* Set some flag to mark as user struct - implementation dependent */
  #ifdef TYPE_USER_STRUCT
  TYPE_USER_STRUCT(user_struct) = 1;
  #endif
  global_user_struct_type = user_struct;
  gt_ggc_mx(global_user_struct_type);
  
  /* 9. LANG STRUCT TYPE - TYPE_LANG_STRUCT */
  /* Language-specific struct type */
  tree lang_struct = create_struct_with_fields(1);
  /* Set language-specific data */
  #ifdef TYPE_LANG_SPECIFIC
  SET_TYPE_LANG_SPECIFIC(lang_struct, (struct lang_type *)1);
  #endif
  global_lang_struct_type = lang_struct;
  gt_ggc_mx(global_lang_struct_type);
  
  /* 10. Process undefined type if possible */
  #ifdef TYPE_UNDEFINED
  /* Some systems might have undefined type marker */
  #endif
  
  /* Force processing of all globals through multiple paths */
  gt_pch_nx(&global_scalar_type);
  gt_pch_nx(&global_pointer_type);
  gt_pch_nx(&global_array_type);
  gt_pch_nx(&global_struct_type);
  gt_pch_nx(&global_union_type);
  gt_pch_nx(&global_string_type);
  gt_pch_nx(&global_callback_type);
  gt_pch_nx(&global_user_struct_type);
  gt_pch_nx(&global_lang_struct_type);
}

/* Alternative: Use a container struct with GTY annotation */
typedef struct GTY(()) type_container {
  tree scalar;
  tree pointer;
  tree array;
  tree record;
  tree union_type;
  tree string;
  tree callback;
  tree user_struct;
  tree lang_struct;
} type_container_t;

static GTY(()) type_container_t all_types;

void __attribute__((noinline))
test_gengtype_via_container(void)
{
  /* Fill container with all type categories */
  all_types.scalar = integer_type_node;
  all_types.pointer = build_pointer_type(char_type_node);
  all_types.array = build_array_type(integer_type_node, NULL_TREE);
  all_types.record = create_struct_with_fields(3);
  all_types.union_type = create_union_with_fields(2);
  all_types.string = build_pointer_type(char_type_node);
  
  tree func_type = build_function_type(void_type_node, NULL_TREE);
  all_types.callback = build_pointer_type(func_type);
  
  tree user_struct = create_struct_with_fields(1);
  #ifdef TYPE_USER_STRUCT
  TYPE_USER_STRUCT(user_struct) = 1;
  #endif
  all_types.user_struct = user_struct;
  
  tree lang_struct = create_struct_with_fields(1);
  #ifdef TYPE_LANG_SPECIFIC
  SET_TYPE_LANG_SPECIFIC(lang_struct, (struct lang_type *)1);
  #endif
  all_types.lang_struct = lang_struct;
  
  /* Force processing */
  gt_ggc_mx(all_types);
  gt_pch_nx(&all_types);
}

/* Main entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC environment if needed */
  test_gengtype_categorization();
  test_gengtype_via_container();
  return 0;
}
#endif

/* Plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  test_gengtype_via_container();
  return 0;
}
#endif
