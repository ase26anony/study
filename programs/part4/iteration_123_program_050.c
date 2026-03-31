/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "stringpool.h"
#include "attribs.h"

/* Forward declarations for helper functions */
static tree create_test_struct(int field_count);
static tree create_test_union(int field_count);
static tree create_test_array(tree element_type, int dimensions);
static tree create_function_pointer_type(void);

/* Global variables with GTY annotations to force gengtype processing */
static GTY(()) tree global_scalar_types[3];
static GTY(()) tree global_pointer_types[2];
static GTY(()) tree global_array_types[2];
static GTY(()) tree global_struct_type;
static GTY(()) tree global_union_type;
static GTY(()) tree global_string_type;
static GTY(()) tree global_callback_type;
static GTY(()) tree global_user_struct_type;
static GTY(()) tree global_lang_struct_type;

/* Helper to mark a type as user-defined */
static void mark_as_user_struct(tree type)
{
#ifdef TYPE_LANG_FLAG_0
    TYPE_LANG_FLAG_0(type) = 1;
#endif
    /* Alternative method if TYPE_LANG_FLAG_0 is not available */
    TYPE_USER_ALIGN(type) = 1;
}

/* Helper to mark a type as language-specific */
static void mark_as_lang_struct(tree type)
{
#ifdef TYPE_LANG_SPECIFIC
    if (!TYPE_LANG_SPECIFIC(type)) {
        struct lang_type *lt = ggc_alloc<struct lang_type>();
        TYPE_LANG_SPECIFIC(type) = lt;
    }
#endif
}

/* Create a struct type with variable number of fields */
static tree create_test_struct(int field_count)
{
    tree struct_type = make_node(RECORD_TYPE);
    tree field_list = NULL_TREE;
    
    /* Push a dummy binding contour for fields */
    pushlevel();
    
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
    TYPE_NAME(struct_type) = get_identifier("test_struct");
    
    /* Finish the struct */
    layout_type(struct_type);
    poplevel();
    
    return struct_type;
}

/* Create a union type with variable number of fields */
static tree create_test_union(int field_count)
{
    tree union_type = make_node(UNION_TYPE);
    tree field_list = NULL_TREE;
    
    pushlevel();
    
    for (int i = 0; i < field_count; i++) {
        char field_name[32];
        sprintf(field_name, "member_%d", i);
        
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier(field_name),
                               (i % 2 == 0) ? integer_type_node : char_type_node);
        DECL_CONTEXT(field) = union_type;
        
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(union_type) = field_list;
    TYPE_NAME(union_type) = get_identifier("test_union");
    
    layout_type(union_type);
    poplevel();
    
    return union_type;
}

/* Create an array type with specified dimensions */
static tree create_test_array(tree element_type, int dimensions)
{
    tree array_type = element_type;
    
    for (int i = 0; i < dimensions; i++) {
        /* Create array type with 10 elements in each dimension */
        tree index_type = build_index_type(size_int(9));
        array_type = build_array_type(array_type, index_type);
    }
    
    return array_type;
}

/* Create a function pointer type (callback) */
static tree create_function_pointer_type(void)
{
    /* Create a simple function type: int(void) */
    tree return_type = integer_type_node;
    tree arg_types = NULL_TREE;
    tree func_type = build_function_type(return_type, arg_types);
    
    /* Create pointer to function type */
    tree func_ptr_type = build_pointer_type(func_type);
    
    return func_ptr_type;
}

