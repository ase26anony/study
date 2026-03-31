/* test_gengtype_switch.c - Complex type definitions to exercise gengtype switch cases */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC this marks GC roots */
#define GTY(x) 

/* Forward declarations to create pointer cycles */
struct ForwardDecl;
union ForwardUnion;

/* TYPE_SCALAR: Basic scalar types */
typedef struct {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    unsigned uint_field;
    _Bool bool_field;
} GTY(()) ScalarStruct;

/* TYPE_STRING: String types */
typedef struct {
    const char* const_string;
    char* mutable_string;
    const char* const_array_string[3];
} GTY(()) StringStruct;

/* TYPE_STRUCT: Regular structure */
typedef struct {
    int x;
    double y;
    ScalarStruct nested_scalar;
} GTY(()) RegularStruct;

/* TYPE_USER_STRUCT: User-defined structure */
struct GTY(()) UserDefined {
    RegularStruct regular;
    void* opaque_data;
    size_t size;
};

/* TYPE_UNION: Union types */
typedef union {
    int as_int;
    double as_double;
    void* as_pointer;
    struct {
        int a;
        int b;
    } as_struct;
} GTY(()) ComplexUnion;

/* TYPE_POINTER: Various pointer types */
typedef struct {
    int* int_ptr;
    double* double_ptr;
    ScalarStruct* struct_ptr;
    void** void_ptr_ptr;
    int (*func_ptr)(int, char*);
    const volatile int* cv_ptr;
    struct ForwardDecl* forward_ptr;
    union ForwardUnion* union_forward_ptr;
} GTY(()) PointerStruct;

/* TYPE_ARRAY: Array types */
typedef struct {
    int fixed_array[10];
    double multi_dim[3][4][5];
    char* pointer_array[8];
    ScalarStruct struct_array[5];
    int flexible_array[];
} GTY(()) ArrayStruct;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*CallbackFunc)(int, void*);
typedef void (*ComplexCallback)(struct UserDefined*, ArrayStruct*, CallbackFunc);

typedef struct {
    CallbackFunc simple_callback;
    ComplexCallback complex_callback;
    int (*array_of_callbacks[5])(void);
} GTY(()) CallbackStruct;

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
typedef struct GTY(()) {
    void* lang_specific;
    int lang_tag;
    union {
        int int_val;
        void* ptr_val;
    } lang_union;
} LangStruct;

/* Complete the forward declarations */
struct GTY(()) ForwardDecl {
    int value;
    struct ForwardDecl* next;
    PointerStruct* ptr_struct;
};

union GTY(()) ForwardUnion {
    struct ForwardDecl* as_struct;
    ArrayStruct* as_array;
    LangStruct* as_lang;
};

/* TYPE_UNDEFINED: Create a chain that could lead to undefined */
typedef struct GTY(()) UndefinedContainer {
    void* unknown_ptr;
    struct UndefinedContainer* self_ref;
} UndefinedContainer;

/* Complex nested structure combining all types */
typedef struct GTY(()) MasterStruct {
    ScalarType scalar_part;
    StringStruct string_part;
    RegularStruct regular_part;
    struct UserDefined user_part;
    ComplexUnion union_part;
    PointerStruct pointer_part;
    ArrayStruct* array_part;
    CallbackStruct callback_part;
    LangStruct lang_part;
    struct ForwardDecl* forward_part;
    union ForwardUnion forward_union_part;
    UndefinedContainer undefined_part;
    
    /* Self-reference for cycles */
    struct MasterStruct* next;
    struct MasterStruct* prev;
    
    /* Array of various types */
    union {
        int i;
        void* p;
    } variant_array[7];
} MasterStruct;

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void* ptr) {
    return (size_t)ptr ^ 0x12345678;
}

/* Function using callback */
__attribute__((noinline))
int sample_callback(int x, void* data) {
    return x * 2 + (int)((size_t)data & 0xFF);
}

/* Main function to reference all types */
int main() {
    /* Declare instances of all complex types */
    volatile ScalarStruct scalar_instance = {0};
    volatile StringStruct string_instance = {"Hello", "World", {"A", "B", "C"}};
    volatile RegularStruct regular_instance = {42, 3.14, {1, 'X', 2.0f, 3.0, 4L, 5, 6u, 1}};
    volatile struct UserDefined user_instance = {regular_instance, NULL, sizeof(regular_instance)};
    volatile ComplexUnion union_instance = {.as_double = 2.71828};
    volatile PointerStruct pointer_instance = {0};
    volatile ArrayStruct* array_ptr = NULL;
    volatile CallbackStruct callback_instance = {sample_callback, NULL, {NULL}};
    volatile LangStruct lang_instance = {NULL, 99, {.int_val = 100}};
    volatile struct ForwardDecl forward_instance = {777, NULL, NULL};
    volatile union ForwardUnion forward_union_instance = {&forward_instance};
    volatile UndefinedContainer undefined_instance = {NULL, NULL};
    volatile MasterStruct master_instance = {0};
    
    /* Take addresses to ensure types are considered */
    void* addresses[] = {
        &scalar_instance,
        &string_instance,
        &regular_instance,
        &user_instance,
        &union_instance,
        &pointer_instance,
        &array_ptr,
        &callback_instance,
        &lang_instance,
        &forward_instance,
        &forward_union_instance,
        &undefined_instance,
        &master_instance
    };
    
    /* Compute sizeof all types */
    size_t sizes[] = {
        sizeof(ScalarStruct),
        sizeof(StringStruct),
        sizeof(RegularStruct),
        sizeof(struct UserDefined),
        sizeof(ComplexUnion),
        sizeof(PointerStruct),
        sizeof(ArrayStruct),
        sizeof(CallbackStruct),
        sizeof(LangStruct),
        sizeof(struct ForwardDecl),
        sizeof(union ForwardUnion),
        sizeof(UndefinedContainer),
        sizeof(MasterStruct),
        sizeof(int*),
        sizeof(CallbackFunc),
        sizeof(int[10]),
        sizeof(double[3][4][5])
    };
    
    /* Perform operations to prevent optimization */
    size_t total_size = 0;
    size_t address_sum = 0;
    
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_size += sizes[i];
    }
    
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        address_sum += compute_checksum((void*)addresses[i]);
    }
    
    /* Use function pointers */
    callback_instance.simple_callback = sample_callback;
    int callback_result = callback_instance.simple_callback(42, (void*)total_size);
    
    /* Create pointer cycles */
    pointer_instance.forward_ptr = &forward_instance;
    forward_instance.ptr_struct = &pointer_instance;
    master_instance.next = &master_instance;
    master_instance.prev = &master_instance;
    undefined_instance.self_ref = &undefined_instance;
    
    /* Print something to ensure code isn't optimized away */
    printf("Type analysis test:\n");
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Address checksum: %zu\n", address_sum);
    printf("Callback result: %d\n", callback_result);
    printf("Pointer to scalar struct: %p\n", (void*)&scalar_instance.int_field);
    printf("Offset of string field: %zu\n", offsetof(StringStruct, mutable_string));
    
    return 0;
}
