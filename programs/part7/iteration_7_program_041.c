/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward declarations ========== */
struct opaque;  /* Incomplete type - TYPE_UNDEFINED */
struct forward_declared;

/* ========== TYPE_SCALAR - All fundamental types ========== */
volatile char v_char = 'A';
volatile signed char v_schar = -1;
volatile unsigned char v_uchar = 255;
volatile short v_short = -32768;
volatile unsigned short v_ushort = 65535;
volatile int v_int = -2147483648;
volatile unsigned int v_uint = 4294967295U;
volatile long v_long = -9223372036854775807L;
volatile unsigned long v_ulong = 18446744073709551615UL;
volatile long long v_llong = -9223372036854775807LL;
volatile unsigned long long v_ullong = 18446744073709551615ULL;
volatile float v_float = 3.1415926535f;
volatile double v_double = 2.718281828459045;
volatile long double v_ldouble = 1.6180339887498948482L;
volatile _Bool v_bool = 1;
volatile _Complex float v_cfloat = 1.0f + 2.0fi;
volatile _Complex double v_cdouble = 3.0 + 4.0i;
volatile _Complex long double v_cldouble = 5.0L + 6.0Li;

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, GGC!";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable string";

/* ========== TYPE_CALLBACK - Function pointers ========== */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(int, const char*, void*);
typedef void (*event_handler)(int, void*);

/* Callback implementations */
static void callback_impl1(void) { v_int++; }
static int callback_impl2(int x, const char* s, void* p) { 
    return x + (int)((char*)p - (char*)0); 
}
static void event_handler_impl(int event, void* data) { v_char = 'B'; }

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */

/* Simple nested anonymous struct */
struct Container {
    struct {
        int a;
        char b;
        unsigned:4;  /* Unnamed bitfield */
        signed field:8;  /* Named bitfield */
    } inner;
    union {
        long x;
        double y;
        void* ptr;
    } data;
    volatile int counter;
};

/* Struct with flexible array member */
struct FAM {
    size_t length;
    int data[];
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedStruct {
    char c;
    int i;
    short s;
};

/* Struct containing function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    void (*cleanup)(struct Plugin*);
    complex_callback cb;
};

/* Complex nested struct */
struct TreeNode {
    int value;
    struct TreeNode* left;
    struct TreeNode* right;
    struct {
        unsigned visited:1;
        unsigned is_leaf:1;
        unsigned depth:6;
    } flags;
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    double as_double;
    struct {
        short len;
        char buf[16];  /* Fixed-size instead of flexible for portability */
    } as_string;
    long long as_llong;
};

/* Tagged union */
struct TaggedVariant {
    enum { INT, PTR, FLOAT, STRING } tag;
    union {
        int i;
        void* p;
        float f;
        char str[32];
    } value;
};

/* ========== TYPE_ARRAY - Multi-dimensional arrays ========== */
int matrix_2d[5][5];
int* pointer_array[10];
struct Container container_array[3][2];
int (*function_ptr_array[5])(void);

/* Three-dimensional array */
double tensor[2][3][4];

/* Array of pointers to arrays */
int* jagged[4];

/* ========== TYPE_POINTER - Complex pointer chains ========== */
int*** triple_ptr;
struct Container** container_ptr_ptr;
void (*signal_handler)(int);
int (*array_of_5_ptrs[5])(float, double);

/* Function returning pointer to array */
int (*func_returning_ptr_to_array(void))[5] {
    static int arr[5];
    return &arr;
}

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */
/* Vector types */
typedef int __attribute__((vector_size(16))) int32x4_t;
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef double __attribute__((vector_size(64))) double64x8_t;

/* SIMD struct */
struct SIMDData {
    int32x4_t vec_int;
    float32x8_t vec_float;
    double64x8_t vec_double;
};

/* Transparent union */
typedef union __attribute__((transparent_union)) TransUnion {
    int i;
    long l;
} TransUnion;

/* ========== Global instances ========== */
struct Plugin plugin_registry[3];
union Variant global_variant;
struct SIMDData simd_instance;
struct TaggedVariant tagged_instance;

/* ========== Function definitions ========== */

/* Initialize plugin registry */
static void init_plugins(void) {
    static int plugin_counter = 0;
    
    plugin_registry[0].name = "PluginA";
    plugin_registry[0].init = { return 0; };
    plugin_registry[0].process = { v_int = 42; };
    plugin_registry[0].cleanup = NULL;
    plugin_registry[0].cb = callback_impl2;
    
    plugin_registry[1].name = "PluginB";
    plugin_registry[1].init = { return 1; };
    plugin_registry[1].process = { v_char = 'X'; };
    plugin_registry[1].cleanup = { ((struct Plugin*)p)->name = NULL; };
    plugin_registry[1].cb = NULL;
    
    plugin_registry[2].name = "PluginC";
    plugin_registry[2].init = { plugin_counter++; return plugin_counter; };
    plugin_registry[2].process = { v_float += 1.0f; };
    plugin_registry[2].cleanup = NULL;
    plugin_registry[2].cb = callback_impl2;
}

/* Process variant based on runtime condition */
static int process_variant(union Variant* v, int mode) {
    switch (mode % 4) {
        case 0:
            return v->as_int * 2;
        case 1:
            return (int)(intptr_t)v->as_ptr;
        case 2:
            return (int)v->as_float;
        case 3:
            return v->as_string.len;
    }
    return 0;
}

/* Traverse pointer chain */
static int traverse_pointers(int**** pppp) {
    if (pppp && *pppp && **pppp && ***pppp) {
        return ****pppp;
    }
    return -1;
}

/* SIMD operations */
static void simd_operations(void) {
    int32x4_t a = {1, 2, 3, 4};
    int32x4_t b = {5, 6, 7, 8};
    int32x4_t c = a + b;
    
    float32x8_t f1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t f2 = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    float32x8_t f3 = f1 * f2;
    
    simd_instance.vec_int = c;
    simd_instance.vec_float = f3;
}

/* Multi-dimensional array processing */
static int process_matrix(void) {
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix_2d[i][j] = i * j;
            sum += matrix_2d[i][j];
        }
    }
    return sum;
}

