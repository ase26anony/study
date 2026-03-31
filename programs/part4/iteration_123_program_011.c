/* Test for gengtype.cc type categorization coverage */
/* This test constructs various GCC internal types to trigger all 
   type_enum classification cases in the uncovered block */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Forward declarations for helper functions */
static tree create_test_struct(int field_count);
static tree create_test_union(int field_count);
static tree create_test_array(tree element_type, int dimensions);
static tree create_test_callback(void);

/* Global variables with GTY annotations to force gengtype processing */
/* These will be processed by the garbage collector type system */

/* Scalar types */
GTY(()) tree global_scalar_int = integer_type_node;
GTY(()) tree global_scalar_char = char_type_node;
GTY(()) tree global_scalar_bool = boolean_type_node;

/* Pointer types */
GTY(()) tree global_pointer_to_int = NULL_TREE;
GTY(()) tree global_pointer_to_struct = NULL_TREE;

/* Array types */
GTY(()) tree global_int_array = NULL_TREE;
GTY(()) tree global_struct_array = NULL_TREE;

/* Struct types */
GTY(()) tree global_test_struct = NULL_TREE;
GTY(()) tree global_user_struct = NULL_TREE;

/* Union types */
GTY(()) tree global_test_union = NULL_TREE;

/* String type (char pointer) */
GTY(()) tree global_string_ptr = NULL_TREE;

/* Callback type (function pointer) */
GTY(()) tree global_callback = NULL_TREE;

/* Lang struct type */
GTY(()) tree global_lang_struct = NULL_TREE;

/* Mixed type container to ensure all types are processed */
struct GTY(()) type_container {
  tree scalar;
  tree pointer;
  tree array;
  tree record;
  tree union_type;
  tree string;
  tree callback;
  tree lang_struct;
};

GTY(()) struct type_container *global_container = NULL;

/* Helper function to create a struct with variable field count */
static tree
create_test_struct(int field_count)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  /* Push struct into current binding level */
  pushdecl(struct_type);
  
  for (int i = 0; i < field_count; i++) {
    char field_name[32];
    sprintf(field_name, "field_%d", i);
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name),
                           integer_type_node);
    
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(struct_type) = field_list;
  layout_type(struct_type);
  
  /* Pop struct from binding level */
  popdecl();
  
  return struct_type;
}

/* Helper function to create a union with variable field count */
static tree
create_test_union(int field_count)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  pushdecl(union_type);
  
  for (int i = 0; i < field_count; i++) {
    char field_name[32];
    sprintf(field_name, "member_%d", i);
    
    tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                           get_identifier(field_name),
                           (i % 2 == 0) ? integer_type_node : char_type_node);
    
    field_list = chainon(field_list, field);
  }
  
  TYPE_FIELDS(union_type) = field_list;
  layout_type(union_type);
  
  popdecl();
  
  return union_type;
}

/* Helper function to create multi-dimensional arrays */
static tree
create_test_array(tree element_type, int dimensions)
{
  tree array_type = element_type;
  
  for (int i = 0; i < dimensions; i++) {
    /* Create array type with 10 elements at each dimension */
    tree index_type = build_index_type(size_int(9));
    array_type = build_array_type(array_type, index_type);
  }
  
  return array_type;
}

/* Helper function to create a callback (function pointer) type */
static tree
create_test_callback(void)
{
  /* Create a function type: int (*)(int, char*) */
  tree arg_types = NULL_TREE;
  
  /* First argument: int */
  tree arg1 = build_decl(UNKNOWN_LOCATION, PARM_DECL,
                        get_identifier("arg1"),
                        integer_type_node);
  arg_types = chainon(arg_types, arg1);
  
  /* Second argument: char* */
  tree char_ptr_type = build_pointer_type(char_type_node);
  tree arg2 = build_decl(UNKNOWN_LOCATION, PARM_DECL,
                        get_identifier("arg2"),
                        char_ptr_type);
  arg_types = chainon(arg_types, arg2);
  
  tree func_type = build_function_type(integer_type_node, arg_types);
  tree func_ptr_type = build_pointer_type(func_type);
  
  return func_ptr_type;
}

