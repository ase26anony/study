/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -O0 -g -c test_gengtype_categorization.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "ggc.h"

/* Guard for version-specific features */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 0
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 0
#endif

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree __scalar_var__;
static GTY(()) tree __pointer_var__;
static GTY(()) tree __array_var__;
static GTY(()) tree __struct_var__;
static GTY(()) tree __union_var__;
static GTY(()) tree __string_var__;
static GTY(()) tree __callback_var__;
static GTY(()) tree __user_struct_var__;
static GTY(()) tree __lang_struct_var__;

/* Helper function to create struct with variable field count */
static tree
create_struct_with_fields(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier("field"),
                           integer_type_node);
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(struct_type) = field_list;
  layout_type(struct_type);
  return struct_type;
}

/* Helper function to create union with variable member count */
static tree
create_union_with_members(int member_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree member_list = NULL_TREE;
  
  for (int i = 0; i < member_count; i++) {
    tree member = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("member"),
                            integer_type_node);
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
  /* TYPE_SCALAR - Basic scalar types */
  __scalar_var__ = integer_type_node;      /* int */
  gt_ggc_mx(&__scalar_var__);
  
  __scalar_var__ = char_type_node;         /* char */
  gt_ggc_mx(&__scalar_var__);
  
  __scalar_var__ = boolean_type_node;      /* bool */
  gt_ggc_mx(&__scalar_var__);
  
  /* TYPE_POINTER - Pointer types */
  tree int_ptr_type = build_pointer_type(integer_type_node);
  __pointer_var__ = int_ptr_type;
  gt_ggc_mx(&__pointer_var__);
  
  /* TYPE_ARRAY - Array types */
  tree array_type = build_array_type(integer_type_node, NULL_TREE);
  __array_var__ = array_type;
  gt_ggc_mx(&__array_var__);
  
  /* Fixed-size array */
  tree fixed_array = build_array_type_nelts(integer_type_node, 10);
  __array_var__ = fixed_array;
  gt_ggc_mx(&__array_var__);
  
  /* TYPE_STRUCT - Struct types */
  tree struct_type = create_struct_with_fields(__FIELD_COUNT__);
  __struct_var__ = struct_type;
  gt_ggc_mx(&__struct_var__);
  
  /* TYPE_UNION - Union types */
  tree union_type = create_union_with_members(__FIELD_COUNT__);
  __union_var__ = union_type;
  gt_ggc_mx(&__union_var__);
  
  /* TYPE_STRING - String type (char*) */
  tree string_type = build_pointer_type(char_type_node);
  __string_var__ = string_type;
  gt_ggc_mx(&__string_var__);
  
  /* TYPE_CALLBACK - Function pointer types */
  tree func_type = build_function_type(integer_type_node, NULL_TREE);
  tree func_ptr_type = build_pointer_type(func_type);
  __callback_var__ = func_ptr_type;
  gt_ggc_mx(&__callback_var__);
  
  /* TYPE_USER_STRUCT - User-defined struct with special flag */
  tree user_struct = create_struct_with_fields(2);
  /* Mark as user struct if supported */
#ifdef SET_TYPE_USER_STRUCT
  SET_TYPE_USER_STRUCT(user_struct);
#endif
  __user_struct_var__ = user_struct;
  gt_ggc_mx(&__user_struct_var__);
  
  /* TYPE_LANG_STRUCT - Language-specific struct */
  tree lang_struct = create_struct_with_fields(3);
  /* Add language-specific data if available */
#ifdef TYPE_LANG_SPECIFIC
  if (TYPE_LANG_SPECIFIC(lang_struct) == NULL) {
    /* Create dummy lang-specific data */
    struct lang_type *lt = ggc_alloc<struct lang_type>();
    TYPE_LANG_SPECIFIC(lang_struct) = lt;
  }
#endif
  __lang_struct_var__ = lang_struct;
  gt_ggc_mx(&__lang_struct_var__);
  
  /* Process all types through gengtype categorization */
  /* This triggers the switch statement in gengtype.cc */
  ggc_mark_roots();
}

/* Alternative approach using direct type registration */
void __attribute__((noinline))
test_direct_type_registration(void)
{
  /* Create a variety of types and register them */
  enum {
    TYPE_KIND_SCALAR = 0,
    TYPE_KIND_POINTER,
    TYPE_KIND_ARRAY,
    TYPE_KIND_STRUCT,
    TYPE_KIND_UNION,
    TYPE_KIND_STRING,
    TYPE_KIND_CALLBACK,
    TYPE_KIND_USER_STRUCT,
    TYPE_KIND_LANG_STRUCT
  };
  
  int type_kind = __TYPE_KIND__;
  
  switch (type_kind) {
    case TYPE_KIND_SCALAR:
      gt_ggc_mx(&integer_type_node);
      break;
    case TYPE_KIND_POINTER:
      gt_ggc_mx(&build_pointer_type(integer_type_node));
      break;
    case TYPE_KIND_ARRAY:
      gt_ggc_mx(&build_array_type(integer_type_node, NULL_TREE));
      break;
    case TYPE_KIND_STRUCT: {
      tree s = create_struct_with_fields(3);
      gt_ggc_mx(&s);
      break;
    }
    case TYPE_KIND_UNION: {
      tree u = create_union_with_members(2);
      gt_ggc_mx(&u);
      break;
    }
    case TYPE_KIND_STRING:
      gt_ggc_mx(&build_pointer_type(char_type_node));
      break;
    case TYPE_KIND_CALLBACK: {
      tree f = build_function_type(integer_type_node, NULL_TREE);
      gt_ggc_mx(&build_pointer_type(f));
      break;
    }
    case TYPE_KIND_USER_STRUCT: {
      tree us = create_struct_with_fields(2);
#ifdef SET_TYPE_USER_STRUCT
      SET_TYPE_USER_STRUCT(us);
#endif
      gt_ggc_mx(&us);
      break;
    }
    case TYPE_KIND_LANG_STRUCT: {
      tree ls = create_struct_with_fields(3);
#ifdef TYPE_LANG_SPECIFIC
      if (TYPE_LANG_SPECIFIC(ls) == NULL) {
        struct lang_type *lt = ggc_alloc<struct lang_type>();
        TYPE_LANG_SPECIFIC(ls) = lt;
      }
#endif
      gt_ggc_mx(&ls);
      break;
    }
  }
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Test all type categories */
  for (int i = 0; i < 9; i++) {
    __TYPE_KIND__ = i;
    __FIELD_COUNT__ = (i % 3) + 1;  /* Vary field count 1-3 */
    test_gengtype_categorization();
    test_direct_type_registration();
  }
  
  return 0;
}
#endif

/* Plugin entry point if compiled as GCC plugin */
#ifdef GCC_PLUGIN
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
