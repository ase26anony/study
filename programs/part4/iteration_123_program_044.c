/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC type nodes to trigger all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Forward declarations for test functions */
static void create_and_process_types(void);
static tree create_test_struct(int field_count);
static tree create_test_union(int field_count);
static tree create_test_array(tree element_type, int dimensions);
static tree create_test_callback(void);

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

/* Helper to mark a type as user-defined */
static void mark_as_user_struct(tree type)
{
#ifdef TYPE_LANG_FLAG_0
    TYPE_LANG_FLAG_0(type) = 1;
#endif
}

/* Helper to mark a type as language-specific */
static void mark_as_lang_struct(tree type)
{
#ifdef TYPE_LANG_SPECIFIC
    /* Create a dummy lang-specific structure */
    struct lang_type *lt = ggc_alloc<struct lang_type>();
    SET_TYPE_LANG_SPECIFIC(type, lt);
#endif
}

void test_gengtype_categorization(void)
{
    /* Create and process each type category */
    
    /* 1. SCALAR types */
    global_scalar_type = integer_type_node;
    gt_ggc_mx(global_scalar_type);
    
    /* Also test other scalar types */
    gt_ggc_mx(char_type_node);
    gt_ggc_mx(boolean_type_node);
    gt_ggc_mx(size_type_node);
    
    /* 2. POINTER types */
    global_pointer_type = build_pointer_type(integer_type_node);
    gt_ggc_mx(global_pointer_type);
    
    /* Test multiple pointer variations */
    tree ptr_to_char = build_pointer_type(char_type_node);
    tree ptr_to_ptr = build_pointer_type(global_pointer_type);
    gt_ggc_mx(ptr_to_char);
    gt_ggc_mx(ptr_to_ptr);
    
    /* 3. STRING type (pointer to char) */
    global_string_type = ptr_type_node;  /* This is char* */
    gt_ggc_mx(global_string_type);
    
    /* Alternative string type */
    tree string_type = build_pointer_type(char_type_node);
    gt_ggc_mx(string_type);
    
    /* 4. ARRAY types */
    /* Create 1D array */
    tree array_1d = build_array_type(integer_type_node, NULL_TREE);
    global_array_type = array_1d;
    gt_ggc_mx(global_array_type);
    
    /* Create multi-dimensional array */
    tree array_2d = create_test_array(integer_type_node, 2);
    gt_ggc_mx(array_2d);
    
    /* Create array with specified bounds */
    tree array_with_bounds = build_array_type_nelts(integer_type_node, 10);
    gt_ggc_mx(array_with_bounds);
    
    /* 5. STRUCT types */
    /* Create struct with __FIELD_COUNT__ fields (placeholder for mutation) */
    int field_count = 3;  /* Default, can be mutated */
    global_struct_type = create_test_struct(field_count);
    gt_ggc_mx(global_struct_type);
    
    /* Create empty struct */
    tree empty_struct = make_node(RECORD_TYPE);
    gt_ggc_mx(empty_struct);
    
    /* 6. UNION types */
    global_union_type = create_test_union(field_count);
    gt_ggc_mx(global_union_type);
    
    /* Create empty union */
    tree empty_union = make_node(UNION_TYPE);
    gt_ggc_mx(empty_union);
    
    /* 7. CALLBACK types (function pointers) */
    global_callback_type = create_test_callback();
    gt_ggc_mx(global_callback_type);
    
    /* Create various function types */
    tree func_type_void = build_function_type(void_type_node, NULL_TREE);
    tree func_ptr_void = build_pointer_type(func_type_void);
    gt_ggc_mx(func_ptr_void);
    
    /* Function with parameters */
    tree arg_list = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
    tree func_with_args = build_function_type(integer_type_node, arg_list);
    tree func_ptr_with_args = build_pointer_type(func_with_args);
    gt_ggc_mx(func_ptr_with_args);
    
    /* 8. USER STRUCT types */
    global_user_struct_type = create_test_struct(2);
    mark_as_user_struct(global_user_struct_type);
    gt_ggc_mx(global_user_struct_type);
    
    /* 9. LANG STRUCT types */
    global_lang_struct_type = create_test_struct(2);
    mark_as_lang_struct(global_lang_struct_type);
    gt_ggc_mx(global_lang_struct_type);
    
    /* Test TYPE_UNDEFINED by creating incomplete types */
    tree incomplete_struct = make_node(RECORD_TYPE);
    /* Don't finish it - leave it incomplete */
    gt_ggc_mx(incomplete_struct);
    
    /* Test nested/complex types to ensure thorough coverage */
    tree struct_with_array = create_test_struct(2);
    /* Add array field */
    tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL, 
                                  get_identifier("array_field"), 
                                  array_1d);
    TYPE_FIELDS(struct_with_array) = array_field;
    gt_ggc_mx(struct_with_array);
    
    /* Struct containing pointers */
    tree struct_with_pointers = create_test_struct(2);
    gt_ggc_mx(struct_with_pointers);
    
    /* Union with various types */
    tree complex_union = make_node(UNION_TYPE);
    tree union_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                   get_identifier("as_int"),
                                   integer_type_node);
    tree union_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                   get_identifier("as_ptr"),
                                   global_pointer_type);
    TYPE_FIELDS(complex_union) = chainon(union_field1, union_field2);
    gt_ggc_mx(complex_union);
}

