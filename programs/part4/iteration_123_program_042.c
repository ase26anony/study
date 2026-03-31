/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "stringpool.h"
#include "attribs.h"

/* Guard for GCC version compatibility */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 4
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 9
#endif

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree scalar_types[3];
static GTY(()) tree pointer_types[2];
static GTY(()) tree array_types[2];
static GTY(()) tree struct_types[2];
static GTY(()) tree union_types[1];
static GTY(()) tree callback_types[1];
static GTY(()) tree string_type;
static GTY(()) tree lang_struct_type;
static GTY(()) tree user_struct_type;

/* Helper function to create a struct with variable field count */
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

/* Helper function to create a union with variable field count */
static tree
create_union_with_fields(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier_with_length("ufield", 6),
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
  /* 1. SCALAR TYPES - triggers TYPE_SCALAR case */
  scalar_types[0] = integer_type_node;
  scalar_types[1] = char_type_node;
  scalar_types[2] = boolean_type_node;
  
  /* Force processing of scalar types */
  gt_ggc_mx (scalar_types);
  
  /* 2. POINTER TYPES - triggers TYPE_POINTER case */
  pointer_types[0] = build_pointer_type(integer_type_node);
  pointer_types[1] = build_pointer_type(char_type_node);
  
  /* Process pointer types */
  gt_ggc_mx (pointer_types);
  
  /* 3. ARRAY TYPES - triggers TYPE_ARRAY case */
  tree index_type = build_index_type(size_int(__FIELD_COUNT__));
  array_types[0] = build_array_type(integer_type_node, index_type);
  
  /* Multi-dimensional array */
  tree range_type = build_range_type(integer_type_node,
                                     size_int(0),
                                     size_int(__FIELD_COUNT__ - 1));
  array_types[1] = build_array_type(char_type_node, range_type);
  
  gt_ggc_mx (array_types);
  
  /* 4. STRUCT TYPES - triggers TYPE_STRUCT case */
  struct_types[0] = create_struct_with_fields(__FIELD_COUNT__);
  struct_types[1] = create_struct_with_fields(__FIELD_COUNT__ * 2);
  
  /* Mark one as user struct - triggers TYPE_USER_STRUCT case */
  user_struct_type = struct_types[0];
  SET_TYPE_LANG_SPECIFIC(user_struct_type, (struct lang_type *)1);
  
  gt_ggc_mx (struct_types);
  gt_ggc_mx (&user_struct_type);
  
  /* 5. UNION TYPES - triggers TYPE_UNION case */
  union_types[0] = create_union_with_fields(__FIELD_COUNT__);
  
  gt_ggc_mx (union_types);
  
  /* 6. STRING TYPE - triggers TYPE_STRING case */
  /* In GCC, string type is pointer to char */
  string_type = build_pointer_type(char_type_node);
  
  gt_ggc_mx (&string_type);
  
  /* 7. CALLBACK TYPES - triggers TYPE_CALLBACK case */
  /* Function pointer type */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  callback_types[0] = build_pointer_type(func_type);
  
  gt_ggc_mx (callback_types);
  
  /* 8. LANG STRUCT TYPE - triggers TYPE_LANG_STRUCT case */
  /* Create a struct and mark it as language-specific */
  lang_struct_type = create_struct_with_fields(1);
  TYPE_LANG_FLAG_0(lang_struct_type) = 1;
  
  gt_ggc_mx (&lang_struct_type);
  
  /* 9. Process all types together to ensure coverage */
  tree all_types[] = {
    integer_type_node,                    /* SCALAR */
    build_pointer_type(char_type_node),   /* POINTER */
    array_types[0],                       /* ARRAY */
    struct_types[0],                      /* STRUCT */
    union_types[0],                       /* UNION */
    string_type,                          /* STRING */
    callback_types[0],                    /* CALLBACK */
    user_struct_type,                     /* USER_STRUCT */
    lang_struct_type                      /* LANG_STRUCT */
  };
  
  /* Force processing through gengtype categorization */
  for (size_t i = 0; i < sizeof(all_types)/sizeof(all_types[0]); i++) {
    if (all_types[i]) {
      gt_ggc_m_9tree_node (all_types[i]);
    }
  }
  
  /* Additional test: Process types in different orders */
  switch (__TYPE_KIND__) {
    case 0:
      gt_ggc_mx (&scalar_types[0]);
      break;
    case 1:
      gt_ggc_mx (&pointer_types[0]);
      break;
    case 2:
      gt_ggc_mx (&array_types[0]);
      break;
    case 3:
      gt_ggc_mx (&struct_types[0]);
      break;
    case 4:
      gt_ggc_mx (&union_types[0]);
      break;
    default:
      /* Process everything */
      gt_ggc_mx (all_types);
      break;
  }
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* GCC plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif

/* In-tree test suite entry */
#ifdef GENGTYPE_TEST
void gt_types_test(void)
{
  test_gengtype_categorization();
}
#endif
