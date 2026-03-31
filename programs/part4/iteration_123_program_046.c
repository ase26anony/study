/* Test for gengtype.cc type categorization coverage */
/* This should be compiled as part of GCC's test suite */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"

/* Forward declarations for helper functions */
static tree create_test_struct_type(void);
static tree create_test_union_type(void);
static tree create_test_array_type(void);
static tree create_test_callback_type(void);

/* Global variables with GTY annotations to force type processing */
/* Each variable type should trigger a different case in the switch */

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
static GTY(()) const char *string_var = "test";

/* TYPE_CALLBACK - function pointer */
static GTY(()) tree callback_var = NULL_TREE;

/* Complex struct with nested types to trigger multiple cases */
struct GTY(()) complex_type {
    int scalar_field;           /* TYPE_SCALAR */
    void *pointer_field;        /* TYPE_POINTER */
    char *string_field;         /* TYPE_STRING */
    int array_field[10];        /* TYPE_ARRAY */
    tree tree_field;            /* TYPE_POINTER (tree is pointer type) */
};

static GTY(()) struct complex_type complex_var;

/* Union type */
union GTY(()) test_union {
    int i;
    float f;
    void *p;
};

static GTY(()) union test_union union_var2;

/* Struct with variable number of fields for mutation */
struct GTY(()) variable_struct {
    int field1;
    /* __FIELD_COUNT__ more fields will be added during mutation */
    int field2;
    int field3;
    char *name;
};

static GTY(()) struct variable_struct var_struct;

/* Array of structs */
static GTY(()) struct variable_struct struct_array[5];

/* Function pointer type for callback */
typedef void (*callback_func)(int, void*);
static GTY(()) callback_func callback_ptr = NULL;

/* Language-specific struct simulation */
#ifdef TYPE_LANG_STRUCT
struct GTY(()) lang_specific_struct {
    int lang_data;
    tree lang_tree;
};

static GTY(()) struct lang_specific_struct lang_struct_var;
#endif

/* User struct type - marked with special flag */
struct GTY((user)) user_struct {
    int user_data;
    char *user_string;
};

static GTY(()) struct user_struct user_struct_var;

/* Helper function to create a struct type with fields */
static tree
create_test_struct_type(void)
{
    tree struct_type = make_node(RECORD_TYPE);
    tree field_list = NULL_TREE;
    
    /* Add some fields to the struct */
    tree int_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("field1"),
                               integer_type_node);
    DECL_CHAIN(int_field) = field_list;
    field_list = int_field;
    
    tree char_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("field2"),
                                char_type_node);
    DECL_CHAIN(char_field) = int_field;
    field_list = char_field;
    
    TYPE_FIELDS(struct_type) = field_list;
    layout_type(struct_type);
    
    return struct_type;
}

/* Helper function to create a union type */
static tree
create_test_union_type(void)
{
    tree union_type = make_node(UNION_TYPE);
    tree field_list = NULL_TREE;
    
    /* Add fields to the union */
    tree int_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier("int_field"),
                               integer_type_node);
    DECL_CHAIN(int_field) = field_list;
    field_list = int_field;
    
    tree float_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                 get_identifier("float_field"),
                                 float_type_node);
    DECL_CHAIN(float_field) = int_field;
    field_list = float_field;
    
    TYPE_FIELDS(union_type) = field_list;
    layout_type(union_type);
    
    return union_type;
}

/* Helper function to create an array type */
static tree
create_test_array_type(void)
{
    /* Create array of 10 integers */
    tree index_type = build_index_type(size_int(9));
    return build_array_type(integer_type_node, index_type);
}

/* Helper function to create a callback (function pointer) type */
static tree
create_test_callback_type(void)
{
    /* Create function type: void (*)(int) */
    tree arg_type = integer_type_node;
    tree func_type = build_function_type_list(void_type_node,
                                             arg_type, NULL_TREE);
    return build_pointer_type(func_type);
}

/* Main test function */
void
test_gengtype_categorization(void)
{
    /* Initialize global variables with different types */
    
    /* TYPE_SCALAR - already initialized to integer_type_node */
    
    /* TYPE_POINTER */
    pointer_var = build_pointer_type(integer_type_node);
    
    /* TYPE_ARRAY */
    array_var = create_test_array_type();
    
    /* TYPE_STRUCT */
    struct_var = create_test_struct_type();
    
    /* TYPE_UNION */
    union_var = create_test_union_type();
    
    /* TYPE_STRING - char* is already a string type */
    
    /* TYPE_CALLBACK */
    callback_var = create_test_callback_type();
    
    /* Initialize complex struct fields */
    complex_var.scalar_field = 42;
    complex_var.pointer_field = &complex_var;
    complex_var.string_field = "Hello";
    complex_var.tree_field = integer_type_node;
    
    /* Initialize union */
    union_var2.i = 100;
    
    /* Initialize variable struct - vary field count via mutation */
    var_struct.field1 = 1;
    var_struct.field2 = 2;
    var_struct.field3 = 3;
    var_struct.name = "test_struct";
    
    /* Initialize array of structs */
    for (int i = 0; i < 5; i++) {
        struct_array[i].field1 = i;
        struct_array[i].field2 = i * 2;
        struct_array[i].field3 = i * 3;
        struct_array[i].name = "array_element";
    }
    
    /* Initialize user struct */
    user_struct_var.user_data = 999;
    user_struct_var.user_string = "user_struct";
    
#ifdef TYPE_LANG_STRUCT
    /* Initialize language-specific struct if supported */
    lang_struct_var.lang_data = 123;
    lang_struct_var.lang_tree = void_type_node;
#endif
    
    /* Force GC registration of all types by using gt_ggc_mx macros */
    /* This triggers gengtype processing */
    gt_ggc_mx (scalar_var);
    gt_ggc_mx (pointer_var);
    gt_ggc_mx (array_var);
    gt_ggc_mx (struct_var);
    gt_ggc_mx (union_var);
    gt_ggc_mx (callback_var);
    gt_ggc_mx (&complex_var);
    gt_ggc_mx (&union_var2);
    gt_ggc_mx (&var_struct);
    gt_ggc_mx (&struct_array[0]);
    gt_ggc_mx (&user_struct_var);
#ifdef TYPE_LANG_STRUCT
    gt_ggc_mx (&lang_struct_var);
#endif
    
    /* Also process through alternative path if available */
#ifdef gt_types_enum_last
    /* This might trigger categorization directly */
    for (enum gt_types_enum i = (enum gt_types_enum)0; 
         i < gt_types_enum_last; 
         i = (enum gt_types_enum)((int)i + 1)) {
        /* Access type table to ensure processing */
        if (gt_type_tab[i] != NULL) {
            /* Force reference */
            volatile void *dummy = (void *)gt_type_tab[i];
            (void)dummy;
        }
    }
#endif
}

/* Main function for standalone testing */
#ifdef STANDALONE_TEST
int main(void)
{
    test_gengtype_categorization();
    return 0;
}
#endif