/* Complex callback usage */
static void use_callbacks(void) {
    event_handler handlers[2] = {event_handler_impl, NULL};
    simple_callback scb = callback_impl1;
    
    if (scb) scb();
    
    for (int i = 0; i < 2; i++) {
        if (handlers[i]) {
            handlers[i](i, &global_variant);
        }
    }
    
    /* Call through plugin */
    if (plugin_registry[0].cb) {
        plugin_registry[0].cb(42, "test", &v_int);
    }
}

/* Main function with comprehensive type usage */
int main(void) {
    /* 1. Initialize complex structs and unions */
    struct Container container = {
        .inner = {.a = 42, .b = 'Z', .field = 127},
        .data = {.y = 3.14159},
        .counter = 0
    };
    
    union Variant local_variant;
    local_variant.as_int = 100;
    
    struct TaggedVariant tagged = {
        .tag = STRING,
        .value = {.str = "Hello"}
    };
    
    /* 2. Initialize and use plugin registry */
    init_plugins();
    if (plugin_registry[0].init) {
        plugin_registry[0].init();
    }
    if (plugin_registry[0].process) {
        plugin_registry[0].process(1);
    }
    
    /* 3. SIMD operations */
    simd_operations();
    
    /* 4. Pointer chain traversal */
    int val = 999;
    int* p1 = &val;
    int** p2 = &p1;
    int*** p3 = &p2;
    int**** p4 = &p3;
    int result = traverse_pointers(p4);
    
    /* 5. Process variant */
    int variant_result = process_variant(&local_variant, 2);
    
    /* 6. Multi-dimensional array operations */
    int matrix_sum = process_matrix();
    
    /* 7. Array of structs */
    struct Container containers[2][2];
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            containers[i][j].inner.a = i + j;
            containers[i][j].inner.b = 'A' + i + j;
            containers[i][j].data.x = i * j;
            containers[i][j].counter++;
        }
    }
    
    /* 8. Use callbacks */
    use_callbacks();
    
    /* 9. Complex pointer/array combinations */
    int arr1[3] = {1, 2, 3};
    int arr2[3] = {4, 5, 6};
    jagged[0] = arr1;
    jagged[1] = arr2;
    jagged[2] = NULL;
    jagged[3] = arr1;
    
    /* 10. Three-dimensional tensor */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                tensor[i][j][k] = i * 100.0 + j * 10.0 + k;
            }
        }
    }
    
    /* 11. Use GCC vector types directly */
    int32x4_t v1 = {1, 2, 3, 4};
    int32x4_t v2 = {5, 6, 7, 8};
    int32x4_t v3 = v1 + v2;
    int32x4_t v4 = v1 * v2;
    
    /* 12. Process opaque pointer (TYPE_UNDEFINED) */
    struct opaque* opaque_ptr = (struct opaque*)&container;
    
    /* Compute deterministic return value using all manipulated data */
    int final_result = 
        result + 
        variant_result + 
        matrix_sum + 
        container.counter + 
        containers[0][0].inner.a +
        (int)v3[0] +
        (int)tensor[0][0][0] +
        (int)v_char +
        (int)v_int % 256;
    
    /* Ensure all volatile variables are used */
    volatile int dummy = 
        v_short + v_ushort + v_uint + v_long + v_ulong + 
        v_llong + v_ullong + v_float + v_double + v_ldouble +
        v_bool + (int)creal(v_cfloat) + (int)cimag(v_cdouble);
    
    return final_result & 0xFF;  /* Return 8-bit result */
}

/* Additional functions to ensure type visibility */
void unused_function_to_keep_types(void) {
    /* Reference all types to ensure they're not optimized away */
    struct forward_declared* fd = NULL;
    struct FAM* fam = NULL;
    struct PackedStruct ps = {0};
    struct TreeNode tn = {0};
    TransUnion tu = {0};
    
    /* Force use of all arrays */
    pointer_array[0] = &v_int;
    function_ptr_array[0] = plugin_registry[0].init;
    array_of_5_ptrs[0] = NULL;
    
    /* Reference all global strings */
    const char* s1 = global_string;
    const char* s2 = error_messages[0];
    char* s3 = mutable_string;
}
