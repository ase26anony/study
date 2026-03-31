/* gengtype-test.c - Complex type definitions to exercise gengtype type enumeration */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC build this would be the actual GTY marker */
#define GTY(x)

/* Forward declarations to create pointer cycles */
struct ForwardDecl;
union ForwardUnion;

/* ========== TYPE_SCALAR definitions ========== */
GTY(())
struct Scalars {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    unsigned uint_field;
    _Bool bool_field;
    int8_t int8_field;
    int64_t int64_field;
};

/* ========== TYPE_STRING definitions ========== */
GTY(())
struct WithStrings {
    const char* string_ptr;
    char string_array[32];
    const char* const_string;
    char* mutable_string;
};

/* ========== TYPE_STRUCT definitions ========== */
GTY(())
struct InnerStruct {
    int inner_data;
    double inner_value;
};

GTY(())
struct OuterStruct {
    struct InnerStruct nested;
    struct InnerStruct* nested_ptr;
    int outer_data;
};

/* ========== TYPE_USER_STRUCT definitions ========== */
/* User-defined struct types with complex nesting */
typedef struct InnerStruct UserInner;

GTY(())
struct UserStructWrapper {
    UserInner user_field;
    UserInner* user_ptr;
};

/* ========== TYPE_UNION definitions ========== */
GTY(())
union DataUnion {
    int as_int;
    double as_double;
    void* as_ptr;
    struct {
        int tag;
        char data[16];
    } as_struct;
};

GTY(())
struct UnionContainer {
    union DataUnion data;
    union {
        int variant_a;
        struct InnerStruct variant_b;
    } tagged_union;
};

/* ========== TYPE_POINTER definitions ========== */
GTY(())
struct PointerFest {
    int* int_ptr;
    void* void_ptr;
    struct OuterStruct* struct_ptr;
    union DataUnion* union_ptr;
    char** double_ptr;
    int (*func_ptr)(int, char);
    void (*void_func_ptr)(void);
    struct ForwardDecl* forward_ptr;
    union ForwardUnion* forward_union_ptr;
    int (*array_of_func_ptrs[5])(void);
};

/* ========== TYPE_ARRAY definitions ========== */
GTY(())
struct ArrayTypes {
    int simple_array[10];
    struct InnerStruct struct_array[5];
    union DataUnion union_array[8];
    char* pointer_array[12];
    int multi_dim_array[3][4][5];
    int flexible_array[];
};

GTY(())
struct WithFlexArray {
    int count;
    char data[];  /* Flexible array member */
};

/* ========== TYPE_CALLBACK definitions ========== */
/* Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, void*);
typedef struct InnerStruct* (*FactoryFunc)(int);

GTY(())
struct CallbackContainer {
    Comparator compare;
    CallbackFunc callback;
    FactoryFunc factory;
    void (*handlers[4])(void);
};

/* ========== TYPE_LANG_STRUCT definitions ========== */
/* Simulating language-specific struct types */
GTY(())
struct LangSpecificBase {
    int lang_tag;
    void* lang_data;
};

/* ========== Complex nested type with all kinds ========== */
GTY(())
struct MegaType {
    /* TYPE_SCALAR */
    int id;
    double value;
    
    /* TYPE_STRING */
    const char* name;
    char buffer[256];
    
    /* TYPE_STRUCT */
    struct InnerStruct inner;
    
    /* TYPE_USER_STRUCT */
    UserInner user_type;
    
    /* TYPE_UNION */
    union DataUnion data;
    
    /* TYPE_POINTER */
    struct MegaType* self_ptr;
    void** void_double_ptr;
    int (*operations[3])(struct MegaType*);
    
    /* TYPE_ARRAY */
    struct InnerStruct structs[5];
    int matrix[3][3];
    
    /* TYPE_CALLBACK */
    CallbackFunc on_event;
    
    /* Nested anonymous struct/union */
    struct {
        int anonymous_data;
        union {
            int anon_int;
            double anon_double;
        } anon_union;
    } anonymous;
    
    /* Pointer to forward declared type */
    struct ForwardDecl* future;
};

/* ========== Forward declared types ========== */
GTY(())
struct ForwardDecl {
    int data;
    struct MegaType* link_back;
};

