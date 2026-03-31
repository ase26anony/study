/* Test for gengtype.cc type categorization coverage */
/* This test constructs various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree __SCALAR_TYPE__ = NULL_TREE;
static GTY(()) tree __POINTER_TYPE__ = NULL_TREE;
static GTY(()) tree __ARRAY_TYPE__ = NULL_TREE;
static GTY(()) tree __STRUCT_TYPE__ = NULL_TREE;
static GTY(()) tree __UNION_TYPE__ = NULL_TREE;
static GTY(()) tree __STRING_TYPE__ = NULL_TREE;
static GTY(()) tree __CALLBACK_TYPE__ = NULL_TREE;
static GTY(()) tree __LANG_STRUCT_TYPE__ = NULL_TREE;
static GTY(()) tree __USER_STRUCT_TYPE__ = NULL_TREE;

/* Helper to create a struct with variable field count */
static tree
create_struct_with_fields(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
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

/* Helper to create a union with variable member count */
static tree
create_union_with_members(int member_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree member_list = NULL_TREE;
  
  for (int i = 0; i < member_count; i++) {
    tree member = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("member"),
                            (i % 2 == 0) ? integer_type_node : char_type_node);
    DECL_CONTEXT(member) = union_type;
    member_list = chainon(member_list, member);
  }
  
  TYPE_FIELDS(union_type) = member_list;
  layout_type(union_type);
  return union_type;
}

/* Main test function */
void __attribute__((noinline))
test_gengtype_categorization(void)
{
  /* TYPE_SCALAR - various scalar types */
  __SCALAR_TYPE__ = integer_type_node;      /* int */
  gt_ggc_mx(&__SCALAR_TYPE__);
  
  __SCALAR_TYPE__ = char_type_node;         /* char */
  gt_ggc_mx(&__SCALAR_TYPE__);
  
  __SCALAR_TYPE__ = boolean_type_node;      /* bool */
  gt_ggc_mx(&__SCALAR_TYPE__);
  
  __SCALAR_TYPE__ = void_type_node;         /* void */
  gt_ggc_mx(&__SCALAR_TYPE__);
  
  /* TYPE_POINTER */
  __POINTER_TYPE__ = build_pointer_type(integer_type_node);
  gt_ggc_mx(&__POINTER_TYPE__);
  
  /* TYPE_ARRAY with variable dimensions */
  tree array_type1 = build_array_type(integer_type_node, NULL_TREE);
  __ARRAY_TYPE__ = array_type1;
  gt_ggc_mx(&__ARRAY_TYPE__);
  
  tree array_type2 = build_array_type_nelts(char_type_node, 10);
  __ARRAY_TYPE__ = array_type2;
  gt_ggc_mx(&__ARRAY_TYPE__);
  
  /* TYPE_STRUCT with variable field count */
  __STRUCT_TYPE__ = create_struct_with_fields(__FIELD_COUNT__);
  gt_ggc_mx(&__STRUCT_TYPE__);
  
  /* TYPE_UNION with variable member count */
  __UNION_TYPE__ = create_union_with_members(__FIELD_COUNT__);
  gt_ggc_mx(&__UNION_TYPE__);
  
  /* TYPE_STRING - char* type */
  __STRING_TYPE__ = build_pointer_type(char_type_node);
  gt_ggc_mx(&__STRING_TYPE__);
  
  /* TYPE_CALLBACK - function pointer */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  __CALLBACK_TYPE__ = build_pointer_type(func_type);
  gt_ggc_mx(&__CALLBACK_TYPE__);
  
  /* TYPE_LANG_STRUCT - language-specific struct */
  tree lang_struct = create_struct_with_fields(2);
#ifdef TYPE_LANG_STRUCT
  SET_TYPE_LANG_SPECIFIC(lang_struct, (struct lang_type *)1);
#endif
  __LANG_STRUCT_TYPE__ = lang_struct;
  gt_ggc_mx(&__LANG_STRUCT_TYPE__);
  
  /* TYPE_USER_STRUCT - user-defined struct */
  tree user_struct = create_struct_with_fields(3);
#ifdef TYPE_USER_STRUCT
  TYPE_USER_STRUCT(user_struct) = 1;
#endif
  __USER_STRUCT_TYPE__ = user_struct;
  gt_ggc_mx(&__USER_STRUCT_TYPE__);
  
  /* Process all types through gengtype machinery */
  gt_types_enum_last = gt_types_enum_last;
  
  /* Force processing of type arrays */
  {
    tree type_array[] = {
      integer_type_node,
      __POINTER_TYPE__,
      __ARRAY_TYPE__,
      __STRUCT_TYPE__,
      __UNION_TYPE__,
      __STRING_TYPE__,
      __CALLBACK_TYPE__,
      __LANG_STRUCT_TYPE__,
      __USER_STRUCT_TYPE__
    };
    
    for (size_t i = 0; i < sizeof(type_array)/sizeof(type_array[0]); i++) {
      gt_ggc_mx(&type_array[i]);
    }
  }
}

/* Alternative: Use GTY annotations on a container struct */
typedef struct GTY(()) type_container {
  tree scalar;
  tree pointer;
  tree array;
  tree struct_type;
  tree union_type;
  tree string;
  tree callback;
  tree lang_struct;
  tree user_struct;
} type_container_t;

static GTY(()) type_container_t __TYPE_CONTAINER__;

void __attribute__((noinline))
test_gengtype_via_container(void)
{
  /* Fill container with all type categories */
  __TYPE_CONTAINER__.scalar = integer_type_node;
  __TYPE_CONTAINER__.pointer = build_pointer_type(char_type_node);
  __TYPE_CONTAINER__.array = build_array_type(integer_type_node, NULL_TREE);
  __TYPE_CONTAINER__.struct_type = create_struct_with_fields(__FIELD_COUNT__);
  __TYPE_CONTAINER__.union_type = create_union_with_members(__FIELD_COUNT__);
  __TYPE_CONTAINER__.string = build_pointer_type(char_type_node);
  
  tree func_type = build_function_type(void_type_node, NULL_TREE);
  __TYPE_CONTAINER__.callback = build_pointer_type(func_type);
  
  tree lang_struct = create_struct_with_fields(2);
#ifdef TYPE_LANG_STRUCT
  SET_TYPE_LANG_SPECIFIC(lang_struct, (struct lang_type *)1);
#endif
  __TYPE_CONTAINER__.lang_struct = lang_struct;
  
  tree user_struct = create_struct_with_fields(3);
#ifdef TYPE_USER_STRUCT
  TYPE_USER_STRUCT(user_struct) = 1;
#endif
  __TYPE_CONTAINER__.user_struct = user_struct;
  
  /* Force gengtype processing */
  gt_ggc_mx(&__TYPE_CONTAINER__);
}

/* Main entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC runtime if needed */
#ifdef GCC_INITIALIZE
  gcc_init();
#endif
  
  /* Run categorization tests */
  test_gengtype_categorization();
  test_gengtype_via_container();
  
  return 0;
}
#endif
