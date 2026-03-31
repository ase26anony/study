/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Incomplete/undefined type */
struct forward_declared_struct;    /* Forward declaration */

/* ========== TYPE_SCALAR - All fundamental types ========== */
volatile char global_char = 'A';
volatile signed char global_schar = -1;
volatile unsigned char global_uchar = 255;
volatile short global_short = -32768;
volatile unsigned short global_ushort = 65535;
volatile int global_int = -2147483648;
volatile unsigned int global_uint = 4294967295U;
volatile long global_long = -9223372036854775807L;
volatile unsigned long global_ulong = 18446744073709551615UL;
volatile long long global_llong = -9223372036854775807LL;
volatile unsigned long long global_ullong = 18446744073709551615ULL;
volatile float global_float = 3.1415926535f;
volatile double global_double = 2.718281828459045;
volatile long double global_ldouble = 1.6180339887498948482L;
volatile _Bool global_bool = 1;
volatile float _Complex global_cfloat = 1.0f + 2.0f * I;
volatile double _Complex global_cdouble = 3.0 + 4.0 * I;

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, GCC GGC!";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable string data";

/* ========== TYPE_LANG_STRUCT - GCC-specific types ========== */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Packed and aligned structures */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(64))) AlignedStruct {
    double data[8];
    int tag;
};

/* ========== TYPE_CALLBACK - Function pointers and typedefs ========== */
typedef void (*simple_callback)(void);
typedef int (*processor_func)(const char*, size_t);
typedef void (*event_handler)(int event_id, void* user_data);
typedef double (*math_func)(double, double);

/* Struct with function pointer members */
struct Plugin {
    const char* name;
    int (*init)(void* context);
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
    event_handler on_event;
};

/* Union with function pointer */
union CallbackUnion {
    simple_callback cb_simple;
    processor_func cb_complex;
    void* (*cb_generic)(void*);
};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Simple struct */
struct Point {
    int x;
    int y;
    int z;
};

/* Nested anonymous struct */
struct Container {
    struct {
        int a;
        char b;
        float c;
    } inner;
    
    union {
        long as_long;
        double as_double;
        void* as_ptr;
    } data;
    
    struct Point point;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;  /* Padding */
    signed int value : 10;
    unsigned int mode : 6;
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t length;
    int data[];
};

/* Complex nested struct */
struct TreeNode {
    int value;
    struct TreeNode* left;
    struct TreeNode* right;
    struct TreeNode* parent;
    
    union {
        struct {
            char color;
            unsigned char depth;
        } metadata;
        unsigned short flags;
    } info;
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    long as_long;
    double as_double;
    void* as_ptr;
    const char* as_string;
    
    struct {
        short type;
        char data[8];
    } as_packed;
    
    struct {
        short len;
        char buf[];
    } as_flexible;
};

union TaggedUnion {
    struct {
        int type;
        union {
            int i;
            float f;
            void* p;
        } value;
    } tagged;
    
    long long as_llong;
    double as_double[2];
};

/* ========== TYPE_ARRAY / TYPE_POINTER - Complex combinations ========== */
/* Multi-dimensional arrays */
int matrix_2d[10][20];
float matrix_3d[5][5][5];

/* Array of structs */
struct Point point_array[100];
struct Container container_array[50];

/* Array of pointers */
struct TreeNode* node_ptr_array[64];
void* void_ptr_array[32];

/* Pointer to array */
int (*ptr_to_array)[20];
float (*ptr_to_3d_array)[5][5];

/* Multi-level pointers */
int*** triple_ptr;
struct Container**** quad_ptr;

/* Array of function pointers */
math_func math_funcs[] = {NULL, NULL, NULL};

/* Complex function pointer with array parameter */
int (*signal_processor)(float[][256], int**, size_t);

/* ========== Global instances ========== */
struct Plugin plugin_registry[3];
union Variant global_variants[10];
float32x8_t global_vector1, global_vector2;
struct DynamicArray* global_dyn_array = NULL;

/* ========== Function definitions ========== */
static int plugin_init_default(void* context) {
    (void)context;
    return 0;
}

static void plugin_process_default(int data) {
    global_int += data;
}

static void plugin_cleanup_default(struct Plugin* self) {
    if (self) {
        /* Do nothing for test */
    }
}

static void event_handler_example(int event_id, void* user_data) {
    *(int*)user_data = event_id;
}

static double add_func(double a, double b) { return a + b; }
static double mul_func(double a, double b) { return a * b; }

