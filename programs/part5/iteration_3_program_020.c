/* test_gengtype_coverage.c
 * Complex type definitions to exercise gengtype.cc type enumeration logic
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC this marks GC roots */
#define GTY(x) 

/* Forward declarations to create pointer cycles */
struct ComplexStruct;
union NestedUnion;

/* ========== TYPE_SCALAR triggers ========== */
GTY(())
struct ScalarsOnly {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    short short_field;
    long long_field;
    long long longlong_field;
};

/* ========== TYPE_STRING triggers ========== */
GTY(())
struct WithStrings {
    const char* static_string;
    char* dynamic_string;
    const char* const constant_string;
    char fixed_string[64];
};

/* ========== TYPE_STRUCT triggers ========== */
GTY(())
struct SimpleStruct {
    int x;
    double y;
};

/* ========== TYPE_USER_STRUCT triggers ========== */
/* Nested struct definitions */
GTY(())
struct OuterStruct {
    struct SimpleStruct inner;
    struct {
        int anonymous_member;
        float anonymous_float;
    } anonymous_struct;
};

/* ========== TYPE_UNION triggers ========== */
GTY(())
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_pointer;
};

GTY(())
struct UnionContainer {
    union SimpleUnion data;
    union {
        char bytes[8];
        uint64_t word;
    } inline_union;
};

/* ========== TYPE_POINTER triggers ========== */
GTY(())
struct PointerFest {
    int* int_ptr;
    void* void_ptr;
    struct SimpleStruct* struct_ptr;
    union SimpleUnion* union_ptr;
    struct ComplexStruct* forward_ptr;  /* Forward declared */
    int** double_ptr;
    volatile int* volatile_ptr;
    const char* const const_ptr_to_const;
};

/* ========== TYPE_ARRAY triggers ========== */
GTY(())
struct ArrayCollection {
    int simple_array[10];
    struct SimpleStruct struct_array[5];
    union SimpleUnion union_array[3];
    char* pointer_array[8];
    int multi_dim[3][4][5];
    int flexible_array[];  /* Flexible array member */
};

/* ========== TYPE_CALLBACK triggers ========== */
typedef int (*callback_func)(int, void*);
typedef void (*simple_callback)(void);

GTY(())
struct WithCallbacks {
    callback_func handler;
    simple_callback notify;
    int (*array_of_callbacks[5])(void);
    void (*complex_callback)(struct SimpleStruct*, union SimpleUnion*);
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* In GCC, these are language-specific structures. We'll create something
   that looks like it could be language-specific */
GTY(())
struct LangLikeStruct {
    enum { TAG1, TAG2, TAG3 } discriminant;
    struct {
        int lang_specific_field;
        void* lang_data;
    } lang_part;
};

/* ========== Complex nested structure ========== */
GTY(())
struct ComplexStruct {
    /* Scalar fields */
    int id;
    double value;
    
    /* String field */
    const char* name;
    
    /* Struct field */
    struct SimpleStruct component;
    
    /* Union field */
    union SimpleUnion variant;
    
    /* Pointer fields */
    struct ComplexStruct* next;
    struct ComplexStruct* prev;
    void** pointer_matrix[4];
    
    /* Array fields */
    int coefficients[20];
    struct SimpleStruct parts[3];
    
    /* Callback field */
    callback_func validator;
    
    /* Nested anonymous struct */
    struct {
        unsigned flags;
        char metadata[16];
    } header;
    
    /* Pointer to array */
    int (*dynamic_array)[];
    
    /* Function pointer with complex signature */
    union SimpleUnion* (*factory)(int, const char*);
};

/* ========== Even more complex type ========== */
GTY(())
union NestedUnion {
    struct ComplexStruct as_struct;
    struct {
        int type_tag;
        GTY(()) union NestedUnion* recursive_ptr;
        GTY(()) struct ArrayCollection* array_collection;
    } as_metadata;
    callback_func as_function;
};

/* ========== Container with everything ========== */
GTY(())
struct UltimateContainer {
    struct ScalarsOnly scalars;
    struct WithStrings strings;
    struct OuterStruct nested_struct;
    struct UnionContainer unions;
    struct PointerFest pointers;
    struct ArrayCollection arrays;
    struct WithCallbacks callbacks;
    struct LangLikeStruct lang_struct;
    struct ComplexStruct complex;
    union NestedUnion recursive_union;
    
