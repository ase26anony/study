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
#include "ggc.h"

/* Global variables with GTY annotations to force type processing */
static GTY(()) tree __SCALAR_TYPE__ = integer_type_node;
static GTY(()) tree __POINTER_TYPE__ = NULL_TREE;
static GTY(()) tree __ARRAY_TYPE__ = NULL_TREE;
static GTY(()) tree __STRUCT_TYPE__ = NULL_TREE;
static GTY(()) tree __UNION_TYPE__ = NULL_TREE;
static GTY(()) tree __STRING_TYPE__ = NULL_TREE;
static GTY(()) tree __CALLBACK_TYPE__ = NULL_TREE;
static GTY(()) tree __USER_STRUCT_TYPE__ = NULL_TREE;
static GTY(()) tree __LANG_STRUCT_TYPE__ = NULL_TREE;

/* Helper function to create struct with variable field count */
static tree create_struct_type(int field_count) {
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

/* Helper function to create union with variable field count */
static tree create_union_type(int field_count) {
    tree union_type = make_node(UNION_TYPE);
    tree field_list = NULL_TREE;
    
    for (int i = 0; i < field_count; i++) {
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
void test_gengtype_categorization(void) {
    /* 1. SCALAR TYPES - TYPE_SCALAR */
    /* Already covered by integer_type_node in __SCALAR_TYPE__ */
    tree char_type = char_type_node;
    tree bool_type = boolean_type_node;
    
    /* Force processing of scalar types */
    gt_ggc_mx(integer_type_node);
    gt_ggc_mx(char_type);
    gt_ggc_mx(bool_type);
    
    /* 2. POINTER TYPES - TYPE_POINTER */
    tree int_ptr_type = build_pointer_type(integer_type_node);
    __POINTER_TYPE__ = int_ptr_type;
    gt_ggc_mx(int_ptr_type);
    
    /* 3. ARRAY TYPES - TYPE_ARRAY */
    tree index_type = build_index_type(size_int(__FIELD_COUNT__));
    tree array_type = build_array_type(integer_type_node, index_type);
    __ARRAY_TYPE__ = array_type;
    gt_ggc_mx(array_type);
    
    /* 4. STRUCT TYPES - TYPE_STRUCT */
    tree struct_type = create_struct_type(__FIELD_COUNT__);
    __STRUCT_TYPE__ = struct_type;
    gt_ggc_mx(struct_type);
    
    /* 5. UNION TYPES - TYPE_UNION */
    tree union_type = create_union_type(__FIELD_COUNT__);
    __UNION_TYPE__ = union_type;
    gt_ggc_mx(union_type);
    
    /* 6. STRING TYPE - TYPE_STRING */
    /* In GCC, string type is pointer to char */
    tree string_type = build_pointer_type(char_type_node);
    __STRING_TYPE__ = string_type;
    gt_ggc_mx(string_type);
    
    /* 7. CALLBACK TYPE - TYPE_CALLBACK */
    /* Function pointer type */
    tree func_type = build_function_type(integer_type_node, NULL_TREE);
    tree func_ptr_type = build_pointer_type(func_type);
    __CALLBACK_TYPE__ = func_ptr_type;
    gt_ggc_mx(func_type);
    gt_ggc_mx(func_ptr_type);
    
    /* 8. USER STRUCT TYPE - TYPE_USER_STRUCT */
    /* Create a struct and mark it as user-defined */
    tree user_struct = create_struct_type(2);
#ifdef TYPE_USER_STRUCT
    TYPE_USER_STRUCT(user_struct) = 1;
#endif
    __USER_STRUCT_TYPE__ = user_struct;
    gt_ggc_mx(user_struct);
    
    /* 9. LANG STRUCT TYPE - TYPE_LANG_STRUCT */
    /* Create a struct with language-specific data */
    tree lang_struct = create_struct_type(3);
#ifdef TYPE_LANG_SPECIFIC
    if (!TYPE_LANG_SPECIFIC(lang_struct)) {
        struct lang_type *lt = ggc_alloc<struct lang_type>();
        TYPE_LANG_SPECIFIC(lang_struct) = lt;
    }
#endif
    __LANG_STRUCT_TYPE__ = lang_struct;
    gt_ggc_mx(lang_struct);
    
    /* 10. Process all types through ggc_mark_roots */
    /* This ensures all types go through gengtype categorization */
    ggc_mark_roots();
    
    /* Additional type variations for comprehensive coverage */
    
    /* Array of pointers */
    tree array_of_ptrs = build_array_type(int_ptr_type, index_type);
    gt_ggc_mx(array_of_ptrs);
    
    /* Pointer to array */
    tree ptr_to_array = build_pointer_type(array_type);
    gt_ggc_mx(ptr_to_array);
    
    /* Struct containing various types */
    tree complex_struct = make_node(RECORD_TYPE);
    tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("int_field"),
                            integer_type_node);
    tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("ptr_field"),
                            int_ptr_type);
    tree field3 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("array_field"),
                            array_type);
    TYPE_FIELDS(complex_struct) = chainon(field1, chainon(field2, field3));
    layout_type(complex_struct);
    gt_ggc_mx(complex_struct);
    
    /* Union with pointer members */
    tree complex_union = make_node(UNION_TYPE);
    tree ufield1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("int_field"),
                             integer_type_node);
    tree ufield2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("func_ptr"),
                             func_ptr_type);
    TYPE_FIELDS(complex_union) = chainon(ufield1, ufield2);
    layout_type(complex_union);
    gt_ggc_mx(complex_union);
    
    /* Multi-dimensional array */
    tree index_type2 = build_index_type(size_int(5));
    tree md_array_type = build_array_type(array_type, index_type2);
    gt_ggc_mx(md_array_type);
    
    /* Void pointer */
    tree void_ptr_type = build_pointer_type(void_type_node);
    gt_ggc_mx(void_ptr_type);
    
    /* Const qualified types */
    tree const_int_type = build_qualified_type(integer_type_node, TYPE_QUAL_CONST);
    gt_ggc_mx(const_int_type);
    
    /* Volatile qualified types */
    tree volatile_ptr_type = build_qualified_type(int_ptr_type, TYPE_QUAL_VOLATILE);
    gt_ggc_mx(volatile_ptr_type);
    
    /* Function type with arguments */
    tree arg_types = tree_cons(NULL_TREE, integer_type_node,
                              tree_cons(NULL_TREE, char_type_node, NULL_TREE));
    tree func_with_args = build_function_type(integer_type_node, arg_types);
    gt_ggc_mx(func_with_args);
    
    /* Ensure all counters would be incremented */
    /* The actual increment happens inside gengtype when types are processed */
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void) {
    test_gengtype_categorization();
    return 0;
}
#endif

/* Plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
    test_gengtype_categorization();
    return 0;
}
#endif
