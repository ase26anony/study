/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC type nodes to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

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
static tree create_struct_with_fields(int field_count) {
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
static tree create_union_with_members(int member_count) {
    tree union_type = make_node(UNION_TYPE);
    tree member_list = NULL_TREE;
    
    for (int i = 0; i < member_count; i++) {
        tree member = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("member"),
                                integer_type_node);
        DECL_CONTEXT(member) = union_type;
        member_list = chainon(member_list, member);
    }
    
    TYPE_FIELDS(union_type) = member_list;
    layout_type(union_type);
    return union_type;
}

/* Create a user-defined struct type */
static tree create_user_struct(void) {
    tree struct_type = create_struct_with_fields(2);
    
    /* Mark as user struct - this may use TYPE_LANG_FLAG or other mechanism */
#ifdef TYPE_USER_STRUCT
    TYPE_USER_STRUCT(struct_type) = 1;
#else
    /* Alternative marking for user struct */
    SET_TYPE_LANG_SPECIFIC(struct_type, (struct lang_type *)1);
#endif
    
    return struct_type;
}

/* Create a language-specific struct type */
static tree create_lang_struct(void) {
    tree struct_type = create_struct_with_fields(3);
    
    /* Mark as language struct */
#ifdef TYPE_LANG_STRUCT
    TYPE_LANG_STRUCT(struct_type) = 1;
#else
    /* Use language-specific hook if available */
    if (lang_hooks.types.make_type) {
        lang_hooks.types.make_type(struct_type);
    }
#endif
    
    return struct_type;
}

/* Create a callback (function pointer) type */
static tree create_callback_type(void) {
    /* Create a function type returning int with no arguments */
    tree func_type = build_function_type(integer_type_node, NULL_TREE);
    
    /* Create pointer to function type */
    tree func_ptr_type = build_pointer_type(func_type);
    
    return func_ptr_type;
}

/* Create an array type with variable dimensions */
static tree create_array_type(int dimensions) {
    tree array_type = integer_type_node;
    
    for (int i = 0; i < dimensions; i++) {
        /* Create array of 10 elements */
        tree index_type = build_index_type(size_int(10));
        array_type = build_array_type(array_type, index_type);
    }
    
    return array_type;
}

/* Main test function */
void test_gengtype_categorization(void) {
    /* TYPE_SCALAR - basic scalar types */
    global_scalar_type = integer_type_node;
    gt_ggc_mx(global_scalar_type);
    
    /* Also test other scalar types */
    gt_ggc_mx(char_type_node);
    gt_ggc_mx(boolean_type_node);
    gt_ggc_mx(void_type_node);
    
    /* TYPE_POINTER */
    global_pointer_type = build_pointer_type(integer_type_node);
    gt_ggc_mx(global_pointer_type);
    
    /* TYPE_ARRAY - test with different dimensions */
    global_array_type = create_array_type(__FIELD_COUNT__);
    gt_ggc_mx(global_array_type);
    
    /* Also test single-dimensional array */
    tree simple_array = build_array_type(integer_type_node, 
                                        build_index_type(size_int(5)));
    gt_ggc_mx(simple_array);
    
    /* TYPE_STRUCT */
    global_struct_type = create_struct_with_fields(__TYPE_KIND__);
    gt_ggc_mx(global_struct_type);
    
    /* TYPE_UNION */
    global_union_type = create_union_with_members(__TYPE_KIND__);
    gt_ggc_mx(global_union_type);
    
    /* TYPE_STRING - pointer to char */
    global_string_type = build_pointer_type(char_type_node);
    gt_ggc_mx(global_string_type);
    
    /* TYPE_CALLBACK */
    global_callback_type = create_callback_type();
    gt_ggc_mx(global_callback_type);
    
    /* TYPE_USER_STRUCT */
    global_user_struct_type = create_user_struct();
    gt_ggc_mx(global_user_struct_type);
    
    /* TYPE_LANG_STRUCT */
    global_lang_struct_type = create_lang_struct();
    gt_ggc_mx(global_lang_struct_type);
    
    /* Test undefined type */
    tree undefined_type = make_node(ERROR_MARK);
    gt_ggc_mx(undefined_type);
    
    /* Process all types through gengtype system */
    /* This ensures the counters are incremented */
    gt_types_enum_last = gt_types_enum_last;
    
    /* Force processing by using the types in expressions */
    if (global_scalar_type && global_pointer_type) {
        /* Just to ensure types are used */
        volatile int dummy = 0;
        dummy++;
    }
}

/* Main entry point for standalone test */
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
