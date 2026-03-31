/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC internal types to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Function to force type processing through gengtype */
static void force_gengtype_processing(tree type) {
    /* Use GTY marker to force type through gengtype system */
    GTY((user)) tree processed_type = type;
    (void)processed_type; /* Suppress unused warning */
}

/* Main test function */
void test_gengtype_categorization(void) {
    /* TYPE_SCALAR cases */
    force_gengtype_processing(integer_type_node);
    force_gengtype_processing(char_type_node);
    force_gengtype_processing(boolean_type_node);
    force_gengtype_processing(void_type_node);
    
    /* TYPE_POINTER cases */
    tree int_ptr_type = build_pointer_type(integer_type_node);
    force_gengtype_processing(int_ptr_type);
    
    tree char_ptr_type = build_pointer_type(char_type_node);
    force_gengtype_processing(char_ptr_type);
    
    /* TYPE_STRING case (pointer to char) */
    /* In GCC, string type is typically represented as pointer to char */
    tree string_type = build_pointer_type(char_type_node);
    force_gengtype_processing(string_type);
    
    /* TYPE_ARRAY cases */
    tree array_type_1d = build_array_type(integer_type_node, NULL_TREE);
    force_gengtype_processing(array_type_1d);
    
    /* Create array with bounds */
    tree index_type = build_index_type(size_int(10));
    tree array_type_bounded = build_array_type(char_type_node, index_type);
    force_gengtype_processing(array_type_bounded);
    
    /* Multi-dimensional array */
    tree array_type_2d = build_array_type(array_type_1d, NULL_TREE);
    force_gengtype_processing(array_type_2d);
    
    /* TYPE_STRUCT cases */
    tree struct_type = make_node(RECORD_TYPE);
    tree struct_decl = build_decl(UNKNOWN_LOCATION, TYPE_DECL, 
                                  get_identifier("test_struct"), struct_type);
    DECL_CONTEXT(struct_decl) = struct_type;
    TYPE_NAME(struct_type) = struct_decl;
    TYPE_STUB_DECL(struct_type) = struct_decl;
    
    /* Add fields to the struct */
    tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("field1"), integer_type_node);
    tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("field2"), char_type_node);
    
    /* Chain fields together */
    DECL_CHAIN(field1) = field2;
    TYPE_FIELDS(struct_type) = field1;
    
    /* Set field contexts */
    DECL_CONTEXT(field1) = struct_type;
    DECL_CONTEXT(field2) = struct_type;
    
    /* Layout the struct */
    layout_type(struct_type);
    force_gengtype_processing(struct_type);
    
    /* TYPE_USER_STRUCT - mark with user flag */
    tree user_struct_type = make_node(RECORD_TYPE);
    tree user_struct_decl = build_decl(UNKNOWN_LOCATION, TYPE_DECL,
                                       get_identifier("user_struct"), user_struct_type);
    TYPE_NAME(user_struct_type) = user_struct_decl;
    
    /* Add a field */
    tree user_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("user_field"), integer_type_node);
    TYPE_FIELDS(user_struct_type) = user_field;
    DECL_CONTEXT(user_field) = user_struct_type;
    
    /* Mark as user struct - this depends on GCC version */
    #ifdef TYPE_USER_STRUCT
    TYPE_USER_STRUCT(user_struct_type) = 1;
    #endif
    
    layout_type(user_struct_type);
    force_gengtype_processing(user_struct_type);
    
    /* TYPE_UNION case */
    tree union_type = make_node(UNION_TYPE);
    tree union_decl = build_decl(UNKNOWN_LOCATION, TYPE_DECL,
                                 get_identifier("test_union"), union_type);
    TYPE_NAME(union_type) = union_decl;
    
    /* Add union fields */
    tree union_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                   get_identifier("int_field"), integer_type_node);
    tree union_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                   get_identifier("char_field"), char_type_node);
    
    DECL_CHAIN(union_field1) = union_field2;
    TYPE_FIELDS(union_type) = union_field1;
    DECL_CONTEXT(union_field1) = union_type;
    DECL_CONTEXT(union_field2) = union_type;
    
    layout_type(union_type);
    force_gengtype_processing(union_type);
    
    /* TYPE_CALLBACK case (function pointer) */
    /* Create a function type */
    tree func_type = build_function_type(integer_type_node, NULL_TREE);
    force_gengtype_processing(func_type);
    
    /* Create pointer to function type */
    tree func_ptr_type = build_pointer_type(func_type);
    force_gengtype_processing(func_ptr_type);
    
    /* Function type with arguments */
    tree arg_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
    tree func_type_with_args = build_function_type(integer_type_node, arg_list);
    force_gengtype_processing(func_type_with_args);
    
    /* TYPE_LANG_STRUCT case */
    tree lang_struct_type = make_node(RECORD_TYPE);
    tree lang_struct_decl = build_decl(UNKNOWN_LOCATION, TYPE_DECL,
                                       get_identifier("lang_struct"), lang_struct_type);
    TYPE_NAME(lang_struct_type) = lang_struct_decl;
    
    /* Add a field */
    tree lang_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("lang_field"), integer_type_node);
    TYPE_FIELDS(lang_struct_type) = lang_field;
    DECL_CONTEXT(lang_field) = lang_struct_type;
    
    /* Mark with language-specific flag if available */
    #ifdef TYPE_LANG_SPECIFIC
    /* Allocate and set lang-specific data */
    struct lang_type *lang_data = ggc_alloc<struct lang_type>();
    TYPE_LANG_SPECIFIC(lang_struct_type) = lang_data;
    #endif
    
    layout_type(lang_struct_type);
    force_gengtype_processing(lang_struct_type);
    
    /* Complex pointer types */
    tree ptr_to_struct = build_pointer_type(struct_type);
    force_gengtype_processing(ptr_to_struct);
    
    tree ptr_to_array = build_pointer_type(array_type_1d);
    force_gengtype_processing(ptr_to_array);
    
    /* Nested struct */
    tree nested_struct = make_node(RECORD_TYPE);
    tree nested_decl = build_decl(UNKNOWN_LOCATION, TYPE_DECL,
                                  get_identifier("nested_struct"), nested_struct);
    TYPE_NAME(nested_struct) = nested_decl;
    
    /* Add struct pointer field */
    tree struct_ptr_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                       get_identifier("struct_ptr"), ptr_to_struct);
    TYPE_FIELDS(nested_struct) = struct_ptr_field;
    DECL_CONTEXT(struct_ptr_field) = nested_struct;
    
    layout_type(nested_struct);
    force_gengtype_processing(nested_struct);
    
    /* Array of structs */
    tree array_of_structs = build_array_type(struct_type, NULL_TREE);
    force_gengtype_processing(array_of_structs);
    
    /* Void pointer */
    tree void_ptr_type = build_pointer_type(void_type_node);
    force_gengtype_processing(void_ptr_type);
    
    /* Const qualified types */
    tree const_int_type = build_qualified_type(integer_type_node, TYPE_QUAL_CONST);
    force_gengtype_processing(const_int_type);
    
    /* Volatile qualified types */
    tree volatile_int_type = build_qualified_type(integer_type_node, TYPE_QUAL_VOLATILE);
    force_gengtype_processing(volatile_int_type);
    
    /* Reference types (for C++) */
    #ifdef REFERENCE_TYPE
    tree ref_type = build_reference_type(integer_type_node);
    force_gengtype_processing(ref_type);
    #endif
    
    /* Method pointer types (for C++) */
    #ifdef METHOD_TYPE
    tree method_type = build_method_type(integer_type_node, NULL_TREE);
    force_gengtype_processing(method_type);
    #endif
}

