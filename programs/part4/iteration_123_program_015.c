/* test_gengtype_categorization.c - Comprehensive test for gengtype type categorization */
/* Compile with: gcc -I$gcc_build/gcc -I$gcc_src/gcc -fplugin=$gcc_build/gcc/cc1 -O0 -g -c test.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-core.h"
#include "gtype-desc.h"
#include "gengtype.h"
#include <stdio.h>

/* Prevent optimization from removing our test code */
#pragma GCC optimize ("O0")

/* Define placeholders for variability */
#ifndef FIELD_COUNT
#define FIELD_COUNT 3
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE 10
#endif

/* ==================== TYPE DEFINITIONS WITH GTY ANNOTATIONS ==================== */

/* 1. SCALAR TYPES - TYPE_SCALAR */
static GTY(()) tree scalar_var = NULL_TREE;

/* 2. POINTER TYPES - TYPE_POINTER */
static GTY(()) tree pointer_var = NULL_TREE;

/* 3. ARRAY TYPES - TYPE_ARRAY */
static GTY(()) tree array_var = NULL_TREE;

/* 4. STRUCT TYPES - TYPE_STRUCT */
struct GTY(()) test_struct {
    int field1;
    tree GTY((tag("0"))) field2;
    void* GTY((skip)) field3;
};

/* 5. USER STRUCT - TYPE_USER_STRUCT */
struct GTY((user)) test_user_struct {
    int user_field1;
    tree GTY((tag("1"))) user_field2;
};

/* 6. UNION TYPES - TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    tree GTY((tag("2"))) tree_val;
    void* ptr_val;
};

/* 7. STRING TYPE - TYPE_STRING (char* with length) */
static GTY((length("strlen(%h) + 1"))) const char* string_var = NULL;

/* 8. CALLBACK TYPE - TYPE_CALLBACK (function pointer) */
typedef void (*callback_func)(void);
static GTY(()) callback_func callback_var = NULL;

/* 9. LANG STRUCT - TYPE_LANG_STRUCT */
struct GTY((desc("%1.type"), tag("3"))) test_lang_struct {
    enum tree_code type;
    tree GTY((skip)) lang_specific;
};

/* 10. NESTED STRUCTURES for comprehensive coverage */
struct GTY(()) container_struct {
    struct test_struct GTY((tag("4"))) nested_struct;
    union test_union GTY((tag("5"))) nested_union;
    tree GTY((tag("6"))) nested_array[ARRAY_SIZE];
    struct test_user_struct* GTY((tag("7"))) user_struct_ptr;
};

/* Global variable containing all types */
static GTY(()) struct container_struct* global_container = NULL;

/* ==================== TYPE CONSTRUCTION FUNCTIONS ==================== */

