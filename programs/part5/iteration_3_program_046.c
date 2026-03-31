/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * 
 * Compilation for gengtype testing:
 *   gcc -O0 -g -fdump-tree-all -c -ffat-lto-objects test_gengtype_coverage.c
 * 
 * For integration into GCC build:
 *   Place in gcc/testsuite/gcc.dg/gengtype/ and run make check-gengtype
 */

/* Dummy GTY macro for compilation outside GCC */
#ifndef GTY
#define GTY(x) 
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Forward declarations to create complex type dependencies */
struct forward_declared_struct;
union forward_declared_union;

/* TYPE_SCALAR: Basic scalar types */
GTY(()) struct ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_value;
    unsigned int unsigned_integer;
    _Bool boolean;
    enum { RED, GREEN, BLUE } color_enum;
};

/* TYPE_STRING: String types */
GTY(()) struct StringTypes {
    const char *constant_string;
    char *mutable_string;
    char fixed_string[64];
    wchar_t *wide_string;
};

/* TYPE_POINTER: Various pointer types */
GTY(()) struct PointerTypes {
    int *int_ptr;
    void *void_ptr;
    struct ScalarTypes *struct_ptr;
    struct forward_declared_struct *forward_ptr;
    int **double_ptr;
    
    /* Function pointer - TYPE_CALLBACK */
    int (*func_ptr)(int, char*);
    void (*callback)(void*);
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to pointer to function */
    int (*(*complex_func_ptr))(void);
};

/* TYPE_ARRAY: Array types */
GTY(()) struct ArrayTypes {
    int simple_array[10];
    char multi_dim_array[5][10];
    struct ScalarTypes struct_array[3];
    void *pointer_array[8];
    
    /* Flexible array member */
    int flexible_array[];
};

/* TYPE_STRUCT: Nested structures */
GTY(()) struct OuterStruct {
    int id;
    
    /* Nested anonymous struct */
    struct {
        float x;
        float y;
    } point;
    
    /* Nested named struct */
    struct InnerStruct {
        int counter;
        char label[32];
    } inner;
    
    /* Array of nested structs */
    struct {
        int start;
        int end;
    } ranges[4];
};

/* TYPE_UNION: Union types */
GTY(()) union DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
    char as_bytes[8];
    
    /* Nested union in struct */
    struct {
        int type;
        union {
            int int_value;
            float float_value;
        } data;
    } tagged;
};

/* TYPE_USER_STRUCT: User-defined structure with complex nesting */
GTY(()) struct UserDefinedStruct {
    /* Contains all previous types */
    struct ScalarTypes scalars;
    struct StringTypes strings;
    struct PointerTypes pointers;
    struct ArrayTypes arrays;
    struct OuterStruct nested;
    union DataUnion variant;
    
    /* Self-referential pointer */
    struct UserDefinedStruct *next;
    
    /* Pointer to incomplete type */
    struct forward_declared_struct *incomplete;
    
    /* Array of function pointers */
    int (*operations[5])(struct UserDefinedStruct*);
};

/* Complete the forward declarations */
GTY(()) struct forward_declared_struct {
    int magic_number;
    struct UserDefinedStruct *user_struct;
};

GTY(()) union forward_declared_union {
    long long_value;
    struct forward_declared_struct *struct_ptr;
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
/* This would typically be defined in language-specific headers */
GTY(()) struct LangSpecificStruct {
    void *language_data;
    int language_tag;
    
    /* Language-specific extensions */
    struct {
        int lang_feature_flags;
        void *lang_context;
    } extensions;
};

/* Callback function types for TYPE_CALLBACK */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CleanupFunc)(void*);
typedef struct UserDefinedStruct* (*FactoryFunc)(int);

/* Global variables to ensure types are used */
GTY(()) struct UserDefinedStruct global_user_struct;
GTY(()) union DataUnion global_union;
GTY(()) struct LangSpecificStruct global_lang_struct;

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_type_sizes(void) {
    size_t total = 0;
    
    total += sizeof(struct ScalarTypes);
    total += sizeof(struct StringTypes);
    total += sizeof(struct PointerTypes);
    total += sizeof(struct ArrayTypes);
    total += sizeof(struct OuterStruct);
    total += sizeof(union DataUnion);
    total += sizeof(struct UserDefinedStruct);
    total += sizeof(struct forward_declared_struct);
    total += sizeof(union forward_declared_union);
    total += sizeof(struct LangSpecificStruct);
    
    return total;
}

