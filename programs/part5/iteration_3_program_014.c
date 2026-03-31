/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * 
 * Compilation for gengtype testing:
 * 1. Place in GCC source tree (e.g., gcc/testsuite/gengtype/)
 * 2. Use: gcc -c -O0 -g -fdump-tree-all -ffat-lto-objects test_gengtype_coverage.c
 * 3. Or integrate into GCC build with GTY markers
 */

/* Dummy GTY macro for standalone compilation */
#ifndef GTY
#define GTY(x) /* nothing */
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Forward declarations to create complex type dependencies */
struct forward_declared;
union forward_union;

/* TYPE_SCALAR: Basic scalar types */
struct GTY(()) ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_prec;
    long long int64;
    unsigned short uint16;
    _Bool boolean;
};

/* TYPE_STRING: String types */
struct GTY(()) StringTypes {
    const char* constant_string;
    char* mutable_string;
    char fixed_string[32];
    wchar_t* wide_string;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) PointerTypes {
    void* void_ptr;
    int* int_ptr;
    struct ScalarTypes* struct_ptr;
    union forward_union* union_ptr;
    int (*func_ptr)(int, char);  /* Function pointer */
    void (*callback)(void*);     /* Callback pointer */
    struct forward_declared** double_ptr;
};

/* TYPE_ARRAY: Array types */
struct GTY(()) ArrayTypes {
    int simple_array[10];
    char multi_dim[5][5];
    struct ScalarTypes* ptr_array[8];
    float flexible_array[];  /* Flexible array member */
};

/* TYPE_STRUCT: Nested structure */
struct GTY(()) NestedStruct {
    struct ScalarTypes scalars;
    struct StringTypes strings;
    int depth;
    struct NestedStruct* next;  /* Linked list */
};

/* TYPE_UNION: Union types */
union GTY(()) ComplexUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;
    struct {
        int tag;
        union ComplexUnion* next;
    } nested;
};

/* TYPE_USER_STRUCT: User-defined structure with tags */
struct GTY((tag("USER_TYPE"))) UserTaggedStruct {
    int user_id;
    char* user_name;
    struct UserTaggedStruct* self_ref;
};

/* TYPE_CALLBACK: Structure containing callback pointers */
struct GTY(()) CallbackContainer {
    int (*comparator)(const void*, const void*);
    void (*handler)(int, void*);
    char* (*string_processor)(char*);
    struct CallbackContainer* chain;
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
struct GTY(()) LangSpecific {
    void* lang_data;
    int lang_tag;
    struct LangSpecific* (*lang_method)(int);
};

/* Complete the forward declarations */
struct GTY(()) forward_declared {
    int value;
    struct forward_declared* next;
};

union GTY(()) forward_union {
    int int_val;
    struct forward_declared* struct_ptr;
};

/* TYPE_UNDEFINED: Create a self-referential type that might be undefined */
struct GTY(()) SelfReferential {
    int data;
    struct SelfReferential* ref;  /* Self-reference */
    struct UndefinedType* undefined_ptr;  /* Forward to undefined type */
};

/* Main container with all types */
struct GTY(()) TypeContainer {
    /* Scalar fields */
    int counter;
    float ratio;
    
    /* String field */
    const char* name;
    
    /* Struct fields */
    struct ScalarTypes scalars;
    struct NestedStruct nested;
    struct UserTaggedStruct user_struct;
    
    /* Union field */
    union ComplexUnion variant;
    
    /* Pointer fields */
    void* data_ptr;
    int* int_array_ptr;
    struct TypeContainer* next_container;
    
    /* Array fields */
    int scores[20];
    struct ScalarTypes object_array[5];
    
    /* Callback field */
    int (*operation)(int, int);
    
    /* Language struct field */
    struct LangSpecific lang_info;
    
    /* Self-reference */
    struct TypeContainer* self;
    
