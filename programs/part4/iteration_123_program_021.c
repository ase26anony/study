/* test_gengtype_categorization.c - Comprehensive test for GCC's gengtype type categorization */
/* This test must be compiled as part of GCC's build system */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "ggc.h"

/* Forward declarations for helper functions */
static tree create_test_struct(int field_count);
static tree create_test_union(int field_count);
static tree create_test_array(tree element_type, int dimensions);
static tree create_test_callback(void);

/* Global variables with GTY annotations to force gengtype processing */
/* These will trigger the categorization logic when processed by gengtype */

/* TYPE_SCALAR */
static GTY(()) tree scalar_var = NULL_TREE;

/* TYPE_POINTER */
static GTY(()) tree pointer_var = NULL_TREE;

/* TYPE_ARRAY */
static GTY(()) tree array_var = NULL_TREE;

/* TYPE_STRUCT */
static GTY(()) tree struct_var = NULL_TREE;

/* TYPE_UNION */
static GTY(()) tree union_var = NULL_TREE;

/* TYPE_STRING - char* type */
static GTY(()) const char *string_var = NULL;

/* TYPE_CALLBACK - function pointer */
typedef void (*callback_func)(void);
static GTY(()) callback_func callback_var = NULL;

/* Complex nested structure to test multiple categories */
struct GTY(()) complex_nested {
    int scalar_field;                    /* TYPE_SCALAR */
    tree GTY((tag("0"))) tree_ptr;       /* TYPE_POINTER */
    int GTY((length("array_len"))) *array_field; /* TYPE_ARRAY */
    struct complex_nested *next;         /* TYPE_POINTER to struct */
};

static GTY(()) struct complex_nested *nested_var = NULL;

/* User-defined struct type - TYPE_USER_STRUCT */
struct GTY((user)) user_defined_struct {
    int id;
    char *name;
};

static GTY(()) struct user_defined_struct *user_struct_var = NULL;

/* Language-specific struct - TYPE_LANG_STRUCT */
#ifdef TYPE_LANG_STRUCT
struct GTY((desc("%1"))) lang_specific_struct {
    int lang_specific_data;
    tree lang_tree_node;
};

static GTY(()) struct lang_specific_struct *lang_struct_var = NULL;
#endif

/* Helper function to create a struct with variable field count */
static tree
create_test_struct(int field_count)
{
    tree struct_type = make_node(RECORD_TYPE);
    tree field_list = NULL_TREE;
    
    /* Push a dummy binding contour */
    pushlevel();
    
    for (int i = 0; i < field_count; i++) {
        char field_name[32];
        sprintf(field_name, "field_%d", i);
        
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier(field_name),
                               integer_type_node);
        
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(struct_type) = field_list;
    layout_type(struct_type);
    
    /* Pop the binding contour */
    poplevel();
    
    return struct_type;
}

/* Helper function to create a union with variable field count */
static tree
create_test_union(int field_count)
{
    tree union_type = make_node(UNION_TYPE);
    tree field_list = NULL_TREE;
    
    pushlevel();
    
    for (int i = 0; i < field_count; i++) {
        char field_name[32];
        sprintf(field_name, "union_field_%d", i);
        
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier(field_name),
                               (i % 2 == 0) ? integer_type_node : char_type_node);
        
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(union_type) = field_list;
    layout_type(union_type);
    
    poplevel();
    
    return union_type;
}

/* Helper function to create multi-dimensional array */
static tree
create_test_array(tree element_type, int dimensions)
{
    tree array_type = element_type;
    
    for (int i = 0; i < dimensions; i++) {
        /* Create array type with 10 elements in each dimension */
        array_type = build_array_type_nelts(array_type, 10);
    }
    
    return array_type;
}

/* Helper function to create callback (function pointer) type */
static tree
create_test_callback(void)
{
    /* Create a function type returning void with no parameters */
    tree return_type = void_type_node;
    tree arg_types = NULL_TREE;
    
    tree func_type = build_function_type(return_type, arg_types);
    tree func_ptr_type = build_pointer_type(func_type);
    
    return func_ptr_type;
}

