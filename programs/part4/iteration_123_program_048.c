/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Guard for GCC version compatibility */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 4
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 9
#endif

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree test_scalar_var;
static GTY(()) tree test_pointer_var;
static GTY(()) tree test_array_var;
static GTY(()) tree test_struct_var;
static GTY(()) tree test_union_var;
static GTY(()) tree test_string_var;
static GTY(()) tree test_callback_var;
static GTY(()) tree test_user_struct_var;
static GTY(()) tree test_lang_struct_var;

/* Helper to create a struct with variable field count */
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
    layout_type(union_type);
    return union_type;
}

/* Helper to create an array with variable dimensions */
static tree create_test_array(tree element_type, int dimensions) {
    tree array_type = element_type;
    
    for (int i = 0; i < dimensions; i++) {
        tree index_type = build_index_type(size_int(10 + i));
        array_type = build_array_type(array_type, index_type);
    }
    
    return array_type;
}

/* Helper to mark a type as user struct */
static void mark_as_user_struct(tree type) {
    /* Set language-specific flag to indicate user struct */
    TYPE_LANG_FLAG_0(type) = 1;
}

/* Helper to mark a type as language struct */
static void mark_as_lang_struct(tree type) {
    /* Create and attach language-specific data */
    struct lang_type *lang = (struct lang_type *)xmalloc(sizeof(struct lang_type));
    TYPE_LANG_SPECIFIC(type) = lang;
}

/* Main test function */
void test_gengtype_categorization(void) {
    /* Create various types to cover all cases */
    
    /* 1. SCALAR types */
    test_scalar_var = integer_type_node;  /* TYPE_SCALAR */
    gt_ggc_mx(tree, &test_scalar_var);
    
    test_scalar_var = boolean_type_node;  /* Another scalar */
    gt_ggc_mx(tree, &test_scalar_var);
    
    test_scalar_var = char_type_node;     /* Another scalar */
    gt_ggc_mx(tree, &test_scalar_var);
    
    /* 2. POINTER types */
    tree ptr_to_int = build_pointer_type(integer_type_node);
    test_pointer_var = ptr_to_int;        /* TYPE_POINTER */
    gt_ggc_mx(tree, &test_pointer_var);
    
    /* 3. ARRAY types */
    tree simple_array = build_array_type(integer_type_node, 
                                        build_index_type(size_int(5)));
    test_array_var = simple_array;        /* TYPE_ARRAY */
    gt_ggc_mx(tree, &test_array_var);
    
    /* Multi-dimensional array */
    tree multi_array = create_test_array(integer_type_node, __FIELD_COUNT__);
    test_array_var = multi_array;
    gt_ggc_mx(tree, &test_array_var);
    
    /* 4. STRUCT types */
    tree simple_struct = create_test_struct(2);
    test_struct_var = simple_struct;      /* TYPE_STRUCT */
    gt_ggc_mx(tree, &test_struct_var);
    
    /* Struct with variable field count */
    tree var_struct = create_test_struct(__FIELD_COUNT__);
    test_struct_var = var_struct;
    gt_ggc_mx(tree, &test_struct_var);
    
    /* 5. UNION types */
    tree simple_union = create_test_union(2);
    test_union_var = simple_union;        /* TYPE_UNION */
    gt_ggc_mx(tree, &test_union_var);
    
    /* Union with variable field count */
    tree var_union = create_test_union(__FIELD_COUNT__);
    test_union_var = var_union;
    gt_ggc_mx(tree, &test_union_var);
    
    /* 6. STRING type (pointer to char) */
    tree string_type = build_pointer_type(char_type_node);
    test_string_var = string_type;        /* TYPE_STRING */
    gt_ggc_mx(tree, &test_string_var);
    
    /* 7. CALLBACK type (function pointer) */
    tree func_type = build_function_type(integer_type_node, NULL_TREE);
    tree func_ptr = build_pointer_type(func_type);
    test_callback_var = func_ptr;         /* TYPE_CALLBACK */
    gt_ggc_mx(tree, &test_callback_var);
    
    /* 8. USER STRUCT type */
    tree user_struct = create_test_struct(3);
    mark_as_user_struct(user_struct);
    test_user_struct_var = user_struct;   /* TYPE_USER_STRUCT */
    gt_ggc_mx(tree, &test_user_struct_var);
    
    /* 9. LANG STRUCT type */
    tree lang_struct = create_test_struct(3);
    mark_as_lang_struct(lang_struct);
    test_lang_struct_var = lang_struct;   /* TYPE_LANG_STRUCT */
    gt_ggc_mx(tree, &test_lang_struct_var);
    
    /* Process all types through gengtype machinery */
    gt_types_enum_last = gt_types_enum_last;
    
    /* Force processing of all globals */
    gt_ggc_mx(&test_scalar_var);
    gt_ggc_mx(&test_pointer_var);
    gt_ggc_mx(&test_array_var);
    gt_ggc_mx(&test_struct_var);
    gt_ggc_mx(&test_union_var);
    gt_ggc_mx(&test_string_var);
    gt_ggc_mx(&test_callback_var);
    gt_ggc_mx(&test_user_struct_var);
    gt_ggc_mx(&test_lang_struct_var);
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
