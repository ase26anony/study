/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * the type enumeration logic in gengtype.cc's switch statement.
 * When processed by GCC's gengtype utility during a build,
 * it should trigger all type kind cases.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Simulate GTY markers used in GCC internals */
#define GTY(x) /* nothing for test compilation */

/* Forward declarations for complex type relationships */
struct ForwardDeclared;
union ForwardUnion;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

/* TYPE_STRING: String type (const char*) */
typedef const char* string_type;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_func)(int, void*);
typedef int (*comparator_func)(const void*, const void*);

/* TYPE_STRUCT: Basic structure */
struct BasicStruct {
    int id;
    char name[32];
    float value;
};

/* TYPE_UNION: Basic union */
union BasicUnion {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_str[16];
};

/* TYPE_USER_STRUCT: User-defined structure with GTY marker */
struct GTY(()) UserStruct {
    int tag;
    union {
        int int_val;
        double double_val;
        void* ptr_val;
    } data;
    struct UserStruct* next;
};

/* TYPE_ARRAY: Various array types */
struct ArrayContainer {
    int fixed_array[10];           /* Fixed-size array */
    double matrix[3][3];           /* Multi-dimensional array */
    char* string_array[5];         /* Array of pointers */
    struct BasicStruct struct_array[4]; /* Array of structs */
};

/* TYPE_POINTER: Complex pointer relationships */
struct PointerNetwork {
    /* Simple pointers */
    int* int_ptr;
    char** string_ptr_ptr;
    
    /* Pointer to struct */
    struct BasicStruct* struct_ptr;
    
    /* Pointer to union */
    union BasicUnion* union_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to function (callback) */
    callback_func callback_ptr;
    
    /* Self-referential pointer */
    struct PointerNetwork* self_ptr;
    
    /* Pointer to forward declared type */
    struct ForwardDeclared* forward_ptr;
    
    /* Pointer to pointer to pointer */
    void*** triple_ptr;
};

/* TYPE_UNDEFINED: Forward declared but not yet defined */
struct ForwardDeclared {
    int magic;
    struct ForwardDeclared* next;
    union ForwardUnion* related;
};

/* Another union type */
union ForwardUnion {
    long long_val;
    struct ForwardDeclared* struct_ptr;
    callback_func func_ptr;
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
struct LangStruct {
    int lang_specific_tag;
    void* lang_data;
    struct LangStruct* (*lang_method)(int);
};

/* Nested complex structure combining all types */
struct GTY(()) MasterStruct {
    /* Scalars */
    scalar_int_t int_field;
    scalar_float_t float_field;
    scalar_double_t double_field;
    
    /* String */
    string_type str_field;
    
    /* Structures */
    struct BasicStruct nested_struct;
    struct UserStruct user_struct_field;
    
    /* Union */
    union BasicUnion choice;
    
    /* Pointers */
    struct PointerNetwork* ptr_net;
    struct MasterStruct* recursive_ptr;
    
    /* Arrays */
    int dynamic_sized[];  /* Flexible array member */
    
    /* Callback */
    callback_func handler;
    
    /* Language struct */
    struct LangStruct* lang_obj;
};

/* Union containing various types */
union MegaUnion {
    struct BasicStruct as_struct;
    struct UserStruct as_user_struct;
    struct PointerNetwork as_ptr_net;
    struct MasterStruct* as_master_ptr;
    callback_func as_callback;
    int as_array[20];
};

/* Another complex type with deeply nested relationships */
struct GTY(()) TypeGraph {
    enum { NODE_INT, NODE_STRUCT, NODE_UNION, NODE_PTR } node_type;
    
    union {
        int int_value;
        struct TypeGraph* struct_node;
        union MegaUnion* union_node;
        void** ptr_node;
    } data;
    
    struct TypeGraph* left;
    struct TypeGraph* right;
    
    /* Array of function pointers */
    comparator_func comparators[3];
    
