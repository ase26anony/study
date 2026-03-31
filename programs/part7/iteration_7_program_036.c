/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Incomplete/undefined type */
struct forward_declared;           /* Another forward declaration */

/* ========== TYPE_SCALAR - All fundamental scalar types ========== */
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
volatile float v_float = 3.14159f;
volatile double v_double = 2.718281828459045;
volatile long double v_ldouble = 1.618033988749895L;
volatile _Bool v_bool = 1;
volatile _Complex float v_cfloat = 1.0f + 2.0fi;
volatile _Complex double v_cdouble = 3.0 + 4.0i;
volatile _Complex long double v_cldouble = 5.0L + 6.0Li;

/* ========== TYPE_STRING ========== */
const char* global_string = "Global string literal";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable string";

/* ========== TYPE_CALLBACK - Function pointers and callbacks ========== */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(const char*, int, void*);
typedef void (*event_handler)(int, void*);
typedef double (*math_func)(double, double);

/* Callback function implementations */
static void callback_impl(void) {
    v_int++;
}

static int complex_callback_impl(const char* msg, int val, void* data) {
    return val + (int)((intptr_t)data);
}

static void event_handler_impl(int event, void* context) {
    v_char = (char)event;
}

static double add_func(double a, double b) { return a + b; }
static double mul_func(double a, double b) { return a * b; }

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    void (*cleanup)(void);
    complex_callback user_cb;
};

/* Array of function pointers */
math_func math_operations[] = {add_func, mul_func, NULL};

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
        volatile short c;
    } inner;
    
    union {
        long x;
        double y;
        void* ptr;
    } data;
    
    struct Point point;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 24;
    unsigned int : 4;  /* Padding */
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {
    char id;
    int count;
    short checksum;
};

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {
    char data[60];
    int tag;
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
    struct {
        int depth;
        int balance;
    } metadata;
};

/* ========== TYPE_UNION ========== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Tagged union */
union Variant {
    int as_int;
    double as_double;
    void* as_ptr;
    struct {
        short len;
        char buf[];  /* Flexible array member in union */
    } as_string;
};

/* Anonymous union inside struct */
struct WithAnonymousUnion {
    int type;
    union {
        int i;
        float f;
        char* s;
    };
};

/* ========== TYPE_ARRAY - Complex arrays ========== */
/* Multi-dimensional arrays */
int matrix_2d[10][20];
int matrix_3d[5][10][15];

/* Array of structs */
struct Point point_array[100];
struct Container container_array[50];

/* Array of pointers */
struct Point* point_ptr_array[50];
void* void_ptr_array[25];

/* Array of arrays of pointers */
struct TreeNode* tree_grid[10][10];

/* Array of function pointers */
event_handler handlers[8];

/* ========== TYPE_POINTER - Complex pointer chains ========== */
int*** triple_ptr;
struct Container**** quad_container_ptr;
void (*signal_processor)(float[][256], int**);

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */
/* Vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* SIMD types */
typedef int __attribute__((vector_size(64))) int64x8_t;

/* Transparent union */
typedef union __attribute__((transparent_union)) TransUnion {
    int i;
    long l;
} TransUnion;

/* ========== Global instances ========== */
/* Plugin registry */
struct Plugin plugin_registry[3] = {
    {"plugin1", NULL, NULL, NULL, complex_callback_impl},
    {"plugin2", NULL, NULL, NULL, NULL},
    {"plugin3", NULL, NULL, NULL, NULL}
};

/* Union instance */
union Variant global_variant;

/* Vector instance */
float32x8_t global_vector;

/* ========== Function definitions ========== */
/* Function using complex types */
static void process_container(struct Container* c, event_handler cb) {
    if (cb) {
        cb(c->inner.a, &c->data);
    }
    
    /* Access nested struct */
    c->point.x = c->inner.a;
    c->point.y = c->inner.b;
    
    /* Use union */
    if (c->inner.a > 0) {
        c->data.y = 3.14;
    } else {
        c->data.x = 100L;
    }
}

/* Function with multi-dimensional array parameter */
static int sum_matrix(int rows, int cols, int mat[rows][cols]) {
    int total = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            total += mat[i][j];
        }
    }
    return total;
}

