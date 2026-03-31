/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */
/* Compile with: gcc -I. -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "gengtype.h"
#include <stdio.h>

/* Forward declarations for helper functions */
static tree create_test_struct(int field_count);
static tree create_test_union(int field_count);
static tree create_test_array(tree element_type, int dimensions);
static tree create_test_callback(void);

/* Global variables with GTY annotations to force gengtype processing */
/* These will trigger categorization when processed by gengtype */

/* Scalar types */
static GTY(()) tree global_scalar_int = NULL_TREE;
static GTY(()) tree global_scalar_char = NULL_TREE;
static GTY(()) tree global_scalar_bool = NULL_TREE;

/* Pointer types */
static GTY(()) tree global_pointer_to_int = NULL_TREE;
static GTY(()) tree global_pointer_to_struct = NULL_TREE;

/* Array types */
static GTY(()) tree global_int_array = NULL_TREE;
static GTY(()) tree global_struct_array = NULL_TREE;
static GTY(()) tree global_multi_dim_array = NULL_TREE;

/* Struct types */
static GTY(()) tree global_simple_struct = NULL_TREE;
static GTY(()) tree global_complex_struct = NULL_TREE;

/* Union types */
static GTY(()) tree global_simple_union = NULL_TREE;
static GTY(()) tree global_complex_union = NULL_TREE;

/* String type (char pointer) */
static GTY(()) tree global_string_type = NULL_TREE;

/* Callback/function pointer types */
static GTY(()) tree global_callback_type = NULL_TREE;
static GTY(()) tree global_func_ptr_type = NULL_TREE;

/* Language-specific struct type */
#ifdef TYPE_LANG_STRUCT
static GTY(()) tree global_lang_struct = NULL_TREE;
#endif

/* User struct type */
#ifdef TYPE_USER_STRUCT
static GTY(()) tree global_user_struct = NULL_TREE;
#endif

/* Container struct that references all types */
struct GTY(()) type_container {
    tree scalar_int;
    tree scalar_char;
    tree scalar_bool;
    tree pointer_int;
    tree pointer_struct;
    tree array_int;
    tree array_struct;
    tree array_multi;
    tree struct_simple;
    tree struct_complex;
    tree union_simple;
    tree union_complex;
    tree string_type;
    tree callback;
    tree func_ptr;
#ifdef TYPE_LANG_STRUCT
    tree lang_struct;
#endif
#ifdef TYPE_USER_STRUCT
    tree user_struct;
#endif
};

static GTY(()) struct type_container *global_container = NULL;

/* Helper function to create a struct with variable field count */
static tree
create_test_struct(int field_count)
{
    tree struct_type = make_node(RECORD_TYPE);
    tree field_list = NULL_TREE;
    
    /* Set struct name */
    char name[64];
    snprintf(name, sizeof(name), "test_struct_%d", field_count);
    TYPE_NAME(struct_type) = get_identifier(name);
    
    /* Create fields based on field_count */
    for (int i = 0; i < field_count; i++) {
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("field"),
                               integer_type_node);
        DECL_CONTEXT(field) = struct_type;
        
        /* Chain fields together */
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(struct_type) = field_list;
    layout_type(struct_type);
    
    return struct_type;
}

/* Helper function to create a union with variable field count */
static tree
create_test_union(int field_count)
{
    tree union_type = make_node(UNION_TYPE);
    tree field_list = NULL_TREE;
    
    /* Set union name */
    char name[64];
    snprintf(name, sizeof(name), "test_union_%d", field_count);
    TYPE_NAME(union_type) = get_identifier(name);
    
    /* Create fields based on field_count */
    for (int i = 0; i < field_count; i++) {
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("field"),
                               integer_type_node);
        DECL_CONTEXT(field) = union_type;
        
        /* Chain fields together */
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(union_type) = field_list;
    layout_type(union_type);
    
    return union_type;
}

/* Helper function to create multi-dimensional arrays */
static tree
create_test_array(tree element_type, int dimensions)
{
    tree array_type = element_type;
    
    for (int i = 0; i < dimensions; i++) {
        /* Create array type with 10 elements per dimension */
        tree index_type = build_index_type(size_int(9));
        array_type = build_array_type(array_type, index_type);
    }
    
    return array_type;
}

/* Helper function to create callback/function pointer type */
static tree
create_test_callback(void)
{
    /* Create a function type returning int with no parameters */
    tree func_type = build_function_type_list(integer_type_node, NULL_TREE);
    
    /* Create pointer to function type */
    tree func_ptr_type = build_pointer_type(func_type);
    
    return func_ptr_type;
}