/* Main test function that creates and processes all type categories */
void
test_gengtype_categorization(void)
{
    /* TYPE_SCALAR - basic scalar types */
    scalar_var = integer_type_node;      /* int */
    gt_ggc_mx(scalar_var);
    
    scalar_var = char_type_node;         /* char */
    gt_ggc_mx(scalar_var);
    
    scalar_var = boolean_type_node;      /* bool */
    gt_ggc_mx(scalar_var);
    
    /* TYPE_POINTER */
    pointer_var = build_pointer_type(integer_type_node);
    gt_ggc_mx(pointer_var);
    
    /* TYPE_ARRAY - test different dimensions */
    tree array_1d = create_test_array(integer_type_node, 1);
    tree array_2d = create_test_array(char_type_node, 2);
    tree array_3d = create_test_array(pointer_var, 3);
    
    array_var = array_1d;
    gt_ggc_mx(array_var);
    
    array_var = array_2d;
    gt_ggc_mx(array_var);
    
    array_var = array_3d;
    gt_ggc_mx(array_var);
    
    /* TYPE_STRUCT - test with different field counts */
    tree struct_1 = create_test_struct(1);    /* 1 field */
    tree struct_5 = create_test_struct(5);    /* 5 fields */
    tree struct_10 = create_test_struct(10);  /* 10 fields */
    
    struct_var = struct_1;
    gt_ggc_mx(struct_var);
    
    struct_var = struct_5;
    gt_ggc_mx(struct_var);
    
    struct_var = struct_10;
    gt_ggc_mx(struct_var);
    
    /* TYPE_UNION */
    tree union_2 = create_test_union(2);      /* 2 fields */
    tree union_7 = create_test_union(7);      /* 7 fields */
    
    union_var = union_2;
    gt_ggc_mx(union_var);
    
    union_var = union_7;
    gt_ggc_mx(union_var);
    
    /* TYPE_STRING - char* type */
    string_var = "test_string";
    /* Note: char* is automatically categorized as TYPE_STRING by gengtype */
    
    /* TYPE_CALLBACK - function pointer */
    tree callback_type = create_test_callback();
    /* We need to create a variable of this type to trigger processing */
    tree callback_decl = build_decl(UNKNOWN_LOCATION, VAR_DECL,
                                   get_identifier("test_callback"),
                                   callback_type);
    DECL_EXTERNAL(callback_decl) = 1;
    gt_ggc_mx(callback_decl);
    
    /* TYPE_USER_STRUCT - user-defined struct with GTY((user)) */
    user_struct_var = (struct user_defined_struct *)ggc_alloc(sizeof(struct user_defined_struct));
    user_struct_var->id = 42;
    user_struct_var->name = "user_struct_test";
    gt_ggc_mx(user_struct_var);
    
    /* TYPE_LANG_STRUCT - language-specific struct */
#ifdef TYPE_LANG_STRUCT
    lang_struct_var = (struct lang_specific_struct *)ggc_alloc(sizeof(struct lang_specific_struct));
    lang_struct_var->lang_specific_data = 100;
    lang_struct_var->lang_tree_node = integer_type_node;
    gt_ggc_mx(lang_struct_var);
#endif
    
    /* Complex nested structure to test multiple interactions */
    nested_var = (struct complex_nested *)ggc_alloc(sizeof(struct complex_nested));
    nested_var->scalar_field = 1;
    nested_var->tree_ptr = integer_type_node;
    nested_var->array_field = (int *)ggc_alloc(10 * sizeof(int));
    nested_var->next = (struct complex_nested *)ggc_alloc(sizeof(struct complex_nested));
    gt_ggc_mx(nested_var);
    
    /* Test pointer to various types */
    tree ptr_to_struct = build_pointer_type(struct_5);
    gt_ggc_mx(ptr_to_struct);
    
    tree ptr_to_union = build_pointer_type(union_2);
    gt_ggc_mx(ptr_to_union);
    
    tree ptr_to_array = build_pointer_type(array_2d);
    gt_ggc_mx(ptr_to_array);
    
    /* Test array of pointers */
    tree array_of_ptrs = create_test_array(build_pointer_type(char_type_node), 1);
    gt_ggc_mx(array_of_ptrs);
    
    /* Test struct containing arrays */
    tree struct_with_array = make_node(RECORD_TYPE);
    pushlevel();
    
    tree array_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("data_array"),
                                 array_1d);
    TYPE_FIELDS(struct_with_array) = array_field;
    layout_type(struct_with_array);
    
    poplevel();
    gt_ggc_mx(struct_with_array);
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
    /* Initialize GCC's internal structures if needed */
    test_gengtype_categorization();
    return 0;
}
#endif

/* Hook for plugin initialization */
#ifdef PLUGIN_TEST
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    test_gengtype_categorization();
    return 0;
}
#endif
