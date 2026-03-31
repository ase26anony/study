/* test_gengtype_categorization.c - Comprehensive test for gengtype type categorization */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"

/* Forward declarations for helper functions */
static tree create_test_struct(int field_count);
static tree create_test_union(int field_count);
static tree create_test_array(tree element_type, int dimensions);
static tree create_test_callback(void);

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree global_scalar_type = NULL_TREE;
static GTY(()) tree global_pointer_type = NULL_TREE;
static GTY(()) tree global_array_type = NULL_TREE;
static GTY(()) tree global_struct_type = NULL_TREE;
static GTY(()) tree global_union_type = NULL_TREE;
static GTY(()) tree global_string_type = NULL_TREE;
static GTY(()) tree global_callback_type = NULL_TREE;
static GTY(()) tree global_user_struct_type = NULL_TREE;
static GTY(()) tree global_lang_struct_type = NULL_TREE;

/* Container struct to hold multiple types */
struct GTY(()) type_container {
  tree scalar_type;
  tree pointer_type;
  tree array_type;
  tree struct_type;
  tree union_type;
  tree string_type;
  tree callback_type;
  tree user_struct_type;
  tree lang_struct_type;
};

static GTY(()) struct type_container *global_container = NULL;

/* Helper to mark a type as user-defined */
static void mark_as_user_struct(tree type)
{
#ifdef TYPE_USER_STRUCT
  TYPE_USER_STRUCT(type) = 1;
#else
  /* Fallback: set a lang-specific flag if available */
  if (TYPE_LANG_SPECIFIC(type))
    TYPE_LANG_SPECIFIC(type)->user_struct = 1;
#endif
}

/* Helper to mark a type as language-specific */
static void mark_as_lang_struct(tree type)
{
#ifdef TYPE_LANG_STRUCT
  TYPE_LANG_STRUCT(type) = 1;
#else
  /* Create and attach language-specific data */
  struct lang_type *lang = ggc_alloc<struct lang_type>();
  TYPE_LANG_SPECIFIC(type) = lang;
#endif
}

/* Create a struct type with variable field count */
static tree create_test_struct(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    char field_name[32];
    snprintf(field_name, sizeof(field_name), "field_%d", i);
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name),
                           integer_type_node);
    DECL_CONTEXT(field) = struct_type;
    
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(struct_type) = field_list;
  layout_type(struct_type);
  
  return struct_type;
}

/* Create a union type with variable field count */
static tree create_test_union(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  for (int i = 0; i < field_count; i++) {
    char field_name[32];
    snprintf(field_name, sizeof(field_name), "u_field_%d", i);
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name),
                           (i % 2 == 0) ? integer_type_node : char_type_node);
    DECL_CONTEXT(field) = union_type;
    
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  
  return union_type;
}

/* Create a multi-dimensional array type */
static tree create_test_array(tree element_type, int dimensions)
{
  tree array_type = element_type;
  
  for (int i = 0; i < dimensions; i++) {
    /* Create array with 5 elements at each dimension */
    tree index_type = build_index_type(size_int(4));
    array_type = build_array_type(array_type, index_type);
  }
  
  return array_type;
}

/* Create a callback (function pointer) type */
static tree create_test_callback(void)
{
  /* Create a function type returning int with two int parameters */
  tree return_type = integer_type_node;
  tree arg_types = NULL_TREE;
  
  tree arg1 = build_decl(UNKNOWN_LOCATION, PARM_DECL,
                        get_identifier("arg1"),
                        integer_type_node);
  tree arg2 = build_decl(UNKNOWN_LOCATION, PARM_DECL,
                        get_identifier("arg2"),
                        integer_type_node);
  
  arg_types = tree_cons(NULL_TREE, TREE_TYPE(arg1), arg_types);
  arg_types = tree_cons(NULL_TREE, TREE_TYPE(arg2), arg_types);
  
  tree func_type = build_function_type(return_type, arg_types);
  tree func_ptr_type = build_pointer_type(func_type);
  
  return func_ptr_type;
}

