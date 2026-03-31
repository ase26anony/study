/* test_gengtype_categorization.c - Comprehensive test for GCC gengtype type categorization */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "ggc.h"

/* Helper macro to ensure types are processed by GC system */
#define PROCESS_TYPE(t) do { \
    gt_ggc_mx(t); \
    gt_pch_nx(t); \
} while(0)

/* Variability placeholders - can be mutated across test cases */
#ifndef FIELD_COUNT
#define FIELD_COUNT 3
#endif

#ifndef ARRAY_DIM
#define ARRAY_DIM 5
#endif

#ifndef STRUCT_NAME
#define STRUCT_NAME "test_struct"
#endif

/* Global variables with GTY annotations to force type processing */
static GTY(()) tree scalar_types[5];
static GTY(()) tree pointer_types[4];
static GTY(()) tree array_types[3];
static GTY(()) tree struct_types[2];
static GTY(()) tree union_types[2];
static GTY(()) tree callback_types[2];

/* Function to create and register various type categories */
void test_gengtype_categorization(void)
{
    /* 1. SCALAR TYPES (TYPE_SCALAR) */
    scalar_types[0] = integer_type_node;      /* int */
    scalar_types[1] = char_type_node;         /* char */
    scalar_types[2] = boolean_type_node;      /* bool */
    scalar_types[3] = size_type_node;         /* size_t */
    scalar_types[4] = ptrdiff_type_node;      /* ptrdiff_t */
    
    for (int i = 0; i < 5; i++) {
        PROCESS_TYPE(scalar_types[i]);
    }
    
    /* 2. POINTER TYPES (TYPE_POINTER) */
    pointer_types[0] = build_pointer_type(integer_type_node);
    pointer_types[1] = build_pointer_type(char_type_node);
    pointer_types[2] = build_pointer_type(void_type_node);
    pointer_types[3] = ptr_type_node;  /* Generic pointer type */
    
    for (int i = 0; i < 4; i++) {
        PROCESS_TYPE(pointer_types[i]);
    }
    
    /* 3. ARRAY TYPES (TYPE_ARRAY) */
    tree index_type = build_index_type(size_int(ARRAY_DIM - 1));
    array_types[0] = build_array_type(integer_type_node, index_type);
    array_types[1] = build_array_type(char_type_node, index_type);
    array_types[2] = build_array_type_nelts(integer_type_node, ARRAY_DIM);
    
    for (int i = 0; i < 3; i++) {
        PROCESS_TYPE(array_types[i]);
    }
    
    /* 4. STRUCT TYPES (TYPE_STRUCT) */
    tree struct_type = make_node(RECORD_TYPE);
    tree union_type = make_node(UNION_TYPE);
    
    /* Create fields for struct */
    tree field_list = NULL_TREE;
    for (int i = 0; i < FIELD_COUNT; i++) {
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("field"), 
                               (i % 2 == 0) ? integer_type_node : char_type_node);
        DECL_CHAIN(field) = field_list;
        field_list = field;
    }
    TYPE_FIELDS(struct_type) = field_list;
    TYPE_NAME(struct_type) = get_identifier(STRUCT_NAME);
    layout_type(struct_type);
    
    /* 5. UNION TYPES (TYPE_UNION) */
    tree union_field_list = NULL_TREE;
    for (int i = 0; i < 2; i++) {
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("union_field"),
                               (i == 0) ? integer_type_node : char_type_node);
        DECL_CHAIN(field) = union_field_list;
        union_field_list = field;
    }
    TYPE_FIELDS(union_type) = union_field_list;
    layout_type(union_type);
    
    struct_types[0] = struct_type;
    struct_types[1] = union_type;  /* Also test union as struct-like */
    union_types[0] = union_type;
    
    PROCESS_TYPE(struct_type);
    PROCESS_TYPE(union_type);
    
    /* 6. STRING TYPE (TYPE_STRING) */
    /* In GCC, string type is pointer to char */
    tree string_type = build_pointer_type(char_type_node);
    PROCESS_TYPE(string_type);
    
    /* 7. CALLBACK TYPES (TYPE_CALLBACK) - function pointers */
    tree func_type = build_function_type_list(void_type_node, 
                                             integer_type_node, 
                                             char_type_node, 
                                             NULL_TREE);
    tree func_ptr_type = build_pointer_type(func_type);
    callback_types[0] = func_type;
    callback_types[1] = func_ptr_type;
    
    PROCESS_TYPE(func_type);
    PROCESS_TYPE(func_ptr_type);
    
    /* 8. USER STRUCT (TYPE_USER_STRUCT) */
    /* Mark struct as user-defined with TYPE_LANG_FLAG */
    tree user_struct = make_node(RECORD_TYPE);
    TYPE_LANG_FLAG(user_struct) = 1;
    tree user_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("user_field"),
                                integer_type_node);
    TYPE_FIELDS(user_struct) = user_field;
    layout_type(user_struct);
    PROCESS_TYPE(user_struct);
    
    /* 9. LANG STRUCT (TYPE_LANG_STRUCT) */
    /* Create language-specific struct with TYPE_LANG_SPECIFIC */
    tree lang_struct = make_node(RECORD_TYPE);
