/* test_gengtype_categorization.c - Comprehensive test for gengtype type categorization */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "stringpool.h"
#include "attribs.h"

/* Forward declarations */
void test_gengtype_categorization(void);

/* GTY-annotated globals to force gengtype processing */
static GTY(()) tree scalar_types[5];
static GTY(()) tree pointer_types[3];
static GTY(()) tree array_types[3];
static GTY(()) tree struct_types[3];
static GTY(()) tree union_types[2];
static GTY(()) tree callback_types[2];
static GTY(()) tree string_type;
static GTY(()) tree lang_struct_type;

/* Helper to create a struct with variable field count */
static tree
create_test_struct(int field_count, const char *name)
{
  tree struct_type = make_node(RECORD_TYPE);
  tree field_list = NULL_TREE;
  
  if (name)
    TYPE_NAME(struct_type) = get_identifier(name);
  
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

/* Helper to create a union with variable field count */
static tree
create_test_union(int field_count, const char *name)
{
  tree union_type = make_node(UNION_TYPE);
  tree field_list = NULL_TREE;
  
  if (name)
    TYPE_NAME(union_type) = get_identifier(name);
  
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
  int i = 0;
  
  /* 1. SCALAR TYPES (TYPE_SCALAR) */
  scalar_types[i++] = integer_type_node;
  scalar_types[i++] = char_type_node;
  scalar_types[i++] = boolean_type_node;
  scalar_types[i++] = size_type_node;
  scalar_types[i++] = ptrdiff_type_node;
  
  /* 2. POINTER TYPES (TYPE_POINTER) */
  pointer_types[0] = build_pointer_type(integer_type_node);
  pointer_types[1] = build_pointer_type(char_type_node);
  pointer_types[2] = build_pointer_type(void_type_node);
  
  /* 3. ARRAY TYPES (TYPE_ARRAY) */
  tree index_type = build_index_type(size_int(10));
  array_types[0] = build_array_type(char_type_node, index_type);
  array_types[1] = build_array_type(integer_type_node, index_type);
  array_types[2] = build_array_type_nelts(pointer_types[0], 5);
  
  /* 4. STRUCT TYPES (TYPE_STRUCT) */
  struct_types[0] = create_test_struct(__FIELD_COUNT__, "TestStruct1");
  struct_types[1] = create_test_struct(2, "TestStruct2");
  struct_types[2] = create_test_struct(3, "TestStruct3");
  
  /* 5. UNION TYPES (TYPE_UNION) */
  union_types[0] = create_test_union(2, "TestUnion1");
  union_types[1] = create_test_union(3, "TestUnion2");
  
  /* 6. STRING TYPE (TYPE_STRING) - char* */
  string_type = build_pointer_type(char_type_node);
  
  /* 7. CALLBACK TYPES (TYPE_CALLBACK) - function pointers */
  tree void_ftype = build_function_type_list(void_type_node, NULL_TREE);
  callback_types[0] = build_pointer_type(void_ftype);
  
  tree int_ftype = build_function_type_list(integer_type_node,
                                           integer_type_node,
                                           integer_type_node,
                                           NULL_TREE);
  callback_types[1] = build_pointer_type(int_ftype);
  
  /* 8. USER STRUCT (TYPE_USER_STRUCT) */
  /* Create a struct and mark it with user flags */
  tree user_struct = create_test_struct(2, "UserStruct");
  TYPE_USER_ALIGN(user_struct) = 1;
  
  /* 9. LANG STRUCT (TYPE_LANG_STRUCT) */
  /* Create a struct with language-specific data */
  tree lang_struct = create_test_struct(2, "LangStruct");
#ifdef TYPE_LANG_SPECIFIC
  SET_TYPE_LANG_SPECIFIC(lang_struct, ggc_alloc<tree>());
#endif
  lang_struct_type = lang_struct;
  
  /* 10. Force processing through gengtype */
  /* The GTY annotations on the static arrays above will cause gengtype
     to process these types during compilation. Additionally, we can
     explicitly mark them for GC. */
  
  /* Register types with GC system */
  for (i = 0; i < 5; i++)
    if (scalar_types[i])
      gt_ggc_mx(scalar_types[i]);
  
  for (i = 0; i < 3; i++)
  {
    if (pointer_types[i])
      gt_ggc_mx(pointer_types[i]);
    if (array_types[i])
      gt_ggc_mx(array_types[i]);
    if (struct_types[i])
      gt_ggc_mx(struct_types[i]);
  }
  
  for (i = 0; i < 2; i++)
  {
    if (union_types[i])
      gt_ggc_mx(union_types[i]);
    if (callback_types[i])
      gt_ggc_mx(callback_types[i]);
  }
  
  if (string_type)
    gt_ggc_mx(string_type);
  
  if (user_struct)
    gt_ggc_mx(user_struct);
  
  if (lang_struct_type)
    gt_ggc_mx(lang_struct_type);
  
  /* Create some complex nested types to ensure thorough processing */
  tree nested_struct = create_test_struct(2, "NestedStruct");
  tree struct_with_array = create_test_struct(1, "StructWithArray");
  tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("data"),
                               array_types[0]);
  DECL_CONTEXT(array_field) = struct_with_array;
  TYPE_FIELDS(struct_with_array) = array_field;
  layout_type(struct_with_array);
  
  gt_ggc_mx(nested_struct);
  gt_ggc_mx(struct_with_array);
  
  /* Create pointer chains */
  tree ptr_to_ptr = build_pointer_type(pointer_types[0]);
  tree ptr_to_array = build_pointer_type(array_types[1]);
  gt_ggc_mx(ptr_to_ptr);
  gt_ggc_mx(ptr_to_array);
  
  /* Create array of structs */
  tree array_of_structs = build_array_type(struct_types[0], index_type);
  gt_ggc_mx(array_of_structs);
  
  /* Create struct containing pointers */
  tree struct_with_ptrs = create_test_struct(2, "StructWithPtrs");
  tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("ptr"),
                             pointer_types[1]);
  DECL_CONTEXT(ptr_field) = struct_with_ptrs;
  TYPE_FIELDS(struct_with_ptrs) = ptr_field;
  layout_type(struct_with_ptrs);
  gt_ggc_mx(struct_with_ptrs);
}

/* Optional: If compiling as standalone test with main */
#ifdef STANDALONE_TEST
int main(void)
{
  test_gengtype_categorization();
  return 0;
}
#endif
