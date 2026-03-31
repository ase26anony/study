/* Test for gengtype.cc type categorization coverage */
/* This test creates various GCC type nodes to exercise all type_enum cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "gtype-desc.h"
#include "tree-core.h"

/* Forward declarations */
void test_gengtype_categorization(void);

/* GTY-annotated globals to force gengtype processing */
typedef struct test_struct {
    int field1;
    char field2;
    void *field3;
} test_struct_t;

typedef union test_union {
    int int_val;
    char char_val;
    void *ptr_val;
} test_union_t;

/* Various GTY annotations to trigger different processing paths */
static GTY(()) tree scalar_types[5];
static GTY(()) tree pointer_types[3];
static GTY(()) tree array_types[2];
static GTY(()) tree struct_types[2];
static GTY(()) tree union_types[1];
static GTY(()) tree string_type;
static GTY(()) tree callback_types[2];
static GTY(()) tree lang_struct_type;

/* Helper to mark type as language-specific */
static void mark_as_lang_specific(tree type)
{
#ifdef TYPE_LANG_SPECIFIC
    if (TYPE_LANG_SPECIFIC(type) == NULL) {
        /* Allocate and set language-specific data */
        struct lang_type *lt = ggc_alloc<struct lang_type>();
        TYPE_LANG_SPECIFIC(type) = lt;
    }
#endif
}

/* Main test function */
void test_gengtype_categorization(void)
{
    tree type;
    int i;
    
    /* 1. SCALAR TYPES (TYPE_SCALAR) */
    scalar_types[0] = integer_type_node;
    scalar_types[1] = char_type_node;
    scalar_types[2] = boolean_type_node;
    scalar_types[3] = size_type_node;
    scalar_types[4] = ptrdiff_type_node;
    
    /* Force processing of scalar types */
    for (i = 0; i < 5; i++) {
        if (scalar_types[i]) {
            /* Use gt_ggc_mx to trigger gengtype processing */
            gt_ggc_mx (scalar_types[i]);
        }
    }
    
    /* 2. POINTER TYPES (TYPE_POINTER) */
    pointer_types[0] = build_pointer_type(integer_type_node);
    pointer_types[1] = build_pointer_type(char_type_node);
    pointer_types[2] = build_pointer_type(build_pointer_type(integer_type_node));
    
    for (i = 0; i < 3; i++) {
        if (pointer_types[i]) {
            gt_ggc_mx (pointer_types[i]);
        }
    }
    
    /* 3. ARRAY TYPES (TYPE_ARRAY) */
    /* Fixed-size array */
    array_types[0] = build_array_type(integer_type_node, 
                                     build_index_type(size_int(10)));
    
    /* Variable-length array */
    tree domain = build_index_type(NULL_TREE);
    TYPE_DOMAIN(domain) = build_range_type(sizetype, size_zero_node, 
                                          size_int(__FIELD_COUNT__));
    array_types[1] = build_array_type(char_type_node, domain);
    
    for (i = 0; i < 2; i++) {
        if (array_types[i]) {
            gt_ggc_mx (array_types[i]);
        }
    }
    
    /* 4. STRUCT TYPES (TYPE_STRUCT) */
    /* Simple struct with varying field count */
    struct_types[0] = make_node(RECORD_TYPE);
    tree field_list = NULL_TREE;
    
    /* Add fields to struct */
    for (i = 0; i < __FIELD_COUNT__; i++) {
        char field_name[20];
        sprintf(field_name, "field%d", i);
        
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier(field_name),
                               (i % 2 == 0) ? integer_type_node : char_type_node);
        
        field_list = chainon(field_list, field);
    }
    
    TYPE_FIELDS(struct_types[0]) = field_list;
    layout_type(struct_types[0]);
    
    /* Another struct with pointer fields */
    struct_types[1] = make_node(RECORD_TYPE);
    tree field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("data"),
                            build_pointer_type(integer_type_node));
    tree field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("next"),
                            build_pointer_type(struct_types[1]));
    
    TYPE_FIELDS(struct_types[1]) = chainon(field1, field2);
    layout_type(struct_types[1]);
    
    for (i = 0; i < 2; i++) {
        if (struct_types[i]) {
            gt_ggc_mx (struct_types[i]);
        }
    }
    
    /* 5. UNION TYPES (TYPE_UNION) */
    union_types[0] = make_node(UNION_TYPE);
    tree ufield1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("as_int"),
                             integer_type_node);
    tree ufield2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                             get_identifier("as_ptr"),
                             build_pointer_type(void_type_node));
    
    TYPE_FIELDS(union_types[0]) = chainon(ufield1, ufield2);
    layout_type(union_types[0]);
    
    gt_ggc_mx (union_types[0]);
    
    /* 6. STRING TYPE (TYPE_STRING) */
    /* char* is treated as TYPE_STRING in gengtype */
    string_type = build_pointer_type(char_type_node);
    gt_ggc_mx (string_type);
    
    /* 7. CALLBACK TYPES (TYPE_CALLBACK) - function pointers */
    /* Function type returning int, taking no arguments */
    tree func_type = build_function_type(integer_type_node, NULL_TREE);
    callback_types[0] = build_pointer_type(func_type);
    
    /* Function type with arguments */
    tree arg_types = tree_cons(NULL_TREE, integer_type_node, NULL_TREE);
    arg_types = tree_cons(NULL_TREE, char_type_node, arg_types);
    func_type = build_function_type(void_type_node, arg_types);
    callback_types[1] = build_pointer_type(func_type);
    
    for (i = 0; i < 2; i++) {
        if (callback_types[i]) {
            gt_ggc_mx (callback_types[i]);
        }
    }
    
    /* 8. LANG_STRUCT TYPE (TYPE_LANG_STRUCT) */
    /* Create a struct and mark it as language-specific */
    lang_struct_type = make_node(RECORD_TYPE);
    tree lfield = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("lang_data"),
                            integer_type_node);
    TYPE_FIELDS(lang_struct_type) = lfield;
    layout_type(lang_struct_type);
    
    /* Mark as language-specific if supported */
#ifdef TYPE_LANG_SPECIFIC
    mark_as_lang_specific(lang_struct_type);
#endif
    
    gt_ggc_mx (lang_struct_type);
    
    /* 9. USER_STRUCT TYPE (TYPE_USER_STRUCT) */
    /* This typically requires special GTY annotations or user-defined types */
    /* We'll create a struct and process it through gt_pch_nx */
    tree user_struct = make_node(RECORD_TYPE);
    tree ufield = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                            get_identifier("user_field"),
                            integer_type_node);
    TYPE_FIELDS(user_struct) = ufield;
    layout_type(user_struct);
    
    /* Process through alternative gengtype path */
    gt_pch_nx (&user_struct);
    
    /* 10. Process all types through ggc_mark_roots */
    /* This ensures they go through the full gengtype pipeline */
    ggc_mark_roots();
}

/* Main function for standalone testing */
#ifdef TEST_STANDALONE
int main(void)
{
    /* Initialize GCC's type system */
    build_common_tree_nodes(false);
    
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