#ifdef TYPE_LANG_SPECIFIC
    SET_TYPE_LANG_SPECIFIC(lang_struct, ggc_alloc<tree_lang_specific>());
#endif
    tree lang_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("lang_field"),
                                char_type_node);
    TYPE_FIELDS(lang_struct) = lang_field;
    layout_type(lang_struct);
    PROCESS_TYPE(lang_struct);
    
    /* 10. Test TYPE_UNDEFINED - create incomplete type */
    tree undefined_type = make_node(RECORD_TYPE);
    /* Don't add fields or layout - keep it incomplete */
    PROCESS_TYPE(undefined_type);
    
    /* Additional complex types to ensure thorough coverage */
    
    /* Pointer to array */
    tree ptr_to_array = build_pointer_type(array_types[0]);
    PROCESS_TYPE(ptr_to_array);
    
    /* Array of pointers */
    tree array_of_ptrs = build_array_type(pointer_types[0], index_type);
    PROCESS_TYPE(array_of_ptrs);
    
    /* Struct containing pointers and arrays */
    tree complex_struct = make_node(RECORD_TYPE);
    tree complex_fields = NULL_TREE;
    
    tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("data"),
                            integer_type_node);
    tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("next"),
                            build_pointer_type(complex_struct));
    tree field3 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("buffer"),
                            array_types[1]);
    
    DECL_CHAIN(field1) = field2;
    DECL_CHAIN(field2) = field3;
    TYPE_FIELDS(complex_struct) = field1;
    layout_type(complex_struct);
    PROCESS_TYPE(complex_struct);
    
    /* Nested struct */
    tree nested_struct = make_node(RECORD_TYPE);
    tree inner_struct = make_node(RECORD_TYPE);
    
    tree inner_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("inner_data"),
                                 integer_type_node);
    TYPE_FIELDS(inner_struct) = inner_field;
    layout_type(inner_struct);
    
    tree outer_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("outer_data"),
                                  char_type_node);
    tree outer_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("inner"),
                                  inner_struct);
    
    DECL_CHAIN(outer_field1) = outer_field2;
    TYPE_FIELDS(nested_struct) = outer_field1;
    layout_type(nested_struct);
    PROCESS_TYPE(nested_struct);
    PROCESS_TYPE(inner_struct);
    
    /* Variably modified type (if supported) */
#ifdef VARIABLE_MODIFIED_TYPE
    tree vla_type = build_array_type(integer_type_node, NULL_TREE);
    PROCESS_TYPE(vla_type);
#endif
    
    /* Force GC collection to ensure all types are processed */
    ggc_collect();
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
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