/* Main test function that creates all types and triggers gengtype processing */
void test_gengtype_categorization(void)
{
    /* 1. SCALAR TYPES - TYPE_SCALAR */
    global_scalar_types[0] = integer_type_node;      /* int */
    global_scalar_types[1] = char_type_node;         /* char */
    global_scalar_types[2] = boolean_type_node;      /* bool */
    
    /* Force processing of scalar types */
    gt_ggc_mx(global_scalar_types);
    
    /* 2. POINTER TYPES - TYPE_POINTER */
    global_pointer_types[0] = build_pointer_type(integer_type_node);
    global_pointer_types[1] = build_pointer_type(char_type_node);
    
    gt_ggc_mx(global_pointer_types);
    
    /* 3. ARRAY TYPES - TYPE_ARRAY */
    global_array_types[0] = create_test_array(integer_type_node, 1);  /* 1D array */
    global_array_types[1] = create_test_array(char_type_node, 2);     /* 2D array */
    
    gt_ggc_mx(global_array_types);
    
    /* 4. STRUCT TYPES - TYPE_STRUCT */
    global_struct_type = create_test_struct(__FIELD_COUNT__);
    gt_ggc_mx(&global_struct_type);
    
    /* 5. UNION TYPES - TYPE_UNION */
    global_union_type = create_test_union(__FIELD_COUNT__);
    gt_ggc_mx(&global_union_type);
    
    /* 6. STRING TYPE - TYPE_STRING */
    /* In GCC, string type is typically pointer to char */
    global_string_type = build_pointer_type(char_type_node);
    gt_ggc_mx(&global_string_type);
    
    /* 7. CALLBACK TYPE - TYPE_CALLBACK */
    global_callback_type = create_function_pointer_type();
    gt_ggc_mx(&global_callback_type);
    
    /* 8. USER STRUCT - TYPE_USER_STRUCT */
    global_user_struct_type = create_test_struct(2);
    mark_as_user_struct(global_user_struct_type);
    gt_ggc_mx(&global_user_struct_type);
    
    /* 9. LANG STRUCT - TYPE_LANG_STRUCT */
    global_lang_struct_type = create_test_struct(3);
    mark_as_lang_struct(global_lang_struct_type);
    gt_ggc_mx(&global_lang_struct_type);
    
    /* 10. Additional pointer variations for coverage */
    tree void_ptr = build_pointer_type(void_type_node);
    tree const_ptr = build_pointer_type(build_qualified_type(integer_type_node, TYPE_QUAL_CONST));
    
    /* Process through GTY macros */
    gt_ggc_mx(void_ptr);
    gt_ggc_mx(const_ptr);
    
    /* Create and process an undefined type node */
    tree undefined_type = make_node(ERROR_MARK);
    if (TREE_CODE(undefined_type) == ERROR_MARK) {
        /* This should trigger TYPE_UNDEFINED case */
        gt_ggc_mx(undefined_type);
    }
    
    /* Create a complex struct with nested types */
    tree complex_struct = make_node(RECORD_TYPE);
    pushlevel();
    
    /* Add various field types */
    tree int_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("int_field"),
                               integer_type_node);
    DECL_CONTEXT(int_field) = complex_struct;
    
    tree ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("ptr_field"),
                               build_pointer_type(char_type_node));
    DECL_CONTEXT(ptr_field) = complex_struct;
    
    tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("array_field"),
                                 create_test_array(integer_type_node, 1));
    DECL_CONTEXT(array_field) = complex_struct;
    
    TYPE_FIELDS(complex_struct) = chainon(int_field, chainon(ptr_field, array_field));
    TYPE_NAME(complex_struct) = get_identifier("complex_struct");
    
    layout_type(complex_struct);
    poplevel();
    
    gt_ggc_mx(complex_struct);
    
    /* Create a type with __TYPE_KIND__ variations */
    tree variant_type;
    switch (__TYPE_KIND__) {
        case 0:
            variant_type = create_test_struct(1);
            break;
        case 1:
            variant_type = create_test_union(1);
            break;
        case 2:
            variant_type = create_test_array(integer_type_node, 3);
            break;
        default:
            variant_type = build_pointer_type(integer_type_node);
            break;
    }
    gt_ggc_mx(variant_type);
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
    /* Initialize GCC environment */
    init_tree();
    
    /* Run the categorization test */
    test_gengtype_categorization();
    
    return 0;
}
#endif

/* Alternative: Plugin entry point */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    test_gengtype_categorization();
    return 0;
}
#endif
