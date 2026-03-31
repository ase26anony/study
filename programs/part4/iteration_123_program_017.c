/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* This test creates various GCC type nodes to trigger all type_enum categorization cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

/* Guard for version-specific features */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 4
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 9
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

/* Helper function to create a struct with variable field count */
static tree create_test_struct(int field_count) {
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
    TYPE_NAME(struct_type) = get_identifier("TestStruct");
    layout_type(struct_type);
    
    return struct_type;
}

/* Helper to create a union with variable field count */
static tree create_test_union(int field_count) {
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
    TYPE_NAME(union_type) = get_identifier("TestUnion");
    layout_type(union_type);
    
    return union_type;
}

/* Main test function */
void test_gengtype_categorization(void) {
    /* 1. SCALAR TYPES - triggers TYPE_SCALAR case */
    __scalar_var__ = integer_type_node;      /* int */
    gt_ggc_mx(integer_type_node);
    
    __scalar_var__ = char_type_node;         /* char */
    gt_ggc_mx(char_type_node);
    
    __scalar_var__ = boolean_type_node;      /* bool */
    gt_ggc_mx(boolean_type_node);
    
    /* 2. POINTER TYPES - triggers TYPE_POINTER case */
    tree int_ptr_type = build_pointer_type(integer_type_node);
    __pointer_var__ = int_ptr_type;
    gt_ggc_mx(int_ptr_type);
    
    /* 3. ARRAY TYPES - triggers TYPE_ARRAY case */
    /* Create array with __FIELD_COUNT__ elements (placeholder for mutation) */
    int array_size = __FIELD_COUNT__ > 0 ? __FIELD_COUNT__ : 10;
    tree index_type = build_index_type(build_int_cst(integer_type_node, array_size - 1));
    tree array_type = build_array_type(integer_type_node, index_type);
    __array_var__ = array_type;
    gt_ggc_mx(array_type);
    
    /* Multi-dimensional array */
    tree index_type2 = build_index_type(build_int_cst(integer_type_node, 5));
    tree md_array_type = build_array_type(array_type, index_type2);
    gt_ggc_mx(md_array_type);
    
    /* 4. STRUCT TYPES - triggers TYPE_STRUCT case */
    tree struct_type = create_test_struct(__FIELD_COUNT__ > 0 ? __FIELD_COUNT__ : 3);
    __struct_var__ = struct_type;
    gt_ggc_mx(struct_type);
    
    /* 5. UNION TYPES - triggers TYPE_UNION case */
    tree union_type = create_test_union(__FIELD_COUNT__ > 0 ? __FIELD_COUNT__ : 2);
    __union_var__ = union_type;
    gt_ggc_mx(union_type);
    
    /* 6. STRING TYPE - triggers TYPE_STRING case */
    /* In GCC, string type is pointer to char */
    tree string_type = build_pointer_type(char_type_node);
    __string_var__ = string_type;
    gt_ggc_mx(string_type);
    
    /* 7. CALLBACK TYPES (function pointers) - triggers TYPE_CALLBACK case */
    tree func_type = build_function_type(integer_type_node, NULL_TREE);
    tree func_ptr_type = build_pointer_type(func_type);
    __callback_var__ = func_ptr_type;
    gt_ggc_mx(func_ptr_type);
    
    /* Variadic function pointer */
    tree var_func_type = build_function_type(integer_type_node, NULL_TREE);
    TYPE_ARG_TYPES(var_func_type) = void_list_node;
    tree var_func_ptr = build_pointer_type(var_func_type);
    gt_ggc_mx(var_func_ptr);
    
    /* 8. USER STRUCT - triggers TYPE_USER_STRUCT case */
    /* Mark a struct as user-defined type */
    tree user_struct = create_test_struct(2);
    TYPE_LANG_SPECIFIC(user_struct) = (struct lang_type *)1;  /* Simulate language-specific data */
    __user_struct_var__ = user_struct;
    gt_ggc_mx(user_struct);
    
    /* 9. LANG STRUCT - triggers TYPE_LANG_STRUCT case */
    /* Create a language-specific structure type */
    tree lang_struct = make_node(RECORD_TYPE);
    TYPE_FIELDS(lang_struct) = NULL_TREE;
    TYPE_NAME(lang_struct) = get_identifier("LangStruct");
    
    /* Simulate language-specific marking */
    TYPE_LANG_FLAG_0(lang_struct) = 1;
    TYPE_LANG_FLAG_1(lang_struct) = 1;
    __lang_struct_var__ = lang_struct;
    gt_ggc_mx(lang_struct);
    
    /* 10. Test TYPE_UNDEFINED - this might be harder to trigger intentionally,
       but we can try creating an incomplete type */
    tree incomplete = make_node(RECORD_TYPE);
    TYPE_SIZE(incomplete) = NULL_TREE;
    gt_ggc_mx(incomplete);
    
    /* Process all types through gengtype machinery */
    /* Force processing by using the GTY macros on each type */
    gt_pch_nx(&__scalar_var__);
    gt_pch_nx(&__pointer_var__);
    gt_pch_nx(&__array_var__);
    gt_pch_nx(&__struct_var__);
    gt_pch_nx(&__union_var__);
    gt_pch_nx(&__string_var__);
    gt_pch_nx(&__callback_var__);
    gt_pch_nx(&__user_struct_var__);
    gt_pch_nx(&__lang_struct_var__);
    
    /* Additional type variations for comprehensive coverage */
    
    /* Pointer to struct */
    tree struct_ptr = build_pointer_type(struct_type);
    gt_ggc_mx(struct_ptr);
    
    /* Array of pointers */
    tree ptr_array_type = build_array_type(int_ptr_type, index_type);
    gt_ggc_mx(ptr_array_type);
    
    /* Struct containing pointers */
    tree struct_with_ptrs = make_node(RECORD_TYPE);
    tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("data"),
                            int_ptr_type);
    tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("next"),
                            build_pointer_type(struct_with_ptrs));
    TYPE_FIELDS(struct_with_ptrs) = chainon(field1, field2);
    layout_type(struct_with_ptrs);
    gt_ggc_mx(struct_with_ptrs);
    
    /* Union with different field types */
    tree mixed_union = make_node(UNION_TYPE);
    tree ufield1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("as_int"),
                             integer_type_node);
    tree ufield2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("as_ptr"),
                             int_ptr_type);
    TYPE_FIELDS(mixed_union) = chainon(ufield1, ufield2);
    layout_type(mixed_union);
    gt_ggc_mx(mixed_union);
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void) {
    test_gengtype_categorization();
    return 0;
}
#endif

/* Plugin entry point if compiled as GCC plugin */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
    test_gengtype_categorization();
    return 0;
}
#endif