/* Function using vector types */
static float32x8_t add_vectors(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function processing variant union */
static void process_variant(union Variant* v, int type) {
    switch (type) {
        case 0:
            v->as_int = 42;
            break;
        case 1:
            v->as_double = 3.14159;
            break;
        case 2:
            v->as_ptr = &global_string;
            break;
    }
}

/* Function with pointer chain */
static int deref_triple_ptr(int*** ptr) {
    if (ptr && *ptr && **ptr) {
        return ***ptr;
    }
    return 0;
}

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize and use struct types */
    struct Container container = {
        .inner = {10, 'B', 100},
        .data = {.y = 2.71828},
        .point = {1, 2, 3}
    };
    
    struct BitFieldStruct bf = {1, 7, 15, -1000};
    struct PackedData packed = {'X', 1000, 0xABCD};
    struct CacheLine cache_line;
    
    result += container.inner.a;
    result += bf.value;
    result += packed.count;
    
    /* 2. Use union types */
    union SimpleUnion su;
    su.as_int = 42;
    result += su.as_int;
    
    union Variant var;
    process_variant(&var, 0);
    result += var.as_int;
    
    struct WithAnonymousUnion au;
    au.type = 1;
    au.i = 100;
    result += au.i;
    
    /* 3. Populate and call through function pointers */
    plugin_registry[0].init = NULL;
    plugin_registry[0].process = NULL;
    plugin_registry[0].cleanup = NULL;
    
    handlers[0] = event_handler_impl;
    if (handlers[0]) {
        handlers[0](42, &result);
    }
    
    /* Call through math function pointers */
    double math_result = 0.0;
    for (int i = 0; math_operations[i] != NULL; i++) {
        math_result += math_operations[i](1.5, 2.5);
    }
    result += (int)math_result;
    
    /* 4. Use vector types (GCC extension) */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = add_vectors(vec1, vec2);
    
    /* Access vector elements */
    float* vec_ptr = (float*)&vec_sum;
    for (int i = 0; i < 8; i++) {
        result += (int)vec_ptr[i];
    }
    
    /* 5. Multi-dimensional array operations */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix_2d[i][j] = i * j;
        }
    }
    result += sum_matrix(10, 20, matrix_2d);
    
    /* 6. Pointer chain traversal */
    int value = 42;
    int* p1 = &value;
    int** p2 = &p1;
    int*** p3 = &p2;
    triple_ptr = p3;
    
    result += deref_triple_ptr(triple_ptr);
    
    /* 7. Array of structs */
    for (int i = 0; i < 100; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].z = i * 3;
        result += point_array[i].x;
    }
    
    /* 8. String operations */
    const char* str = error_messages[0];
    while (*str) {
        result += *str++;
    }
    
    /* 9. Complex callback usage */
    if (plugin_registry[0].user_cb) {
        result += plugin_registry[0].user_cb("test", 10, (void*)(intptr_t)5);
    }
    
    /* 10. Process container with callback */
    process_container(&container, event_handler_impl);
    result += container.point.x;
    
    /* 11. Use all scalar types in computation */
    result += v_char + v_short + v_int + (int)v_float + (int)v_double;
    result += creal(v_cfloat) + cimag(v_cfloat);
    
    /* 12. Tree structure operations */
    struct TreeNode nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i].value = i * 10;
        nodes[i].left = (i > 0) ? &nodes[i-1] : NULL;
        nodes[i].right = (i < 2) ? &nodes[i+1] : NULL;
        nodes[i].metadata.depth = i;
        nodes[i].metadata.balance = 0;
        result += nodes[i].value;
    }
    
    /* 13. Grid of tree pointers */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            tree_grid[i][j] = &nodes[i % 3];
            if (tree_grid[i][j]) {
                result += tree_grid[i][j]->value;
            }
        }
    }
    
    /* 14. Volatile operations to prevent optimization */
    v_int = result;
    v_char = result & 0xFF;
    
    /* Return deterministic result */
    return result & 0xFF;  /* Return lower 8 bits as observable result */
}