/* Another noinline function to take addresses */
__attribute__((noinline))
void take_addresses(
    struct ScalarTypes *scalar_ptr,
    struct StringTypes *string_ptr,
    struct PointerTypes *pointer_ptr,
    struct ArrayTypes *array_ptr,
    struct OuterStruct *outer_ptr,
    union DataUnion *union_ptr,
    struct UserDefinedStruct *user_ptr,
    struct forward_declared_struct *forward_ptr,
    union forward_declared_union *forward_union_ptr,
    struct LangSpecificStruct *lang_ptr
) {
    /* Volatile to prevent optimization */
    volatile size_t dummy = 0;
    
    dummy += (size_t)scalar_ptr;
    dummy += (size_t)string_ptr;
    dummy += (size_t)pointer_ptr;
    dummy += (size_t)array_ptr;
    dummy += (size_t)outer_ptr;
    dummy += (size_t)union_ptr;
    dummy += (size_t)user_ptr;
    dummy += (size_t)forward_ptr;
    dummy += (size_t)forward_union_ptr;
    dummy += (size_t)lang_ptr;
    
    /* Reference members to ensure complete type analysis */
    if (pointer_ptr && pointer_ptr->func_ptr) {
        dummy += (size_t)pointer_ptr->func_ptr;
    }
    
    if (user_ptr && user_ptr->operations[0]) {
        dummy += (size_t)user_ptr->operations[0];
    }
}

int main(void) {
    /* Declare instances of all complex types */
    struct ScalarTypes scalars = {0};
    struct StringTypes strings = {0};
    struct PointerTypes pointers = {0};
    struct ArrayTypes *array_ptr = NULL;
    struct OuterStruct outer = {0};
    union DataUnion data_union;
    struct UserDefinedStruct user_struct = {0};
    struct forward_declared_struct forward_struct = {0};
    union forward_declared_union forward_union;
    struct LangSpecificStruct lang_struct = {0};
    
    /* Initialize some values */
    scalars.integer = 42;
    strings.constant_string = "Hello, gengtype!";
    pointers.func_ptr = NULL;
    data_union.as_int = 100;
    user_struct.next = &user_struct;  /* Self-reference */
    forward_struct.magic_number = 0xDEADBEEF;
    lang_struct.language_tag = 1;
    
    /* Allocate array with flexible array member */
    array_ptr = (struct ArrayTypes*)malloc(
        sizeof(struct ArrayTypes) + 20 * sizeof(int));
    if (array_ptr) {
        for (int i = 0; i < 10; i++) {
            array_ptr->simple_array[i] = i * i;
        }
    }
    
    /* Take addresses of all instances and their members */
    take_addresses(
        &scalars,
        &strings,
        &pointers,
        array_ptr,
        &outer,
        &data_union,
        &user_struct,
        &forward_struct,
        &forward_union,
        &lang_struct
    );
    
    /* Compute total size of all types */
    size_t total_size = compute_type_sizes();
    
    /* Also compute sizes of individual members */
    size_t member_sizes = 0;
    member_sizes += sizeof(scalars.integer);
    member_sizes += sizeof(strings.constant_string);
    member_sizes += sizeof(pointers.func_ptr);
    member_sizes += sizeof(outer.point);
    member_sizes += sizeof(data_union.as_bytes);
    member_sizes += sizeof(user_struct.next);
    member_sizes += sizeof(forward_struct.magic_number);
    member_sizes += sizeof(lang_struct.extensions);
    
    /* Print results to prevent optimization */
    printf("Total type sizes: %zu bytes\n", total_size);
    printf("Member sizes sum: %zu bytes\n", member_sizes);
    printf("Pointer sizes: int*=%zu, void*=%zu, func*=%zu\n",
           sizeof(int*), sizeof(void*), sizeof(int(*)(void)));
    
    /* Cleanup */
    if (array_ptr) {
        free(array_ptr);
    }
    
    return 0;
}

/* Additional complex type definitions to ensure coverage */

/* Mixed struct with all type kinds */
GTY(()) struct AllTypes {
    /* TYPE_SCALAR */
    int scalar_field;
    
    /* TYPE_STRING */
    const char *string_field;
    
    /* TYPE_POINTER */
    void *pointer_field;
    
    /* TYPE_ARRAY */
    int array_field[5];
    
    /* TYPE_STRUCT */
    struct {
        int nested_scalar;
    } nested_struct;
    
    /* TYPE_UNION */
    union {
        int as_int;
        void *as_ptr;
    } nested_union;
    
    /* TYPE_CALLBACK */
    void (*callback_field)(struct AllTypes*);
    
    /* Self-referential for graph traversal */
    struct AllTypes *self_ptr;
    
    /* Array of pointers to callbacks */
    int (*callbacks[3])(void);
};

/* Complex graph structure */
GTY(()) struct GraphNode {
    int id;
    struct GraphNode **neighbors;
    int neighbor_count;
    
    /* Callback for visiting */
    void (*visit)(struct GraphNode*);
    
    /* Union for node data */
    union {
        int int_data;
        double double_data;
        char *string_data;
    } data;
};

/* Global instance to ensure type is considered */
GTY(()) struct AllTypes global_all_types;
GTY(()) struct GraphNode *global_graph;
