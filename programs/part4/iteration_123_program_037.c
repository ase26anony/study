/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* This test creates various GCC type nodes to trigger all type_enum categorization cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "stringpool.h"
#include "attribs.h"
#include "stor-layout.h"
#include "varasm.h"

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
static tree create_struct_with_fields(int field_count)
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

/* Helper to create a union with variable field count */
static tree create_union_with_fields(int field_count)
{
    tree union_type = make_node(UNION_TYPE);
    tree field_list = NULL_TREE;
    
    for (int i = 0; i < field_count; i++) {
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier_with_length("field", 5),
                               integer_type_node);
        DECL_CONTEXT(field) = union_type;
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(union_type) = field_list;
    layout_type(union_type);
    
    return union_type;
}

/* Create a function pointer type (callback) */
static tree create_callback_type(void)
{
    tree return_type = void_type_node;
    tree arg_types = NULL_TREE;
    
    /* Add some arguments to make it interesting */
    arg_types = tree_cons(NULL_TREE, integer_type_node, arg_types);
    arg_types = tree_cons(NULL_TREE, char_type_node, arg_types);
    
    tree func_type = build_function_type(return_type, arg_types);
    return build_pointer_type(func_type);
}

/* Main test function */
void test_gengtype_categorization(void)
{
    /* TYPE_SCALAR - various scalar types */
    global_scalar_type = integer_type_node;
    gt_ggc_mx(global_scalar_type);
    
    tree bool_type = boolean_type_node;
    gt_ggc_mx(bool_type);
    
    tree char_type = char_type_node;
    gt_ggc_mx(char_type);
    
    /* TYPE_POINTER */
    global_pointer_type = build_pointer_type(integer_type_node);
    gt_ggc_mx(global_pointer_type);
    
    /* TYPE_ARRAY - with variable dimensions */
    tree array_type1 = build_array_type(integer_type_node, 
                                       build_range_type(integer_type_node,
                                                       integer_zero_node,
                                                       build_int_cst(integer_type_node, 10)));
    gt_ggc_mx(array_type1);
    
    /* Multi-dimensional array */
    tree inner_array = build_array_type(char_type_node,
                                       build_range_type(integer_type_node,
                                                       integer_zero_node,
                                                       build_int_cst(integer_type_node, 20)));
    tree md_array = build_array_type(inner_array,
                                    build_range_type(integer_type_node,
                                                    integer_zero_node,
                                                    build_int_cst(integer_type_node, 5)));
    gt_ggc_mx(md_array);
    
    /* TYPE_STRUCT - with variable field count */
    global_struct_type = create_struct_with_fields(__FIELD_COUNT__);
    gt_ggc_mx(global_struct_type);
    
    /* Another struct with different layout */
    tree struct_type2 = make_node(RECORD_TYPE);
    tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("int_field"),
                            integer_type_node);
    tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("ptr_field"),
                            build_pointer_type(char_type_node));
    TYPE_FIELDS(struct_type2) = chainon(field1, field2);
    layout_type(struct_type2);
    gt_ggc_mx(struct_type2);
    
    /* TYPE_UNION */
    global_union_type = create_union_with_fields(__FIELD_COUNT__);
    gt_ggc_mx(global_union_type);
    
    /* TYPE_STRING - char* type */
    global_string_type = build_pointer_type(char_type_node);
    gt_ggc_mx(global_string_type);
    
    /* TYPE_CALLBACK - function pointer */
    global_callback_type = create_callback_type();
    gt_ggc_mx(global_callback_type);
    
    /* TYPE_USER_STRUCT - mark a struct as user-defined */
    tree user_struct = create_struct_with_fields(2);
#ifdef TYPE_LANG_FLAG_0
    TYPE_LANG_FLAG_0(user_struct) = 1;
#endif
    /* Alternative method: use language-specific type */
    SET_TYPE_LANG_SPECIFIC(user_struct, (struct lang_type *)1);
    global_user_struct_type = user_struct;
    gt_ggc_mx(global_user_struct_type);
    
    /* TYPE_LANG_STRUCT - language-specific struct */
    tree lang_struct = create_struct_with_fields(3);
#ifdef TYPE_LANG_FLAG_1
    TYPE_LANG_FLAG_1(lang_struct) = 1;
#endif
    /* Mark as language-specific through various means */
    TYPE_LANG_SPECIFIC(lang_struct) = (struct lang_type *)2;
    global_lang_struct_type = lang_struct;
    gt_ggc_mx(global_lang_struct_type);
    
    /* Process all types through gengtype machinery */
    /* Force processing by using them in GTY macros */
    GTY((skip)) tree *type_array[] = {
        &global_scalar_type,
        &global_pointer_type,
        &global_array_type,
        &global_struct_type,
        &global_union_type,
        &global_string_type,
        &global_callback_type,
        &global_user_struct_type,
        &global_lang_struct_type,
        NULL
    };
    
    /* Additional type variations for comprehensive coverage */
    switch (__TYPE_KIND__) {
        case 1:
            /* Test with nested structs */
            tree nested_struct = create_struct_with_fields(1);
            tree outer_struct = make_node(RECORD_TYPE);
            tree nested_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                          get_identifier("nested"),
                                          nested_struct);
            TYPE_FIELDS(outer_struct) = nested_field;
            layout_type(outer_struct);
            gt_ggc_mx(outer_struct);
            break;
            
        case 2:
            /* Test with pointer chains */
            tree ptr_to_ptr = build_pointer_type(build_pointer_type(integer_type_node));
            gt_ggc_mx(ptr_to_ptr);
            break;
            
        case 3:
            /* Test with array of structs */
            tree struct_for_array = create_struct_with_fields(2);
            tree array_of_struct = build_array_type(struct_for_array,
                                                   build_range_type(integer_type_node,
                                                                   integer_zero_node,
                                                                   build_int_cst(integer_type_node, 8)));
            gt_ggc_mx(array_of_struct);
            break;
    }
    
    /* Ensure all counters would be incremented by processing these types */
    /* The actual counter increments happen in gengtype.cc when these types
       are processed through the type categorization system */
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
    /* Initialize GCC internal structures if needed */
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
