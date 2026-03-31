/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * When processed by GCC's gengtype utility during a build,
 * it should trigger counts for:
 *   TYPE_SCALAR, TYPE_STRING, TYPE_STRUCT, TYPE_USER_STRUCT,
 *   TYPE_UNION, TYPE_POINTER, TYPE_ARRAY, TYPE_CALLBACK,
 *   TYPE_LANG_STRUCT, and potentially TYPE_UNDEFINED
 */

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation macro */
#define GTY(x) 

#include <stdio.h>
#include <stddef.h>
#include <string.h>

/* Prevent optimization from removing our test structures */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* ========== Complex Type Definitions ========== */

/* Basic scalar types - TYPE_SCALAR */
typedef struct GTY(()) ScalarContainer {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    unsigned uint_field;
} ScalarContainer;

/* String type - TYPE_STRING */
typedef struct GTY(()) StringContainer {
    const char* string_field;
    char* mutable_string;
    const char* const_string_array[3];
} StringContainer;

/* Nested struct - TYPE_STRUCT */
typedef struct GTY(()) InnerStruct {
    int inner_data;
    float inner_float;
} InnerStruct;

typedef struct GTY(()) OuterStruct {
    InnerStruct nested;
    InnerStruct* nested_ptr;
    struct GTY(()) {
        int anonymous_member;
        float anonymous_float;
    };
} OuterStruct;

/* User struct - TYPE_USER_STRUCT */
struct GTY(()) ForwardDeclared;  /* Forward declaration */

typedef struct GTY(()) UserStruct {
    struct ForwardDeclared* forward_ptr;
    void* opaque_data;
} UserStruct;

struct GTY(()) ForwardDeclared {
    UserStruct* back_ref;
    int data;
};

/* Union types - TYPE_UNION */
typedef union GTY(()) SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
} SimpleUnion;

typedef struct GTY(()) UnionContainer {
    union GTY(()) {
        int variant_a;
        double variant_b;
        struct GTY(()) {
            char nested_char;
            short nested_short;
        } variant_c;
    } data;
    int discriminator;
} UnionContainer;

/* Pointer types - TYPE_POINTER */
typedef struct GTY(()) PointerNetwork {
    /* Self-referential pointer */
    struct PointerNetwork* self_ptr;
    
    /* Pointer to different type */
    ScalarContainer* scalar_ptr;
    
    /* Pointer to pointer */
    void** void_ptr_ptr;
    
    /* Function pointer */
    int (*compare_func)(const void*, const void*);
    
    /* Pointer array */
    InnerStruct* struct_ptr_array[5];
} PointerNetwork;

/* Array types - TYPE_ARRAY */
typedef struct GTY(()) ArrayContainer {
    /* Fixed-size arrays */
    int int_array[10];
    float float_array[5][3];  /* Multi-dimensional */
    
    /* Array of structs */
    InnerStruct struct_array[4];
    
    /* Flexible array member (C99) */
    char flexible_array[];
} ArrayContainer;

/* Callback types - TYPE_CALLBACK */
typedef int (*BinaryOp)(int, int);
typedef void (*Callback)(void* data, int result);

typedef struct GTY(()) CallbackContainer {
    BinaryOp operation;
    Callback on_complete;
    void* user_data;
    
    /* Array of function pointers */
    int (*operations[5])(int, int);
} CallbackContainer;

/* Language-specific struct - TYPE_LANG_STRUCT */
/* Simulating GCC's language-specific structures */
typedef struct GTY(()) LangStructBase {
    int lang_specific_tag;
    void* lang_data;
} LangStructBase;

typedef struct GTY(()) CppLangStruct {
    LangStructBase base;
    const char* mangled_name;
    void* template_args;
} CppLangStruct;

typedef struct GTY(()) JavaLangStruct {
    LangStructBase base;
    void* class_ref;
    int* interface_table;
} JavaLangStruct;

/* Complex nested structure combining all types */
typedef struct GTY(()) MegaStruct {
    /* Scalar fields */
    int id;
    double weight;
    
    /* String field */
    const char* name;
    
    /* Nested struct */
    InnerStruct inner;
    
    /* Union field */
    SimpleUnion data_union;
    
    /* Pointer fields */
    struct MegaStruct* next;
    void** data_table;
    
    /* Array field */
    int scores[8];
    
    /* Callback field */
    Callback notify;
    
    /* Language-specific struct */
    CppLangStruct* cpp_info;
    
    /* Self-referential array */
    struct MegaStruct* children[4];
    
    /* Variable length array pointer */
    int* dynamic_array;
    
    /* Bitfield (scalar but different representation) */
    unsigned int flags : 4;
    unsigned int status : 2;
} MegaStruct;

/* ========== Function Definitions ========== */

/* External function to prevent optimization */
NOINLINE static void use_pointer(void* ptr) {
    /* Volatile to prevent optimization */
    volatile int sink = 0;
    if (ptr) sink = 1;
    (void)sink;
}

