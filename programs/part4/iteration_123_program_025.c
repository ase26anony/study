/* Test for gengtype.cc type categorization coverage */
/* This should be compiled as part of GCC's test suite */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "gtype-desc.h"

/* Forward declarations for helper functions */
static tree create_test_struct_type(int field_count);
static tree create_test_union_type(int field_count);
static tree create_test_array_type(tree element_type, int dimensions);
static tree create_test_callback_type(void);

/* Global variables with GTY annotations to force type processing */
/* Each variable uses a different type to cover all categories */

/* TYPE_SCALAR */
static GTY(()) tree scalar_var = integer_type_node;

/* TYPE_POINTER */
static GTY(()) tree pointer_var = NULL_TREE;

/* TYPE_ARRAY */
static GTY(()) tree array_var = NULL_TREE;

/* TYPE_STRUCT */
static GTY(()) tree struct_var = NULL_TREE;

/* TYPE_UNION */
static GTY(()) tree union_var = NULL_TREE;

/* TYPE_STRING - char* type */
static GTY(()) const char* string_var = "test";

/* TYPE_CALLBACK - function pointer */
typedef void (*callback_func)(void);
static GTY(()) callback_func callback_var = NULL;

/* TYPE_USER_STRUCT - struct with language-specific info */
static GTY(()) tree user_struct_var = NULL_TREE;

/* TYPE_LANG_STRUCT - language-specific structure */
static GTY(()) tree lang_struct_var = NULL_TREE;

/* Complex structure containing multiple type categories */
struct GTY(()) complex_container {
    tree scalar_field;      /* TYPE_SCALAR */
    tree* pointer_field;    /* TYPE_POINTER */
    tree array_field[5];    /* TYPE_ARRAY */
    tree struct_field;      /* TYPE_STRUCT */
    tree union_field;       /* TYPE_UNION */
    const char* string_field; /* TYPE_STRING */
    void (*callback_field)(void); /* TYPE_CALLBACK */
};

static GTY(()) struct complex_container container_var;

/* Helper function to create a struct type with variable field count */
static tree
create_test_struct_type(int field_count)
{
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

/* Helper function to create a union type with variable field count */
static tree
create_test_union_type(int field_count)
{
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

/* Helper function to create multi-dimensional array type */
static tree
create_test_array_type(tree element_type, int dimensions)
{
    tree array_type = element_type;
    
    for (int i = 0; i < dimensions; i++) {
        tree index_type = build_index_type(size_int(10 + i));
        array_type = build_array_type(array_type, index_type);
    }
    
    return array_type;
}

/* Helper function to create a callback (function pointer) type */
static tree
create_test_callback_type(void)
{
    tree return_type = void_type_node;
    tree arg_types = NULL_TREE;
    
    /* Create a function type with some arguments */
    for (int i = 0; i < 3; i++) {
        arg_types = chainon(arg_types, integer_type_node);
    }
    
    tree func_type = build_function_type(return_type, arg_types);
    tree func_ptr_type = build_pointer_type(func_type);
    
    return func_ptr_type;
}

/* Main test function that creates and registers all type categories */
void
test_gengtype_categorization(void)
{
    /* Create and register TYPE_SCALAR types */
    scalar_var = integer_type_node;
    gt_ggc_mx (scalar_var);
    
    /* Create and register TYPE_POINTER types */
    pointer_var = build_pointer_type(integer_type_node);
    gt_ggc_mx (pointer_var);
    
    /* Create and register TYPE_ARRAY types */
    array_var = create_test_array_type(integer_type_node, __DIMENSIONS__);
    gt_ggc_mx (array_var);
    
    /* Create and register TYPE_STRUCT types */
    struct_var = create_test_struct_type(__FIELD_COUNT__);
    gt_ggc_mx (struct_var);
    
    /* Create and register TYPE_UNION types */
    union_var = create_test_union_type(__FIELD_COUNT__);
    gt_ggc_mx (union_var);
    
    /* TYPE_STRING is handled by string_var (char*) */
    gt_ggc_mx (string_var);
    
    /* Create and register TYPE_CALLBACK types */
    tree callback_type = create_test_callback_type();
    gt_ggc_mx (callback_type);
    
    /* Create TYPE_USER_STRUCT - mark a struct with user flag */
    tree user_struct = create_test_struct_type(2);
    TYPE_USER_ALIGN(user_struct) = 1;
    user_struct_var = user_struct;
    gt_ggc_mx (user_struct_var);
    
    /* Create TYPE_LANG_STRUCT - struct with language-specific info */
    tree lang_struct = create_test_struct_type(3);
    
    /* Allocate and set language-specific data */
    struct lang_type *lang_data = ggc_alloc<struct lang_type>();
    TYPE_LANG_SPECIFIC(lang_struct) = lang_data;
    lang_struct_var = lang_struct;
    gt_ggc_mx (lang_struct_var);
    
    /* Initialize the complex container to ensure all fields are processed */
    container_var.scalar_field = integer_type_node;
    container_var.pointer_field = &pointer_var;
    container_var.struct_field = struct_var;
    container_var.union_field = union_var;
    container_var.string_field = "container_string";
    container_var.callback_field = NULL;
    
    /* Fill array field */
    for (int i = 0; i < 5; i++) {
        container_var.array_field[i] = integer_type_node;
    }
    
    gt_ggc_mx (&container_var);
    
    /* Process various tree node types that might have different classifications */
    tree node_types[] = {
        void_type_node,
        boolean_type_node,
        char_type_node,
        integer_type_node,
        ptr_type_node,
        build_pointer_type(char_type_node),  /* Another TYPE_STRING candidate */
        build_array_type(char_type_node, build_index_type(size_int(10))),
        make_node(ENUMERAL_TYPE),
        make_node(METHOD_TYPE),
    };
    
    for (size_t i = 0; i < sizeof(node_types)/sizeof(node_types[0]); i++) {
        gt_ggc_mx (node_types[i]);
    }
    
    /* Force processing of TYPE_UNDEFINED by creating an incomplete type */
    tree undefined_type = make_node(LANG_TYPE);
    gt_ggc_mx (undefined_type);
}

/* Main entry point for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
    test_gengtype_categorization();
    return 0;
}
#endif