    /* Pointer to array */
    int (*matrix_view)[4][4];
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_type_sizes(void* ptr) {
    /* This function forces the compiler to consider all types */
    (void)ptr;
    return 0;
}

/* Volatile function to ensure references aren't optimized away */
volatile size_t keep_alive = 0;

int main(void) {
    /* Declare instances of all complex types */
    struct BasicStruct basic = {1, "test", 3.14f};
    union BasicUnion basic_union;
    struct UserStruct user_struct = {0};
    struct ArrayContainer arrays = {0};
    struct PointerNetwork pointers = {0};
    struct ForwardDeclared forward = {42, NULL, NULL};
    union ForwardUnion forward_union;
    struct LangStruct lang_struct = {0};
    struct MasterStruct* master = NULL;
    union MegaUnion mega_union;
    struct TypeGraph type_graph = {0};
    
    /* Initialize some values */
    basic_union.as_int = 100;
    user_struct.tag = 2;
    user_struct.data.int_val = 42;
    user_struct.next = &user_struct;  /* Self-reference */
    
    forward_union.long_val = 0xDEADBEEF;
    
    /* Take addresses of all instances */
    struct BasicStruct* basic_ptr = &basic;
    union BasicUnion* union_ptr = &basic_union;
    struct UserStruct* user_ptr = &user_struct;
    struct ArrayContainer* array_ptr = &arrays;
    struct PointerNetwork* network_ptr = &pointers;
    struct ForwardDeclared* forward_ptr = &forward;
    union ForwardUnion* forward_union_ptr = &forward_union;
    struct LangStruct* lang_ptr = &lang_struct;
    union MegaUnion* mega_ptr = &mega_union;
    struct TypeGraph* graph_ptr = &type_graph;
    
    /* Compute sizes of all types */
    size_t sizes[] = {
        sizeof(struct BasicStruct),
        sizeof(union BasicUnion),
        sizeof(struct UserStruct),
        sizeof(struct ArrayContainer),
        sizeof(struct PointerNetwork),
        sizeof(struct ForwardDeclared),
        sizeof(union ForwardUnion),
        sizeof(struct LangStruct),
        sizeof(struct MasterStruct),
        sizeof(union MegaUnion),
        sizeof(struct TypeGraph),
        sizeof(scalar_int_t),
        sizeof(string_type),
        sizeof(callback_func),
        sizeof(int*),
        sizeof(int[10]),
        sizeof(double[3][3]),
        sizeof(struct BasicStruct*),
        sizeof(void***)
    };
    
    /* Calculate a checksum to prevent optimization */
    size_t checksum = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        checksum += sizes[i];
        keep_alive += sizes[i];  /* Volatile store */
    }
    
    /* Set up pointer relationships */
    pointers.int_ptr = &basic.id;
    pointers.struct_ptr = &basic;
    pointers.union_ptr = &basic_union;
    pointers.self_ptr = &pointers;
    pointers.forward_ptr = &forward;
    pointers.callback_ptr = NULL;
    
    /* Set up array relationships */
    for (int i = 0; i < 10; i++) {
        arrays.fixed_array[i] = i;
    }
    
    /* Set up the type graph */
    type_graph.node_type = 1;  /* NODE_STRUCT */
    type_graph.data.struct_node = &type_graph;  /* Self-reference */
    type_graph.left = NULL;
    type_graph.right = NULL;
    
    /* Reference function pointers */
    type_graph.comparators[0] = NULL;
    
    /* Call external function with various type pointers */
    compute_type_sizes(&basic);
    compute_type_sizes(&user_struct);
    compute_type_sizes(&pointers);
    compute_type_sizes(&type_graph);
    compute_type_sizes(&mega_union);
    
    /* Print something to ensure execution */
    printf("Type analysis test complete. Checksum: %zu\n", checksum);
    printf("Address samples:\n");
    printf("  basic: %p\n", (void*)basic_ptr);
    printf("  user_struct: %p\n", (void*)user_ptr);
    printf("  pointers: %p\n", (void*)network_ptr);
    printf("  type_graph: %p\n", (void*)graph_ptr);
    
    /* Use volatile variable */
    printf("Keep-alive value: %zu\n", keep_alive);
    
    return 0;
}