NOINLINE static size_t compute_checksum(void) {
    size_t sum = 0;
    
    /* Take sizeof all our types */
    sum += sizeof(ScalarContainer);
    sum += sizeof(StringContainer);
    sum += sizeof(InnerStruct);
    sum += sizeof(OuterStruct);
    sum += sizeof(UserStruct);
    sum += sizeof(struct ForwardDeclared);
    sum += sizeof(SimpleUnion);
    sum += sizeof(UnionContainer);
    sum += sizeof(PointerNetwork);
    sum += sizeof(ArrayContainer);
    sum += sizeof(CallbackContainer);
    sum += sizeof(LangStructBase);
    sum += sizeof(CppLangStruct);
    sum += sizeof(JavaLangStruct);
    sum += sizeof(MegaStruct);
    
    return sum;
}

/* Callback function implementations */
static int add_numbers(int a, int b) { return a + b; }
static int multiply_numbers(int a, int b) { return a * b; }
static void print_result(void* data, int result) {
    if (data) printf("Result: %d\n", result);
}

/* ========== Main Function ========== */

int main(void) {
    /* Declare instances of all complex types */
    ScalarContainer scalars = {0};
    StringContainer strings = {0};
    OuterStruct outer = {0};
    UserStruct user_struct = {0};
    struct ForwardDeclared forward = {0};
    SimpleUnion simple_union = {0};
    UnionContainer union_container = {0};
    PointerNetwork pointers = {0};
    
    /* For ArrayContainer with flexible array, we need dynamic allocation */
    size_t array_size = sizeof(ArrayContainer) + 20 * sizeof(char);
    ArrayContainer* arrays = (ArrayContainer*)malloc(array_size);
    
    CallbackContainer callbacks = {0};
    CppLangStruct cpp_struct = {0};
    JavaLangStruct java_struct = {0};
    MegaStruct mega = {0};
    
    /* Initialize some data */
    scalars.int_field = 42;
    scalars.float_field = 3.14159f;
    
    strings.string_field = "Hello, gengtype!";
    strings.mutable_string = strdup("Mutable string");
    
    outer.nested.inner_data = 100;
    outer.nested_ptr = &outer.nested;
    outer.anonymous_member = 999;
    
    user_struct.forward_ptr = &forward;
    forward.back_ref = &user_struct;
    forward.data = 12345;
    
    simple_union.as_int = 65536;
    union_container.data.variant_b = 2.71828;
    union_container.discriminator = 2;
    
    pointers.self_ptr = &pointers;
    pointers.scalar_ptr = &scalars;
    pointers.compare_func = (int (*)(const void*, const void*))strcmp;
    
    if (arrays) {
        for (int i = 0; i < 10; i++) {
            arrays->int_array[i] = i * i;
        }
        strcpy(arrays->flexible_array, "Flexible!");
    }
    
    callbacks.operation = add_numbers;
    callbacks.on_complete = print_result;
    callbacks.operations[0] = add_numbers;
    callbacks.operations[1] = multiply_numbers;
    
    cpp_struct.base.lang_specific_tag = 1;
    cpp_struct.mangled_name = "_ZN4testE";
    
    java_struct.base.lang_specific_tag = 2;
    
    mega.id = 1;
    mega.name = "MegaStruct Instance";
    mega.inner.inner_data = 42;
    mega.data_union.as_float = 3.14f;
    mega.next = &mega;  /* Self-reference */
    mega.notify = print_result;
    mega.cpp_info = &cpp_struct;
    for (int i = 0; i < 8; i++) {
        mega.scores[i] = i * 10;
    }
    
    /* Take addresses of all instances to ensure they're used */
    use_pointer(&scalars);
    use_pointer(&strings);
    use_pointer(&outer);
    use_pointer(&user_struct);
    use_pointer(&forward);
    use_pointer(&simple_union);
    use_pointer(&union_container);
    use_pointer(&pointers);
    use_pointer(arrays);
    use_pointer(&callbacks);
    use_pointer(&cpp_struct);
    use_pointer(&java_struct);
    use_pointer(&mega);
    
    /* Take addresses of members */
    use_pointer(&scalars.int_field);
    use_pointer(&strings.string_field);
    use_pointer(&outer.nested);
    use_pointer(&user_struct.forward_ptr);
    use_pointer(&simple_union.as_int);
    use_pointer(&pointers.self_ptr);
    if (arrays) use_pointer(&arrays->int_array[0]);
    use_pointer(&callbacks.operation);
    use_pointer(&cpp_struct.mangled_name);
    use_pointer(&mega.scores);
    
    /* Compute sizeof all types */
    size_t checksum = compute_checksum();
    
    /* Use function pointers */
    if (callbacks.operation) {
        int result = callbacks.operation(10, 20);
        callbacks.on_complete(&callbacks, result);
    }
    
    /* Print checksum to prevent optimization */
    printf("Type checksum: %zu\n", checksum);
    printf("ScalarContainer size: %zu\n", sizeof(ScalarContainer));
    printf("MegaStruct size: %zu\n", sizeof(MegaStruct));
    
    /* Cleanup */
    if (strings.mutable_string) free(strings.mutable_string);
    if (arrays) free(arrays);
    
    return 0;
}