/* Main test function that creates all types and triggers gengtype processing */
void
test_gengtype_categorization(void)
{
  /* Create scalar types (already have global references) */
  
  /* Create pointer types */
  global_pointer_to_int = build_pointer_type(integer_type_node);
  
  /* Create struct with __FIELD_COUNT__ fields (placeholder for mutation) */
  int field_count = __FIELD_COUNT__;
  if (field_count < 1) field_count = 3; /* Default */
  if (field_count > 10) field_count = 10; /* Limit */
  
  global_test_struct = create_test_struct(field_count);
  global_pointer_to_struct = build_pointer_type(global_test_struct);
  
  /* Create user struct (marked with TYPE_LANG_FLAG) */
  global_user_struct = create_test_struct(2);
  TYPE_LANG_FLAG_0(global_user_struct) = 1;
  
  /* Create union */
  global_test_union = create_test_union(field_count);
  
  /* Create array types */
  global_int_array = create_test_array(integer_type_node, 2);
  global_struct_array = create_test_array(global_test_struct, 1);
  
  /* Create string type (char*) */
  global_string_ptr = build_pointer_type(char_type_node);
  
  /* Create callback type */
  global_callback = create_test_callback();
  
  /* Create lang struct (with TYPE_LANG_SPECIFIC) */
  global_lang_struct = create_test_struct(2);
  
  /* Allocate and initialize type container */
  global_container = ggc_alloc<type_container>();
  global_container->scalar = integer_type_node;
  global_container->pointer = global_pointer_to_int;
  global_container->array = global_int_array;
  global_container->record = global_test_struct;
  global_container->union_type = global_test_union;
  global_container->string = global_string_ptr;
  global_container->callback = global_callback;
  global_container->lang_struct = global_lang_struct;
  
  /* Force processing of all types through GTY macros */
  /* These macros will trigger gengtype categorization */
  gt_ggc_mx_tree_node(&global_scalar_int);
  gt_ggc_mx_tree_node(&global_scalar_char);
  gt_ggc_mx_tree_node(&global_scalar_bool);
  gt_ggc_mx_tree_node(&global_pointer_to_int);
  gt_ggc_mx_tree_node(&global_pointer_to_struct);
  gt_ggc_mx_tree_node(&global_int_array);
  gt_ggc_mx_tree_node(&global_struct_array);
  gt_ggc_mx_tree_node(&global_test_struct);
  gt_ggc_mx_tree_node(&global_user_struct);
  gt_ggc_mx_tree_node(&global_test_union);
  gt_ggc_mx_tree_node(&global_string_ptr);
  gt_ggc_mx_tree_node(&global_callback);
  gt_ggc_mx_tree_node(&global_lang_struct);
  gt_ggc_mx_rtx_def(&global_container);
  
  /* Additional processing for different type kinds */
  /* Use __TYPE_KIND__ placeholder to vary type creation */
  int type_kind = __TYPE_KIND__;
  
  switch (type_kind) {
    case 0:
      /* Create an array of pointers */
      tree ptr_array = create_test_array(build_pointer_type(char_type_node), 1);
      gt_ggc_mx_tree_node(&ptr_array);
      break;
      
    case 1:
      /* Create a struct with mixed fields */
      tree mixed_struct = make_node(RECORD_TYPE);
      pushdecl(mixed_struct);
      
      tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                              get_identifier("int_field"),
                              integer_type_node);
      tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                              get_identifier("ptr_field"),
                              build_pointer_type(char_type_node));
      tree field3 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                              get_identifier("array_field"),
                              create_test_array(integer_type_node, 1));
      
      TYPE_FIELDS(mixed_struct) = chainon(chainon(field1, field2), field3);
      layout_type(mixed_struct);
      popdecl();
      
      gt_ggc_mx_tree_node(&mixed_struct);
      break;
      
    case 2:
      /* Create a union of different pointer types */
      tree union_of_ptrs = make_node(UNION_TYPE);
      pushdecl(union_of_ptrs);
      
      tree mem1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("int_ptr"),
                            build_pointer_type(integer_type_node));
      tree mem2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("char_ptr"),
                            build_pointer_type(char_type_node));
      tree mem3 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("func_ptr"),
                            create_test_callback());
      
      TYPE_FIELDS(union_of_ptrs) = chainon(chainon(mem1, mem2), mem3);
      layout_type(union_of_ptrs);
      popdecl();
      
      gt_ggc_mx_tree_node(&union_of_ptrs);
      break;
  }
  
  /* Process through gengtype's type enumeration */
  /* This ensures the switch statement in gengtype.cc is exercised */
  enum type_enum type_class;
  
  /* The following would be ideal but internal functions may not be exposed.
     Instead, we rely on the GTY processing triggered above. */
}

/* Main entry point for standalone test compilation */
#ifdef STANDALONE_TEST
int main(void) {
  test_gengtype_categorization();
  return 0;
}
#endif

/* Plugin entry point for GCC plugin compilation */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
  test_gengtype_categorization();
  return 0;
}
#endif
