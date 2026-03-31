/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* This test creates various GCC type nodes to exercise all type_enum classification cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

/* Guard for GCC version-specific features */
#ifndef TYPE_USER_STRUCT
#define TYPE_USER_STRUCT 4
#endif

#ifndef TYPE_LANG_STRUCT
#define TYPE_LANG_STRUCT 9
#endif

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

/* Helper function to create a struct with variable field count */
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

/* Helper function to create a union with variable field count */
static tree create_union_with_fields(int field_count) {
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

/* Create a user-defined struct type */
static tree create_user_struct(void) {
    tree struct_type = create_struct_with_fields(2);
    
    /* Mark as user struct - this might be version dependent */
    /* In some GCC versions, this is done via lang-specific flags */
    struct lang_type *lang_specific = ggc_alloc<struct lang_type>();
    SET_TYPE_LANG_SPECIFIC(struct_type, lang_specific);
    
    return struct_type;
}

/* Create a language-specific struct type */
static tree create_lang_struct(void) {
    tree struct_type = create_struct_with_fields(3);
    
    /* Mark as language-specific struct */
    TYPE_LANG_FLAG_0(struct_type) = 1;
    TYPE_LANG_FLAG_1(struct_type) = 1;
    
    return struct_type;
}

/* Create a callback/function pointer type */
static tree create_callback_type(void) {
    /* Create a function type returning int with no arguments */
    tree return_type = integer_type_node;
    tree arg_types = void_list_node;
    tree func_type = build_function_type(return_type, arg_types);
    
    /* Create pointer to function */
    tree func_ptr_type = build_pointer_type(func_type);
    
    return func_ptr_type;
}

/* Main test function */
void test_gengtype_categorization(void) {
    /* 1. SCALAR TYPES - TYPE_SCALAR */
    global_scalar_type = integer_type_node;
    gt_ggc_mx(global_scalar_type);
    
    /* Also test other scalar types */
    gt_ggc_mx(char_type_node);
    gt_ggc_mx(boolean_type_node);
    gt_ggc_mx(size_type_node);
    
    /* 2. POINTER TYPES - TYPE_POINTER */
    global_pointer_type = build_pointer_type(integer_type_node);
    gt_ggc_mx(global_pointer_type);
    
    /* 3. ARRAY TYPES - TYPE_ARRAY */
    /* Create array with __FIELD_COUNT__ elements (placeholder) */
    int array_size = 10; /* Can be mutated to different values */
    tree index_type = build_index_type(size_int(array_size - 1));
    global_array_type = build_array_type(integer_type_node, index_type);
    gt_ggc_mx(global_array_type);
    
    /* Test multi-dimensional array */
    tree md_index_type = build_index_type(size_int(5));
    tree md_array_type = build_array_type(global_array_type, md_index_type);
    gt_ggc_mx(md_array_type);
    
    /* 4. STRUCT TYPES - TYPE_STRUCT */
    /* Create struct with __FIELD_COUNT__ fields (placeholder) */
    int struct_field_count = 3; /* Can be mutated to different values */
    global_struct_type = create_struct_with_fields(struct_field_count);
    gt_ggc_mx(global_struct_type);
    
    /* 5. UNION TYPES - TYPE_UNION */
    /* Create union with __FIELD_COUNT__ fields (placeholder) */
    int union_field_count = 2; /* Can be mutated to different values */
    global_union_type = create_union_with_fields(union_field_count);
    gt_ggc_mx(global_union_type);
    
    /* 6. STRING TYPE - TYPE_STRING */
    /* String type is pointer to char */
    global_string_type = build_pointer_type(char_type_node);
    gt_ggc_mx(global_string_type);
    
    /* 7. CALLBACK TYPE - TYPE_CALLBACK */
    global_callback_type = create_callback_type();
    gt_ggc_mx(global_callback_type);
    
    /* 8. USER STRUCT TYPE - TYPE_USER_STRUCT */
    global_user_struct_type = create_user_struct();
    gt_ggc_mx(global_user_struct_type);
    
    /* 9. LANG STRUCT TYPE - TYPE_LANG_STRUCT */
    global_lang_struct_type = create_lang_struct();
    gt_ggc_mx(global_lang_struct_type);
    
    /* Test nested types to ensure comprehensive coverage */
    tree struct_with_pointer = create_struct_with_fields(1);
    TYPE_FIELDS(struct_with_pointer) = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                                 get_identifier("ptr_field"),
                                                 global_pointer_type);
    gt_ggc_mx(struct_with_pointer);
    
    tree struct_with_array = create_struct_with_fields(1);
    TYPE_FIELDS(struct_with_array) = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                               get_identifier("array_field"),
                                               global_array_type);
    gt_ggc_mx(struct_with_array);
    
    /* Force processing of all globals */
    gt_ggc_mx(&global_scalar_type);
    gt_ggc_mx(&global_pointer_type);
    gt_ggc_mx(&global_array_type);
    gt_ggc_mx(&global_struct_type);
    gt_ggc_mx(&global_union_type);
    gt_ggc_mx(&global_string_type);
    gt_ggc_mx(&global_callback_type);
    gt_ggc_mx(&global_user_struct_type);
    gt_ggc_mx(&global_lang_struct_type);
    
    /* Additional test: Process through ggc_type_tab if accessible */
#ifdef ggc_type_tab
    for (int i = 0; ggc_type_tab[i]; i++) {
        gt_ggc_mx(ggc_type_tab[i]);
    }
#endif
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