    /* Undefined reference */
    struct UndefinedType* undefined;
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void* ptr) {
    /* Simple hash to ensure types are referenced */
    return (size_t)ptr ^ 0xDEADBEEF;
}

/* Another external function */
__attribute__((noinline))
void touch_memory(void* ptr, size_t size) {
    volatile char* vptr = (volatile char*)ptr;
    for (size_t i = 0; i < size && i < 16; i++) {
        (void)vptr[i];  /* Just touch memory */
    }
}

int main(void) {
    /* Declare instances of all complex types */
    struct ScalarTypes scalars = {0};
    struct StringTypes strings = {0};
    struct PointerTypes pointers = {0};
    struct ArrayTypes arrays = {0};
    struct NestedStruct nested = {0};
    union ComplexUnion union_var = {0};
    struct UserTaggedStruct user_struct = {0};
    struct CallbackContainer callbacks = {0};
    struct LangSpecific lang_struct = {0};
    struct forward_declared fwd_decl = {0};
    union forward_union fwd_union = {0};
    struct SelfReferential self_ref = {0};
    struct TypeContainer container = {0};
    
    /* Take addresses to ensure types are considered */
    void* addresses[] = {
        &scalars, &strings, &pointers, &arrays,
        &nested, &union_var, &user_struct, &callbacks,
        &lang_struct, &fwd_decl, &fwd_union, &self_ref,
        &container,
        &scalars.integer, &strings.constant_string,
        &pointers.func_ptr, &arrays.simple_array,
        &nested.next, &user_struct.self_ref,
        &callbacks.comparator, &lang_struct.lang_method,
        &container.scores, &container.operation
    };
    
    /* Compute sizeof all types */
    size_t sizes[] = {
        sizeof(struct ScalarTypes),
        sizeof(struct StringTypes),
        sizeof(struct PointerTypes),
        sizeof(struct ArrayTypes),
        sizeof(struct NestedStruct),
        sizeof(union ComplexUnion),
        sizeof(struct UserTaggedStruct),
        sizeof(struct CallbackContainer),
        sizeof(struct LangSpecific),
        sizeof(struct forward_declared),
        sizeof(union forward_union),
        sizeof(struct SelfReferential),
        sizeof(struct TypeContainer),
        sizeof(int*),
        sizeof(void (*)(void)),
        sizeof(char[32]),
        sizeof(struct ScalarTypes* [8]),
        sizeof(int[5][5])
    };
    
    /* Touch memory to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        touch_memory(addresses[i], 1);
    }
    
    /* Compute a checksum using all addresses and sizes */
    size_t checksum = 0;
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        checksum ^= compute_checksum(addresses[i]);
    }
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        checksum ^= sizes[i];
    }
    
    /* Use the checksum to prevent dead code elimination */
    volatile size_t result = checksum;
    
    printf("Type analysis test complete. Checksum: %zu\n", result);
    printf("Total types processed: %zu\n", 
           sizeof(addresses)/sizeof(addresses[0]) + 
           sizeof(sizes)/sizeof(sizes[0]));
    
    return (int)(result & 0x7FFFFFFF);
}

/* Additional complex type definitions to ensure full coverage */

/* Mixed type with all categories */
struct GTY(()) UltimateType {
    /* TYPE_SCALAR */
    int id;
    float value;
    
    /* TYPE_STRING */
    const char* description;
    
    /* TYPE_STRUCT */
    struct {
        int x, y;
    } point;
    
    /* TYPE_UNION */
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } data;
    
    /* TYPE_POINTER */
    struct UltimateType* next;
    int (*transform)(struct UltimateType*);
    
    /* TYPE_ARRAY */
    int matrix[3][3];
    struct UltimateType* ref_list[5];
    
    /* TYPE_CALLBACK */
    void (*notify)(int, const char*);
    
    /* TYPE_LANG_STRUCT */
    struct LangSpecific* lang_ext;
    
    /* Self-reference for cycles */
    struct UltimateType* self;
    
    /* Reference to undefined type */
    struct NeverDefined* mystery;
};

/* Global instances to ensure they're processed */
struct UltimateType GTY(()) global_ultimate;
struct TypeContainer GTY(()) global_container;