/* Function using all complex types */
void process_complex_types(
    struct Container* containers,
    union Variant* variants,
    int*** ptr_chain,
    float32x8_t* vec_result
) {
    /* Use struct types */
    for (int i = 0; i < 5; i++) {
        containers[i].inner.a = i;
        containers[i].data.as_double = i * 3.14;
        containers[i].point.x = i * 10;
    }
    
    /* Use union types */
    variants[0].as_int = 42;
    variants[1].as_double = 3.14159;
    variants[2].as_string = "Union string";
    
    /* Use pointer chain */
    if (ptr_chain && *ptr_chain && **ptr_chain) {
        ***ptr_chain = 999;
    }
    
    /* Use vector type */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    *vec_result = __builtin_ia32_addps(vec1, vec2);
}

/* Function with callback */
int execute_with_callback(processor_func func, const char* data, size_t len) {
    if (func) {
        return func(data, len);
    }
    return -1;
}

static int sample_processor(const char* str, size_t len) {
    int sum = 0;
    for (size_t i = 0; i < len && str[i]; i++) {
        sum += str[i];
    }
    return sum;
}

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    int callback_data = 0;
    
    /* Initialize plugin registry */
    for (int i = 0; i < 3; i++) {
        plugin_registry[i].name = "Test Plugin";
        plugin_registry[i].init = plugin_init_default;
        plugin_registry[i].process = plugin_process_default;
        plugin_registry[i].cleanup = plugin_cleanup_default;
        plugin_registry[i].on_event = event_handler_example;
    }
    
    /* Call function through pointer */
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init(&callback_data);
    }
    
    if (plugin_registry[0].process) {
        plugin_registry[0].process(42);
    }
    
    /* Use callback union */
    union CallbackUnion cb_union;
    cb_union.cb_complex = sample_processor;
    
    /* Execute callback */
    result += execute_with_callback(cb_union.cb_complex, "test", 4);
    
    /* Initialize math function array */
    math_funcs[0] = add_func;
    math_funcs[1] = mul_func;
    
    if (math_funcs[0]) {
        result += (int)math_funcs[0](10.5, 20.5);
    }
    
    /* Setup pointer chain */
    int value = 0;
    int* ptr1 = &value;
    int** ptr2 = &ptr1;
    int*** ptr3 = &ptr2;
    triple_ptr = ptr3;
    
    /* Setup multi-dimensional array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix_2d[i][j] = i * 100 + j;
        }
    }
    
    ptr_to_array = matrix_2d;
    
    /* Process complex types */
    float32x8_t vec_result;
    process_complex_types(container_array, global_variants, triple_ptr, &vec_result);
    
    /* Use vector result */
    float* vec_elements = (float*)&vec_result;
    for (int i = 0; i < 8; i++) {
        result += (int)vec_elements[i];
    }
    
    /* Process union with type switching */
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            global_variants[i].as_int = i * 100;
            result += global_variants[i].as_int;
        } else if (i % 3 == 1) {
            global_variants[i].as_double = i * 1.234;
            result += (int)global_variants[i].as_double;
        } else {
            global_variants[i].as_string = error_messages[i % 3];
            if (global_variants[i].as_string) {
                result += global_variants[i].as_string[0];
            }
        }
    }
    
    /* Traverse pointer chain */
    if (triple_ptr && *triple_ptr && **triple_ptr) {
        result += ***triple_ptr;
    }
    
    /* Use all scalar types in computation */
    result += global_char + global_short + global_int + (int)global_float;
    result += (int)global_cfloat + (int)global_cdouble;
    
    /* Use string types */
    for (int i = 0; error_messages[i]; i++) {
        result += error_messages[i][0];
    }
    
    /* Ensure all types are referenced to prevent optimization */
    volatile struct BitFieldStruct bfs = {0};
    bfs.flag1 = 1;
    bfs.value = 512;
    result += bfs.value;
    
    volatile struct PackedStruct ps = {'X', 12345, 6789};
    result += ps.b;
    
    volatile struct AlignedStruct as = {{0}};
    as.tag = 99;
    result += as.tag;
    
    /* Create and use a dynamic array */
    size_t dyn_size = 5;
    struct DynamicArray* dyn = (struct DynamicArray*)malloc(
        sizeof(struct DynamicArray) + dyn_size * sizeof(int));
    if (dyn) {
        dyn->length = dyn_size;
        for (size_t i = 0; i < dyn_size; i++) {
            dyn->data[i] = (int)(i * 100);
            result += dyn->data[i];
        }
        free(dyn);
    }
    
    /* Return deterministic result */
    return result % 256;  /* Ensure small, non-zero exit code */
}
