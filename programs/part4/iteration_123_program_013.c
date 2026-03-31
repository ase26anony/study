/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* This test creates various GCC type nodes to ensure all type_enum categories are exercised */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "ggc.h"

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

/* Helper to create a struct with variable field count */
static tree create_struct_with_fields(int field_count)
{
    tree struct_type = make_node(RECORD_TYPE);
    tree field_list = NULL_TREE;
    
    for (int i = 0; i < field_count; i++) {
        char field_name[32];
        sprintf(field_name, "field_%d", i);
        
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

/* Helper to create a union with variable member count */
static tree create_union_with_members(int member_count)
{
    tree union_type = make_node(UNION_TYPE);
    tree member_list = NULL_TREE;
    
    for (int i = 0; i < member_count; i++) {
        char member_name[32];
        sprintf(member_name, "member_%d", i);
        
        tree member = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier(member_name),
                                (i % 2 == 0) ? integer_type_node : char_type_node);
        DECL_CONTEXT(member) = union_type;
        
        member_list = chainon(member_list, member);
    }
    
    TYPE_FIELDS(union_type) = member_list;
    layout_type(union_type);
    
    return union_type;
}

/* Create function pointer type (callback) */
static tree create_callback_type(void)
{
    /* Create a function type returning int with no arguments */
    tree return_type = integer_type_node;
    tree arg_types = NULL_TREE;
    tree func_type = build_function_type(return_type, arg_types);
    
    /* Create pointer to function */
    return build_pointer_type(func_type);
}

/* Create array type with variable dimensions */
static tree create_array_type(int dimensions, tree element_type)
{
    tree array_type = element_type;
    
    for (int i = 0; i < dimensions; i++) {
        /* Create array with unspecified bounds */
        tree index_type = build_index_type(size_int(10 + i));
        array_type = build_array_type(array_type, index_type);
    }
    
    return array_type;
}

/* Main test function */
void test_gengtype_categorization(void)
{
    /* 1. SCALAR TYPES */
    global_scalar_type = integer_type_node;
    gt_ggc_mx(global_scalar_type);
    
    /* Also test other scalar types */
    gt_ggc_mx(char_type_node);
    gt_ggc_mx(boolean_type_node);
    gt_ggc_mx(void_type_node);
    
    /* 2. POINTER TYPES */
    tree int_ptr = build_pointer_type(integer_type_node);
    global_pointer_type = int_ptr;
    gt_ggc_mx(global_pointer_type);
    
    /* Multiple pointer variations */
    tree char_ptr = build_pointer_type(char_type_node);
    gt_ggc_mx(char_ptr);
    
    tree ptr_to_ptr = build_pointer_type(int_ptr);
    gt_ggc_mx(ptr_to_ptr);
    
    /* 3. ARRAY TYPES */
    /* 1D array */
    tree index_type = build_index_type(size_int(10));
    tree int_array = build_array_type(integer_type_node, index_type);
    global_array_type = int_array;
    gt_ggc_mx(global_array_type);
    
    /* Multi-dimensional array */
    tree md_array = create_array_type(3, char_type_node);
    gt_ggc_mx(md_array);
    
    /* Variable length array */
    tree vla_type = build_array_type(integer_type_node, NULL_TREE);
    TYPE_DOMAIN(vla_type) = NULL_TREE;
    gt_ggc_mx(vla_type);
    
    /* 4. STRUCT TYPES */
    /* Struct with __FIELD_COUNT__ fields (placeholder) */
    int field_count = __FIELD_COUNT__;
    tree struct_type = create_struct_with_fields(field_count);
    global_struct_type = struct_type;
    gt_ggc_mx(global_struct_type);
    
    /* Struct with pointer member */
    tree struct_with_ptr = make_node(RECORD_TYPE);
    tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("data_ptr"),
                               int_ptr);
    DECL_CONTEXT(ptr_field) = struct_with_ptr;
    TYPE_FIELDS(struct_with_ptr) = ptr_field;
    layout_type(struct_with_ptr);
    gt_ggc_mx(struct_with_ptr);
    
    /* 5. UNION TYPES */
    /* Union with __FIELD_COUNT__ members (placeholder) */
    int member_count = __FIELD_COUNT__;
    tree union_type = create_union_with_members(member_count);
    global_union_type = union_type;
    gt_ggc_mx(global_union_type);
    
    /* 6. STRING TYPE (char pointer) */
    /* In GCC, string type is typically char* */
    global_string_type = build_pointer_type(char_type_node);
    gt_ggc_mx(global_string_type);
    
    /* Also test const char* */
    tree const_char_type = build_qualified_type(char_type_node, TYPE_QUAL_CONST);
    tree const_char_ptr = build_pointer_type(const_char_type);
    gt_ggc_mx(const_char_ptr);
    
    /* 7. CALLBACK TYPES (function pointers) */
    global_callback_type = create_callback_type();
    gt_ggc_mx(global_callback_type);
    
    /* Function pointer with arguments */
    tree arg_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
    arg_list = tree_cons(NULL_TREE, char_type_node, arg_list);
    tree func_with_args = build_function_type(integer_type_node, arg_list);
    tree func_ptr = build_pointer_type(func_with_args);
    gt_ggc_mx(func_ptr);
    
    /* 8. USER STRUCT TYPE */
    /* Mark a struct as user-defined */
    tree user_struct = create_struct_with_fields(2);
    
    /* Set user flag - method depends on GCC version */
#ifdef TYPE_USER_STRUCT
    TYPE_USER_STRUCT(user_struct) = 1;
#else
    /* Alternative: use lang-specific data */
    SET_TYPE_LANG_SPECIFIC(user_struct, (void *)1);
#endif
    
    global_user_struct_type = user_struct;
    gt_ggc_mx(global_user_struct_type);
    
    /* 9. LANG STRUCT TYPE */
    /* Create language-specific struct */
    tree lang_struct = make_node(RECORD_TYPE);
    
    /* Add lang-specific flag */
#ifdef TYPE_LANG_FLAG
    TYPE_LANG_FLAG(lang_struct) = 1;
#endif
    
    /* Set lang-specific data */
    SET_TYPE_LANG_SPECIFIC(lang_struct, (void *)0x1234);
    
    global_lang_struct_type = lang_struct;
    gt_ggc_mx(global_lang_struct_type);
    
    /* 10. Test type combinations */
    
    /* Struct containing array */
    tree struct_with_array = make_node(RECORD_TYPE);
    tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("array"),
                                 int_array);
    DECL_CONTEXT(array_field) = struct_with_array;
    TYPE_FIELDS(struct_with_array) = array_field;
    layout_type(struct_with_array);
    gt_ggc_mx(struct_with_array);
    
    /* Array of structs */
    tree array_of_structs = build_array_type(struct_type, index_type);
    gt_ggc_mx(array_of_structs);
    
    /* Pointer to array */
    tree ptr_to_array = build_pointer_type(int_array);
    gt_ggc_mx(ptr_to_array);
    
    /* Union containing callback */
    tree union_with_callback = make_node(UNION_TYPE);
    tree callback_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                    get_identifier("callback"),
                                    global_callback_type);
    DECL_CONTEXT(callback_field) = union_with_callback;
    TYPE_FIELDS(union_with_callback) = callback_field;
    layout_type(union_with_callback);
    gt_ggc_mx(union_with_callback);
    
    /* Force processing of all globals */
    gt_ggc_mx_CORE(tree)(&global_scalar_type);
    gt_ggc_mx_CORE(tree)(&global_pointer_type);
    gt_ggc_mx_CORE(tree)(&global_array_type);
    gt_ggc_mx_CORE(tree)(&global_struct_type);
    gt_ggc_mx_CORE(tree)(&global_union_type);
    gt_ggc_mx_CORE(tree)(&global_string_type);
    gt_ggc_mx_CORE(tree)(&global_callback_type);
    gt_ggc_mx_CORE(tree)(&global_user_struct_type);
    gt_ggc_mx_CORE(tree)(&global_lang_struct_type);
}

/* Entry point for standalone test */
#ifdef STANDALONE_TEST
int main(void)
{
    /* Initialize GCC environment if needed */
    test_gengtype_categorization();
    return 0;
}
#endif

/* Plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    test_gengtype_categorization();
    return 0;
}
#endif
