/* test_gengtype_coverage.c
 * Complex type definitions to exercise gengtype.cc type enumeration logic
 * Specifically targets lines 182-213 switch statement
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC this marks GC roots */
#define GTY(x) 

/* Forward declarations to create pointer cycles */
struct ForwardDecl;
union ForwardUnion;

/* ========== TYPE_SCALAR definitions ========== */
GTY(())
struct ScalarsOnly {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    short short_field;
    unsigned uint_field;
};

/* ========== TYPE_STRING definitions ========== */
GTY(())
struct WithStrings {
    const char* static_string;
    char* dynamic_string;
    const char* const constant_string;
    char fixed_string[64];
};

/* ========== TYPE_STRUCT definitions ========== */
GTY(())
struct NestedStruct {
    int depth;
    struct InnerStruct {
        int inner_value;
        struct DeeperStruct {
            int deepest_value;
        } deeper;
    } inner;
};

/* ========== TYPE_USER_STRUCT definitions ========== */
/* User-defined struct with typedef */
typedef GTY(()) struct UserDefined {
    int user_id;
    char user_name[32];
    struct UserDefined* next;  /* Self-reference for TYPE_POINTER */
} UserType;

/* ========== TYPE_UNION definitions ========== */
GTY(())
union VariantData {
    int as_int;
    float as_float;
    double as_double;
    char* as_string;
    struct {
        int tag;
        union {
            int i;
            float f;
        } payload;
    } tagged;
};

/* ========== TYPE_POINTER definitions ========== */
GTY(())
struct PointerFest {
    /* Various pointer types */
    int* int_ptr;
    void* void_ptr;
    struct ForwardDecl* forward_ptr;
    union ForwardUnion* union_ptr;
    
    /* Function pointer (TYPE_CALLBACK) */
    int (*compare_func)(const void*, const void*);
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to pointer */
    char** string_array;
    
    /* Const pointer */
    const int* const_ptr;
    
    /* Pointer to struct with GTY */
    struct ScalarsOnly* scalar_struct_ptr;
};

/* ========== TYPE_ARRAY definitions ========== */
GTY(())
struct ArrayContainer {
    /* Fixed size arrays */
    int fixed_array[100];
    char char_matrix[10][20];
    struct ScalarsOnly struct_array[5];
    
    /* Zero-length array (GCC extension) */
    int flexible_array[];
};

/* ========== TYPE_CALLBACK definitions ========== */
/* Function pointer types */
typedef int (*BinaryOp)(int, int);
typedef void (*Callback)(void* context, int result);

GTY(())
struct WithCallbacks {
    BinaryOp arithmetic_op;
    Callback completion_cb;
    int (*custom_sort)(void**, size_t);
    void (*no_args_void)(void);
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* Language-specific struct - in GCC this would be marked specially */
GTY(())
struct LangSpecific {
    /* Simulating tree nodes or other GCC internal types */
    void* lang_specific_data;
    int lang_tag;
    struct LangSpecific* lang_chain;
};

/* ========== Complex nested type ========== */
GTY(())
struct UltimateType {
    /* Contains everything */
    struct ScalarsOnly scalars;
    struct WithStrings strings;
    union VariantData variant;
    struct PointerFest pointers;
    struct ArrayContainer arrays;
    struct WithCallbacks callbacks;
    struct LangSpecific lang_struct;
    
    /* Self-reference cycle */
    struct UltimateType* next;
    struct UltimateType* prev;
    
    /* Anonymous union inside struct */
    union {
        int anon_int;
        float anon_float;
    };
    
    /* Bitfields (scalar but interesting) */
    unsigned int flags : 8;
    unsigned int mode : 4;
    
    /* Array of function pointers */
    Callback callback_array[4];
};

/* ========== Forward declarations now defined ========== */
GTY(())
struct ForwardDecl {
    int value;
    struct UltimateType* link;
};

GTY(())
union ForwardUnion {
    int as_int;
    struct ForwardDecl* as_struct;
};

/* ========== External function to prevent optimization ========== */
#ifdef __GNUC__
__attribute__((noinline, used))
#else
volatile
#endif
static size_t compute_checksum(void* ptr, size_t size) {
    /* Simple operation to ensure types are referenced */
    return (size_t)ptr ^ size;
}

/* ========== Main driver ========== */
int main(void) {
    /* Declare instances of all complex types */
    struct ScalarsOnly scalars_instance = {0};
    struct WithStrings strings_instance = {
        .static_string = "Hello",
        .dynamic_string = NULL,
        .constant_string = "World",
        .fixed_string = "Test"
    };
    
    struct NestedStruct nested_instance = {
        .depth = 3,
        .inner = {
            .inner_value = 42,
            .deeper = {
                .deepest_value = 999
            }
        }
    };
    
    UserType user_instance = {
        .user_id = 1001,
        .user_name = "TestUser",
        .next = NULL
    };
    
    union VariantData variant_instance;
    variant_instance.as_int = 42;
    
    struct PointerFest pointers_instance = {0};
    struct ArrayContainer arrays_instance = {0};
    struct WithCallbacks callbacks_instance = {0};
    struct LangSpecific lang_instance = {0};
    struct UltimateType ultimate_instance = {0};
    struct ForwardDecl forward_instance = {0};
    union ForwardUnion forward_union_instance;
    
    /* Create pointer cycles */
    user_instance.next = &user_instance;
    ultimate_instance.next = &ultimate_instance;
    ultimate_instance.prev = &ultimate_instance;
    
    /* Take addresses of all instances */
    void* addresses[] = {
        &scalars_instance,
        &strings_instance,
        &nested_instance,
        &user_instance,
        &variant_instance,
        &pointers_instance,
        &arrays_instance,
        &callbacks_instance,
        &lang_instance,
        &ultimate_instance,
        &forward_instance,
        &forward_union_instance
    };
    
    /* Compute sizeof all types */
    size_t sizes[] = {
        sizeof(struct ScalarsOnly),
        sizeof(struct WithStrings),
        sizeof(struct NestedStruct),
        sizeof(UserType),
        sizeof(union VariantData),
        sizeof(struct PointerFest),
        sizeof(struct ArrayContainer),
        sizeof(struct WithCallbacks),
        sizeof(struct LangSpecific),
        sizeof(struct UltimateType),
        sizeof(struct ForwardDecl),
        sizeof(union ForwardUnion),
        sizeof(int*),
        sizeof(int(*)[10]),
        sizeof(int(*)(int, int))
    };
    
    /* Force compiler to consider all types */
    size_t total_checksum = 0;
    
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        total_checksum += compute_checksum(addresses[i], sizeof(addresses[i]));
    }
    
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_checksum ^= sizes[i];
    }
    
    /* Access nested members to ensure full type traversal */
    total_checksum += nested_instance.inner.deeper.deepest_value;
    total_checksum += ultimate_instance.scalars.int_field;
    total_checksum += (size_t)strings_instance.static_string;
    
    /* Print something to prevent complete optimization */
    printf("Type analysis checksum: %zu\n", total_checksum);
    printf("Pointer size: %zu, Int size: %zu\n", 
           sizeof(void*), sizeof(int));
    
    /* Reference function pointers to ensure TYPE_CALLBACK */
    if (callbacks_instance.arithmetic_op) {
        printf("Callback present\n");
    }
    
    return (int)(total_checksum % 256);
}