/* Create a test struct with specified number of fields */
static tree create_test_struct(int field_count)
{
    tree struct_type = make_node(RECORD_TYPE);
    tree field_list = NULL_TREE;
    
    for (int i = 0; i < field_count; i++) {
        char field_name[32];
        sprintf(field_name, "field_%d", i);
        
        tree field_type;
        /* Vary field types for better coverage */
        switch (i % 4) {
            case 0: field_type = integer_type_node; break;
            case 1: field_type = char_type_node; break;
            case 2: field_type = build_pointer_type(integer_type_node); break;
            case 3: field_type = boolean_type_node; break;
            default: field_type = integer_type_node;
        }
        
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier(field_name),
                                field_type);
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(struct_type) = field_list;
    return struct_type;
}

/* Create a test union with specified number of fields */
static tree create_test_union(int field_count)
{
    tree union_type = make_node(UNION_TYPE);
    tree field_list = NULL_TREE;
    
    for (int i = 0; i < field_count; i++) {
        char field_name[32];
        sprintf(field_name, "u_field_%d", i);
        
        tree field_type;
        switch (i % 3) {
            case 0: field_type = integer_type_node; break;
            case 1: field_type = char_type_node; break;
            case 2: field_type = build_pointer_type(void_type_node); break;
            default: field_type = integer_type_node;
        }
        
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier(field_name),
                                field_type);
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(union_type) = field_list;
    return union_type;
}

/* Create a multi-dimensional array */
static tree create_test_array(tree element_type, int dimensions)
{
    tree array_type = element_type;
    
    for (int i = 0; i < dimensions; i++) {
        /* Create array type with unspecified bounds */
        array_type = build_array_type(array_type, NULL_TREE);
    }
    
    return array_type;
}

/* Create a callback (function pointer) type */
static tree create_test_callback(void)
{
    /* Create function type: int (*)(int, char*) */
    tree arg1 = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
    tree arg2 = tree_cons(NULL_TREE, 
                         build_pointer_type(char_type_node), 
                         NULL_TREE);
    tree arg_list = chainon(arg1, arg2);
    
    tree func_type = build_function_type(integer_type_node, arg_list);
    tree func_ptr = build_pointer_type(func_type);
    
    return func_ptr;
}

/* Main test entry point */
int main(void)
{
    /* Initialize GCC runtime if needed */
#ifdef GCC_MAIN_INIT
    gcc_init();
#endif
    
    test_gengtype_categorization();
    
    /* Force garbage collection to process all types */
    ggc_collect();
    
    return 0;
}