/* Main test function */
void
test_gengtype_categorization(void)
{
    printf("Starting gengtype type categorization test...\n");
    
    /* 1. SCALAR TYPES - will increment nb_scalar */
    global_scalar_int = integer_type_node;
    global_scalar_char = char_type_node;
    global_scalar_bool = boolean_type_node;
    
    /* Force processing of scalar types */
    gt_ggc_mx(global_scalar_int);
    gt_ggc_mx(global_scalar_char);
    gt_ggc_mx(global_scalar_bool);
    
    /* 2. POINTER TYPES - will increment nb_pointer */
    global_pointer_to_int = build_pointer_type(integer_type_node);
    global_pointer_to_struct = build_pointer_type(create_test_struct(2));
    
    gt_ggc_mx(global_pointer_to_int);
    gt_ggc_mx(global_pointer_to_struct);
    
    /* 3. ARRAY TYPES - will increment nb_array */
    global_int_array = create_test_array(integer_type_node, 1);
    global_struct_array = create_test_array(create_test_struct(1), 1);
    global_multi_dim_array = create_test_array(integer_type_node, 3);
    
    gt_ggc_mx(global_int_array);
    gt_ggc_mx(global_struct_array);
    gt_ggc_mx(global_multi_dim_array);
    
    /* 4. STRUCT TYPES - will increment nb_struct */
    global_simple_struct = create_test_struct(1);
    global_complex_struct = create_test_struct(5);
    
    gt_ggc_mx(global_simple_struct);
    gt_ggc_mx(global_complex_struct);
    
    /* 5. UNION TYPES - will increment nb_union */
    global_simple_union = create_test_union(1);
    global_complex_union = create_test_union(3);
    
    gt_ggc_mx(global_simple_union);
    gt_ggc_mx(global_complex_union);
    
    /* 6. STRING TYPE - will increment nb_string */
    /* String type is pointer to char */
    global_string_type = build_pointer_type(char_type_node);
    gt_ggc_mx(global_string_type);
    
    /* 7. CALLBACK/FUNCTION POINTER TYPES - will increment nb_callback */
    global_callback_type = create_test_callback();
    /* Also create a direct function type */
    tree func_type = build_function_type(integer_type_node, NULL_TREE);
    global_func_ptr_type = build_pointer_type(func_type);
    
    gt_ggc_mx(global_callback_type);
    gt_ggc_mx(global_func_ptr_type);
    
    /* 8. LANGUAGE-SPECIFIC STRUCT TYPE - will increment nb_lang_struct */
#ifdef TYPE_LANG_STRUCT
    global_lang_struct = create_test_struct(2);
    /* Mark as language-specific */
    SET_TYPE_LANG_SPECIFIC(global_lang_struct);
    gt_ggc_mx(global_lang_struct);
#endif
    
    /* 9. USER STRUCT TYPE - will increment nb_user_struct */
#ifdef TYPE_USER_STRUCT
    global_user_struct = create_test_struct(2);
    /* Mark as user struct - implementation depends on GCC version */
    /* This might require setting specific flags or using TYPE_USER_ALIGN */
    TYPE_USER_ALIGN(global_user_struct) = 1;
    gt_ggc_mx(global_user_struct);
#endif
    
    /* 10. Create a container that references all types */
    global_container = ggc_alloc<type_container>();
    global_container->scalar_int = global_scalar_int;
    global_container->scalar_char = global_scalar_char;
    global_container->scalar_bool = global_scalar_bool;
    global_container->pointer_int = global_pointer_to_int;
    global_container->pointer_struct = global_pointer_to_struct;
    global_container->array_int = global_int_array;
    global_container->array_struct = global_struct_array;
    global_container->array_multi = global_multi_dim_array;
    global_container->struct_simple = global_simple_struct;
    global_container->struct_complex = global_complex_struct;
    global_container->union_simple = global_simple_union;
    global_container->union_complex = global_complex_union;
    global_container->string_type = global_string_type;
    global_container->callback = global_callback_type;
    global_container->func_ptr = global_func_ptr_type;
#ifdef TYPE_LANG_STRUCT
    global_container->lang_struct = global_lang_struct;
#endif
#ifdef TYPE_USER_STRUCT
    global_container->user_struct = global_user_struct;
#endif
    
    /* Force processing of the entire container */
    gt_ggc_mx(global_container);
    
    /* Additional processing to ensure all types are categorized */
    
    /* Process types in a loop with different configurations */
    for (int i = 1; i <= 3; i++) {
        /* Create structs with varying field counts */
        tree var_struct = create_test_struct(i * 2);
        gt_ggc_mx(var_struct);
        
        /* Create unions with varying field counts */
        tree var_union = create_test_union(i);
        gt_ggc_mx(var_union);
        
        /* Create arrays with varying dimensions */
        tree var_array = create_test_array(integer_type_node, i);
        gt_ggc_mx(var_array);
        
        /* Create pointers to different types */
        tree var_pointer = build_pointer_type(var_struct);
        gt_ggc_mx(var_pointer);
    }
    
    /* Test edge cases */
    
    /* Void pointer */
    tree void_ptr = build_pointer_type(void_type_node);
    gt_ggc_mx(void_ptr);
    
    /* Const pointer */
    tree const_int_type = build_qualified_type(integer_type_node, TYPE_QUAL_CONST);
    tree const_ptr = build_pointer_type(const_int_type);
    gt_ggc_mx(const_ptr);
    
    /* Array of pointers */
    tree ptr_array_type = build_array_type(build_pointer_type(integer_type_node),
                                          build_index_type(size_int(4)));
    gt_ggc_mx(ptr_array_type);
    
    /* Struct containing arrays */
    tree struct_with_array = make_node(RECORD_TYPE);
    TYPE_NAME(struct_with_array) = get_identifier("struct_with_array");
    
    tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("data"),
                                 ptr_array_type);
    DECL_CONTEXT(array_field) = struct_with_array;
    TYPE_FIELDS(struct_with_array) = array_field;
    layout_type(struct_with_array);
    
    gt_ggc_mx(struct_with_array);
    
    printf("Gengtype type categorization test completed.\n");
    
    /* Print some debug info if needed */
#ifdef DEBUG_GENGTYPE
    debug_tree(global_simple_struct);
    debug_tree(global_callback_type);
#endif
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void) {
    /* Initialize GCC's internal structures if needed */
    test_gengtype_categorization();
    return 0;
}
#endif

/* Plugin entry point if compiled as a plugin */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
    test_gengtype_categorization();
    return 0;
}
#endif