/* Main test function that creates all type categories */
void test_gengtype_categorization(void)
{
  /* 1. SCALAR TYPES */
  global_scalar_type = integer_type_node;      /* TYPE_SCALAR */
  gt_ggc_mx(global_scalar_type);
  
  /* Also test other scalar types */
  tree char_type = char_type_node;
  tree bool_type = boolean_type_node;
  gt_ggc_mx(char_type);
  gt_ggc_mx(bool_type);
  
  /* 2. POINTER TYPE */
  global_pointer_type = build_pointer_type(integer_type_node);
  gt_ggc_mx(global_pointer_type);
  
  /* 3. ARRAY TYPE - create with variable dimensions */
  int array_dims = __FIELD_COUNT__ % 3 + 1;  /* 1-3 dimensions */
  global_array_type = create_test_array(integer_type_node, array_dims);
  gt_ggc_mx(global_array_type);
  
  /* 4. STRUCT TYPE - create with variable field count */
  int struct_fields = __FIELD_COUNT__ % 5 + 1;  /* 1-5 fields */
  global_struct_type = create_test_struct(struct_fields);
  gt_ggc_mx(global_struct_type);
  
  /* 5. UNION TYPE - create with variable field count */
  int union_fields = __FIELD_COUNT__ % 4 + 1;  /* 1-4 fields */
  global_union_type = create_test_union(union_fields);
  gt_ggc_mx(global_union_type);
  
  /* 6. STRING TYPE (pointer to char) */
  global_string_type = build_pointer_type(char_type_node);
  gt_ggc_mx(global_string_type);
  
  /* 7. CALLBACK TYPE (function pointer) */
  global_callback_type = create_test_callback();
  gt_ggc_mx(global_callback_type);
  
  /* 8. USER STRUCT TYPE */
  global_user_struct_type = create_test_struct(2);
  mark_as_user_struct(global_user_struct_type);
  gt_ggc_mx(global_user_struct_type);
  
  /* 9. LANG STRUCT TYPE */
  global_lang_struct_type = create_test_struct(3);
  mark_as_lang_struct(global_lang_struct_type);
  gt_ggc_mx(global_lang_struct_type);
  
  /* Create a container holding all types to ensure they're processed together */
  global_container = ggc_alloc<struct type_container>();
  global_container->scalar_type = global_scalar_type;
  global_container->pointer_type = global_pointer_type;
  global_container->array_type = global_array_type;
  global_container->struct_type = global_struct_type;
  global_container->union_type = global_union_type;
  global_container->string_type = global_string_type;
  global_container->callback_type = global_callback_type;
  global_container->user_struct_type = global_user_struct_type;
  global_container->lang_struct_type = global_lang_struct_type;
  
  /* Force processing of the entire container */
  gt_ggc_mx(global_container);
  
  /* Additional test: Process types through different gengtype entry points */
  
  /* Test with pointer chains */
  tree double_ptr = build_pointer_type(global_pointer_type);
  gt_ggc_mx(double_ptr);
  
  /* Test array of structs */
  tree array_of_structs = build_array_type(global_struct_type,
                                          build_index_type(size_int(10)));
  gt_ggc_mx(array_of_structs);
  
  /* Test struct containing pointers */
  tree struct_with_ptr = make_node(RECORD_TYPE);
  tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("ptr_field"),
                             global_pointer_type);
  DECL_CONTEXT(ptr_field) = struct_with_ptr;
  TYPE_FIELDS(struct_with_ptr) = ptr_field;
  layout_type(struct_with_ptr);
  gt_ggc_mx(struct_with_ptr);
  
  /* Test union with array */
  tree union_with_array = make_node(UNION_TYPE);
  tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("array_field"),
                               global_array_type);
  DECL_CONTEXT(array_field) = union_with_array;
  TYPE_FIELDS(union_with_array) = array_field;
  layout_type(union_with_array);
  gt_ggc_mx(union_with_array);
}

/* Entry point for standalone test compilation */
#ifdef STANDALONE_TEST
int main(void)
{
  /* Initialize GCC's type system if needed */
  test_gengtype_categorization();
  return 0;
}
#endif

/* Plugin entry point if compiled as GCC plugin */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  test_gengtype_categorization();
  return 0;
}
#endif