    /* Self-referential pointer */
    GTY(()) struct UltimateContainer* self;
    
    /* Array of function pointers */
    void (*operations[10])(struct UltimateContainer*);
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void* ptr, size_t size) {
    /* Simple hash to ensure types are referenced */
    unsigned char* bytes = (unsigned char*)ptr;
    size_t sum = 0;
    for (size_t i = 0; i < size && i < 64; i++) {
        sum += bytes[i];
    }
    return sum;
}

/* Another external function */
__attribute__((noinline))
void touch_memory(void* ptr) {
    volatile char* vptr = (volatile char*)ptr;
    if (vptr) {
        /* Just touch it to prevent optimization */
        (void)*vptr;
    }
}

int main(void) {
    /* Declare instances of all complex types */
    struct ScalarsOnly scalars = {0};
    struct WithStrings strings = {0};
    struct OuterStruct nested_struct = {0};
    struct UnionContainer unions = {0};
    struct PointerFest pointers = {0};
    struct ArrayCollection* arrays_ptr = NULL;
    struct WithCallbacks callbacks = {0};
    struct LangLikeStruct lang_struct = {0};
    struct ComplexStruct complex = {0};
    union NestedUnion recursive_union = {0};
    struct UltimateContainer ultimate = {0};
    
    /* Take addresses of all instances */
    void* addresses[] = {
        &scalars,
        &strings,
        &nested_struct,
        &unions,
        &pointers,
        &arrays_ptr,
        &callbacks,
        &lang_struct,
        &complex,
        &recursive_union,
        &ultimate
    };
    
    /* Compute sizes of all types */
    size_t sizes[] = {
        sizeof(struct ScalarsOnly),
        sizeof(struct WithStrings),
        sizeof(struct OuterStruct),
        sizeof(struct UnionContainer),
        sizeof(struct PointerFest),
        sizeof(struct ArrayCollection),
        sizeof(struct WithCallbacks),
        sizeof(struct LangLikeStruct),
        sizeof(struct ComplexStruct),
        sizeof(union NestedUnion),
        sizeof(struct UltimateContainer),
        sizeof(int*),
        sizeof(callback_func),
        sizeof(int[10]),
        sizeof(struct SimpleStruct*),
        sizeof(union SimpleUnion*)
    };
    
    /* Force computation with all types to prevent optimization */
    size_t total_size = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_size += sizes[i];
    }
    
    /* Touch all addresses */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        touch_memory(addresses[i]);
    }
    
    /* Create some pointer relationships */
    pointers.forward_ptr = &complex;
    ultimate.self = &ultimate;
    recursive_union.as_metadata.recursive_ptr = &recursive_union;
    
    /* Compute a checksum using the types */
    size_t checksum = compute_checksum(&ultimate, sizeof(ultimate));
    checksum += compute_checksum(&complex, sizeof(complex));
    
    /* Print something to ensure code isn't optimized away */
    printf("Type analysis test:\n");
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Checksum: %zu\n", checksum);
    printf("Address of complex struct: %p\n", (void*)&complex);
    printf("Address of ultimate container: %p\n", (void*)&ultimate);
    
    /* Reference all type kinds through operations */
    printf("\nType kind references:\n");
    printf("- Scalars: %zu bytes\n", sizeof(scalars.int_field));
    printf("- Strings: %p\n", (void*)strings.static_string);
    printf("- Structs: %zu\n", sizeof(struct SimpleStruct));
    printf("- Unions: %zu\n", sizeof(union SimpleUnion));
    printf("- Pointers: %zu\n", sizeof(int*));
    printf("- Arrays: %zu\n", sizeof(int[10]));
    printf("- Callbacks: %zu\n", sizeof(callback_func));
    
    return 0;
}