GTY(())
union ForwardUnion {
    int as_int;
    struct ForwardDecl* as_struct;
};

/* ========== TYPE_UNDEFINED simulation ========== */
/* Incomplete/opaque type */
struct OpaqueType;
GTY(())
struct HasOpaque {
    struct OpaqueType* opaque_ptr;  /* Might trigger TYPE_UNDEFINED */
    void* unknown_ptr;
};

/* ========== Function to prevent optimization ========== */
__attribute__((noinline)) 
size_t compute_checksum(void* ptr, size_t size) {
    /* Simple operation to prevent dead code elimination */
    volatile size_t result = 0;
    unsigned char* bytes = (unsigned char*)ptr;
    
    /* Just touch the memory slightly */
    if (size > 0) {
        result = bytes[0] + bytes[size - 1];
    }
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    /* Declare instances of all complex types */
    struct Scalars scalars = {0};
    struct WithStrings strings = {0};
    struct OuterStruct outer = {0};
    struct UserStructWrapper user_wrapper = {0};
    union DataUnion data_union;
    struct UnionContainer union_container = {0};
    struct PointerFest pointers = {0};
    struct ArrayTypes arrays = {0};
    struct WithFlexArray* flex_array = NULL;
    struct CallbackContainer callbacks = {0};
    struct LangSpecificBase lang_struct = {0};
    struct MegaType mega = {0};
    struct ForwardDecl forward = {0};
    struct HasOpaque has_opaque = {0};
    
    /* Take addresses to ensure types are considered */
    volatile void* addresses[] = {
        &scalars, &strings, &outer, &user_wrapper,
        &data_union, &union_container, &pointers,
        &arrays, &flex_array, &callbacks,
        &lang_struct, &mega, &forward, &has_opaque
    };
    
    /* Compute sizeof all types */
    size_t sizes[] = {
        sizeof(struct Scalars),
        sizeof(struct WithStrings),
        sizeof(struct OuterStruct),
        sizeof(struct UserStructWrapper),
        sizeof(union DataUnion),
        sizeof(struct UnionContainer),
        sizeof(struct PointerFest),
        sizeof(struct ArrayTypes),
        sizeof(struct CallbackContainer),
        sizeof(struct LangSpecificBase),
        sizeof(struct MegaType),
        sizeof(struct ForwardDecl),
        sizeof(struct HasOpaque),
        sizeof(int*),
        sizeof(int(*)(void)),
        sizeof(char[10]),
        sizeof(struct InnerStruct*),
        sizeof(union DataUnion*)
    };
    
    /* Perform operations to prevent optimization */
    size_t total_size = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_size += sizes[i];
    }
    
    /* Use compute_checksum on some instances */
    size_t checksum = 0;
    checksum += compute_checksum(&scalars, sizeof(scalars));
    checksum += compute_checksum(&mega, sizeof(mega));
    checksum += compute_checksum(&outer, sizeof(outer));
    
    /* Create pointer cycles */
    mega.self_ptr = &mega;
    forward.link_back = &mega;
    
    /* Access various members to ensure they're used */
    scalars.int_field = 42;
    strings.string_ptr = "Hello, gengtype!";
    outer.nested.inner_data = 100;
    data_union.as_int = 0xDEADBEEF;
    arrays.simple_array[0] = 1;
    arrays.simple_array[9] = 10;
    
    /* Print results to prevent complete optimization */
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Checksum: %zu\n", checksum);
    printf("Address array count: %zu\n", sizeof(addresses)/sizeof(addresses[0]));
    
    /* Demonstrate pointer arithmetic and array access */
    int* ptr = arrays.simple_array;
    for (int i = 0; i < 10; i++) {
        ptr[i] = i * i;
    }
    
    /* Use function pointers */
    int (*dummy_func)(int) = NULL;
    dummy_func = (int (*)(int))compute_checksum;
    
    /* Create a complex expression involving all types */
    size_t complex_expr = 
        sizeof(scalars) + 
        (size_t)&scalars.int_field - (size_t)&scalars +
        (size_t)strings.string_ptr % 1024 +
        outer.nested.inner_data * 2;
    
    printf("Complex expression result: %zu\n", complex_expr);
    
    return 0;
}