/* Global variables with GTY markers to ensure processing */
typedef struct test_container {
    tree scalar_type;
    tree pointer_type;
    tree array_type;
    tree struct_type;
    tree union_type;
    tree func_type;
} test_container_t;

/* GTY-marked global to force type processing */
static GTY(()) test_container_t g_test_container;

/* Initialization function */
void init_test_container(void) {
    /* Recreate types and store in global container */
    g_test_container.scalar_type = integer_type_node;
    g_test_container.pointer_type = build_pointer_type(char_type_node);
    g_test_container.array_type = build_array_type(integer_type_node, NULL_TREE);
    
    /* Create and store a struct type */
    tree local_struct = make_node(RECORD_TYPE);
    tree local_struct_decl = build_decl(UNKNOWN_LOCATION, TYPE_DECL,
                                        get_identifier("global_struct"), local_struct);
    TYPE_NAME(local_struct) = local_struct_decl;
    
    tree local_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("data"), integer_type_node);
    TYPE_FIELDS(local_struct) = local_field;
    DECL_CONTEXT(local_field) = local_struct;
    
    layout_type(local_struct);
    g_test_container.struct_type = local_struct;
    
    /* Create and store a union type */
    tree local_union = make_node(UNION_TYPE);
    tree local_union_decl = build_decl(UNKNOWN_LOCATION, TYPE_DECL,
                                       get_identifier("global_union"), local_union);
    TYPE_NAME(local_union) = local_union_decl;
    
    tree union_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("value"), integer_type_node);
    TYPE_FIELDS(local_union) = union_field;
    DECL_CONTEXT(union_field) = local_union;
    
    layout_type(local_union);
    g_test_container.union_type = local_union;
    
    /* Create and store a function type */
    g_test_container.func_type = build_function_type(integer_type_node, NULL_TREE);
}

/* Main test entry point */
int main(void) {
    /* Initialize GCC internal structures if needed */
    #ifdef GCC_INITIALIZATION
    gcc_init();
    #endif
    
    /* Run the categorization test */
    test_gengtype_categorization();
    
    /* Initialize global container to force more processing */
    init_test_container();
    
    /* Additional type variations */
    /* Variable field count struct */
    for (int field_count = 1; field_count <= 5; field_count++) {
        tree var_struct = make_node(RECORD_TYPE);
        tree var_decl = build_decl(UNKNOWN_LOCATION, TYPE_DECL,
                                   get_identifier("var_struct"), var_struct);
        TYPE_NAME(var_struct) = var_decl;
        
        tree prev_field = NULL_TREE;
        for (int i = 0; i < field_count; i++) {
            char field_name[32];
            sprintf(field_name, "field%d", i);
            tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                    get_identifier(field_name), integer_type_node);
            DECL_CONTEXT(field) = var_struct;
            
            if (prev_field) {
                DECL_CHAIN(prev_field) = field;
            } else {
                TYPE_FIELDS(var_struct) = field;
            }
            prev_field = field;
        }
        
        layout_type(var_struct);
        force_gengtype_processing(var_struct);
    }
    
    /* Different array dimensions */
    for (int dim = 1; dim <= 3; dim++) {
        tree array_type = integer_type_node;
        for (int d = 0; d < dim; d++) {
            array_type = build_array_type(array_type, NULL_TREE);
        }
        force_gengtype_processing(array_type);
    }
    
    return 0;
}