/* Function to create and register various types */
void __attribute__((noinline)) 
construct_and_register_types(void) {
    /* Create scalar types */
    scalar_var = integer_type_node;
    
    /* Create pointer type */
    pointer_var = build_pointer_type(integer_type_node);
    
    /* Create array type */
    tree array_type = build_array_type(integer_type_node, 
                                      build_index_type(size_int(ARRAY_SIZE - 1)));
    array_var = build1(ADDR_EXPR, build_pointer_type(array_type), NULL_TREE);
    
    /* Create struct type with fields */
    tree struct_type = make_node(RECORD_TYPE);
    tree field_list = NULL_TREE;
    
    for (int i = 0; i < FIELD_COUNT; i++) {
        char field_name[20];
        sprintf(field_name, "field%d", i);
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                               get_identifier(field_name),
                               (i % 2 == 0) ? integer_type_node : 
                               build_pointer_type(char_type_node));
        DECL_CHAIN(field) = field_list;
        field_list = field;
    }
    TYPE_FIELDS(struct_type) = field_list;
    layout_type(struct_type);
    
    /* Create union type */
    tree union_type = make_node(UNION_TYPE);
    tree union_field1 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("int_field"),
                                  integer_type_node);
    tree union_field2 = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                  get_identifier("ptr_field"),
                                  build_pointer_type(void_type_node));
    TYPE_FIELDS(union_type) = chainon(union_field1, union_field2);
    layout_type(union_type);
    
    /* Create string type (char*) */
    string_var = "test_string";
    
    /* Create callback type (function pointer) */
    tree func_type = build_function_type_list(void_type_node, NULL_TREE);
    tree func_ptr_type = build_pointer_type(func_type);
    
    /* Mark types for special categorization */
    #ifdef TYPE_LANG_FLAG_0
    TYPE_LANG_FLAG_0(struct_type) = 1;
    #endif
    
    #ifdef TYPE_LANG_FLAG_1
    TYPE_LANG_FLAG_1(union_type) = 1;
    #endif
    
    /* Force processing through gengtype by:
       1. Using gt_ggc_mx macros on the types
       2. Creating GTY-annotated globals that reference them
       3. Triggering GC root registration */
    
    /* Method 1: Direct macro invocation */
    gt_ggc_mx(tree);
    
    /* Method 2: Create and register a complex type graph */
    struct container_struct* container = 
        (struct container_struct*)xmalloc(sizeof(struct container_struct));
    
    /* Initialize fields with different types */
    container->nested_struct.field1 = 42;
    container->nested_struct.field2 = integer_type_node;
    container->nested_struct.field3 = NULL;
    
    container->nested_union.int_val = 100;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        container->nested_array[i] = size_int(i);
    }
    
    container->user_struct_ptr = 
        (struct test_user_struct*)xmalloc(sizeof(struct test_user_struct));
    container->user_struct_ptr->user_field1 = 200;
    container->user_struct_ptr->user_field2 = char_type_node;
    
    global_container = container;
    
    /* Method 3: Register various type nodes directly */
    static tree GTY(()) registered_types[10];
    
    registered_types[0] = integer_type_node;      /* SCALAR */
    registered_types[1] = pointer_var;            /* POINTER */
    registered_types[2] = array_var;              /* ARRAY */
    registered_types[3] = struct_type;            /* STRUCT */
    registered_types[4] = union_type;             /* UNION */
    registered_types[5] = build_pointer_type(char_type_node); /* STRING */
    registered_types[6] = func_ptr_type;          /* CALLBACK */
    
    /* Create a lang_struct by setting lang-specific info */
    tree lang_struct_type = make_node(RECORD_TYPE);
    #ifdef TYPE_LANG_SPECIFIC
    SET_TYPE_LANG_SPECIFIC(lang_struct_type, 
                          (struct lang_type*)xmalloc(sizeof(struct lang_type)));
    #endif
    registered_types[7] = lang_struct_type;       /* LANG_STRUCT */
    
    /* Create user struct type */
    tree user_struct_type = make_node(RECORD_TYPE);
    TYPE_USER_ALIGN(user_struct_type) = 1;
    registered_types[8] = user_struct_type;       /* USER_STRUCT */
    
    /* Force GC to consider all these types */
    for (int i = 0; i < 9; i++) {
        gt_ggc_m_9tree_node(registered_types[i]);
    }
}

/* ==================== TEST DRIVER ==================== */

/* Main test function */
void __attribute__((constructor)) 
test_gengtype_categorization(void) {
    printf("Starting gengtype type categorization test...\n");
    
    /* Multiple iterations with different configurations */
    for (int config = 0; config < 3; config++) {
        /* Vary parameters to hit different code paths */
        #undef FIELD_COUNT
        #undef ARRAY_SIZE
        #define FIELD_COUNT (config + 1)
        #define ARRAY_SIZE ((config + 1) * 5)
        
        construct_and_register_types();
        
        /* Force garbage collection to process types */
        ggc_collect();
        
        printf("Completed configuration %d with FIELD_COUNT=%d, ARRAY_SIZE=%d\n",
               config, FIELD_COUNT, ARRAY_SIZE);
    }
    
    printf("Gengtype type categorization test completed.\n");
    
    /* Diagnostic output - can be removed for production */
    #ifdef DEBUG_GENGTYPE
    extern unsigned nb_scalar, nb_pointer, nb_array, nb_struct, nb_union;
    extern unsigned nb_string, nb_callback, nb_lang_struct, nb_user_struct;
    
    printf("Type counts:\n");
    printf("  Scalars: %u\n", nb_scalar);
    printf("  Pointers: %u\n", nb_pointer);
    printf("  Arrays: %u\n", nb_array);
    printf("  Structs: %u\n", nb_struct);
    printf("  Unions: %u\n", nb_union);
    printf("  Strings: %u\n", nb_string);
    printf("  Callbacks: %u\n", nb_callback);
    printf("  Lang Structs: %u\n", nb_lang_struct);
    printf("  User Structs: %u\n", nb_user_struct);
    #endif
}

/* GCC plugin entry point if compiled as plugin */
#ifdef PLUGIN_LICENSE
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
    test_gengtype_categorization();
    return 0;
}
#endif

/* Standalone test main */
#ifndef PLUGIN_LICENSE
int main(void) {
    test_gengtype_categorization();
    return 0;
}
#endif
