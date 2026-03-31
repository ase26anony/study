/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Forward declarations */
void test_gengtype_categorization(void);

/* Global variables with GTY annotations to force type processing */
/* These will be processed by gengtype during compilation */

/* Scalar type */
static GTY(()) tree scalar_var = NULL_TREE;

/* Pointer type */
static GTY(()) tree pointer_var = NULL_TREE;

/* Array type */
static GTY(()) tree array_var = NULL_TREE;

/* Struct type */
static GTY(()) tree struct_var = NULL_TREE;

/* Union type */
static GTY(()) tree union_var = NULL_TREE;

/* String type (char pointer) */
static GTY(()) tree string_var = NULL_TREE;

/* Callback type (function pointer) */
static GTY(()) tree callback_var = NULL_TREE;

/* Complex struct with lang-specific data */
static GTY(()) tree lang_struct_var = NULL_TREE;

/* User-defined struct type */
static GTY(()) tree user_struct_var = NULL_TREE;

/* Helper function to create and register types */
void create_and_register_types(void)
{
    /* TYPE_SCALAR: Basic scalar types */
    scalar_var = integer_type_node;
    gt_ggc_mx (scalar_var);
    
    /* Also test other scalar types */
    gt_ggc_mx (char_type_node);
    gt_ggc_mx (boolean_type_node);
    gt_ggc_mx (size_type_node);
    
    /* TYPE_POINTER: Create pointer types */
    tree int_ptr_type = build_pointer_type(integer_type_node);
    pointer_var = build_int_cst(int_ptr_type, 0);
    gt_ggc_mx (pointer_var);
    
    /* TYPE_ARRAY: Create array types */
    tree array_type = build_array_type(integer_type_node, 
                                      build_index_type(size_int(__FIELD_COUNT__)));
    array_var = build_int_cst(array_type, 0);
    gt_ggc_mx (array_var);
    
    /* Multi-dimensional array */
    tree md_array_type = build_array_type(array_type,
                                         build_index_type(size_int(5)));
    gt_ggc_mx (md_array_type);
    
    /* TYPE_STRUCT: Create record types */
    tree struct_type = make_node(RECORD_TYPE);
    tree field_decl;
    
    /* Add varying number of fields based on placeholder */
    for (int i = 0; i < __FIELD_COUNT__; i++) {
        field_decl = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("field"),
                               integer_type_node);
        DECL_CHAIN(field_decl) = TYPE_FIELDS(struct_type);
        TYPE_FIELDS(struct_type) = field_decl;
    }
    
    /* Finish struct type */
    layout_type(struct_type);
    struct_var = build_int_cst(struct_type, 0);
    gt_ggc_mx (struct_var);
    
    /* TYPE_UNION: Create union type */
    tree union_type = make_node(UNION_TYPE);
    tree union_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("u_field1"),
                                  integer_type_node);
    tree union_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("u_field2"),
                                  char_type_node);
    
    DECL_CHAIN(union_field2) = union_field1;
    TYPE_FIELDS(union_type) = union_field2;
    layout_type(union_type);
    
    union_var = build_int_cst(union_type, 0);
    gt_ggc_mx (union_var);
    
    /* TYPE_STRING: String type (char pointer) */
    tree char_ptr_type = build_pointer_type(char_type_node);
    string_var = build_string_literal(10, "test string");
    gt_ggc_mx (string_var);
    
    /* TYPE_CALLBACK: Function pointer type */
    tree func_type = build_function_type_list(void_type_node,
                                             integer_type_node,
                                             NULL_TREE);
    tree func_ptr_type = build_pointer_type(func_type);
    callback_var = build_int_cst(func_ptr_type, 0);
    gt_ggc_mx (callback_var);
    
    /* TYPE_LANG_STRUCT: Struct with language-specific data */
    tree lang_struct_type = make_node(RECORD_TYPE);
    
    /* Add some fields */
    tree lang_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("lang_field"),
                                integer_type_node);
    TYPE_FIELDS(lang_struct_type) = lang_field;
    
    /* Mark as language-specific */
    SET_TYPE_LANG_SPECIFIC(lang_struct_type);
    layout_type(lang_struct_type);
    
    lang_struct_var = build_int_cst(lang_struct_type, 0);
    gt_ggc_mx (lang_struct_var);
    
    /* TYPE_USER_STRUCT: User-defined struct type */
    tree user_struct_type = make_node(RECORD_TYPE);
    
    /* Create a struct with special attributes */
    tree user_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("user_field1"),
                                 integer_type_node);
    tree user_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("user_field2"),
                                 char_type_node);
    
    DECL_CHAIN(user_field2) = user_field1;
    TYPE_FIELDS(user_struct_type) = user_field2;
    
    /* Mark as user struct - using TYPE_USER_ALIGN as a proxy */
    TYPE_USER_ALIGN(user_struct_type) = 1;
    layout_type(user_struct_type);
    
    user_struct_var = build_int_cst(user_struct_type, 0);
    gt_ggc_mx (user_struct_var);
    
    /* Test TYPE_UNDEFINED by creating incomplete types */
    tree incomplete_struct = make_node(RECORD_TYPE);
    gt_ggc_mx (incomplete_struct);
    
    /* Test various type combinations */
    tree ptr_to_struct = build_pointer_type(struct_type);
    gt_ggc_mx (ptr_to_struct);
    
    tree array_of_pointers = build_array_type(ptr_to_struct,
                                             build_index_type(size_int(3)));
    gt_ggc_mx (array_of_pointers);
    
    /* Struct containing arrays and pointers */
    tree complex_struct = make_node(RECORD_TYPE);
    tree complex_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                    get_identifier("data"),
                                    array_type);
    tree complex_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                    get_identifier("next"),
                                    ptr_to_struct);
    
    DECL_CHAIN(complex_field2) = complex_field1;
    TYPE_FIELDS(complex_struct) = complex_field2;
    layout_type(complex_struct);
    gt_ggc_mx (complex_struct);
}

/* Main test function */
void test_gengtype_categorization(void)
{
    /* Create types multiple times with different configurations */
    for (int variant = 0; variant < 3; variant++) {
        /* Use different field counts to create variability */
        __FIELD_COUNT__ = 1 + variant * 2;
        
        create_and_register_types();
        
        /* Force garbage collection to process types */
        ggc_collect();
    }
    
    /* Additional edge cases */
    
    /* Void pointer */
    tree void_ptr = build_pointer_type(void_type_node);
    gt_ggc_mx (void_ptr);
    
    /* Const qualified type */
    tree const_type = build_qualified_type(integer_type_node, TYPE_QUAL_CONST);
    gt_ggc_mx (const_type);
    
    /* Reference type (for C++) */
#ifdef ENABLE_TREE_CHECKING
    tree ref_type = build_reference_type(integer_type_node);
    gt_ggc_mx (ref_type);
#endif
    
    /* Method pointer type (for C++) */
#ifdef ENABLE_TREE_CHECKING
    tree method_type = build_method_type_directly(void_type_node,
                                                 integer_type_node,
                                                 NULL_TREE);
    gt_ggc_mx (method_type);
#endif
    
    /* Vector type */
    tree vector_type = build_vector_type(integer_type_node, 4);
    gt_ggc_mx (vector_type);
}

/* Test driver for standalone compilation */
#ifdef TEST_STANDALONE
int main(void)
{
    /* Initialize GCC runtime if needed */
    test_gengtype_categorization();
    return 0;
}
#endif

/* Plugin entry point for GCC plugin compilation */
#ifdef PLUGIN
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    test_gengtype_categorization();
    return 0;
}
#endif
