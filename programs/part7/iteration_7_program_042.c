/* test_rich_types.c - Comprehensive type coverage for GCC gengtype-state.cc */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declaration for undefined type */
struct opaque;  /* TYPE_UNDEFINED candidate */

/* GCC-specific vector type extension */
typedef float __attribute__((vector_size(32))) float32x8_t;  /* TYPE_LANG_STRUCT */

/* Aligned struct with GCC attribute */
struct __attribute__((aligned(64), packed)) PackedAligned {
    char a;
    int b;
    double c;
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*event_handler)(int, void*);  /* TYPE_CALLBACK */
typedef int (*comparator_fn)(const void*, const void*);  /* TYPE_CALLBACK */
typedef float (*transform_fn)(float*, size_t);  /* TYPE_CALLBACK */

/* Struct with function pointer members */
struct Plugin {
    const char* name;
    int (*init)(void);  /* TYPE_CALLBACK */
    void (*process)(int);  /* TYPE_CALLBACK */
    event_handler on_error;  /* TYPE_CALLBACK */
};

/* Complex function pointer signature */
int (*(*complex_callback)(void))[5];  /* TYPE_CALLBACK */

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Nested anonymous struct */
struct Container {
    struct {  /* Anonymous struct */
        int a;
        char b;
        unsigned bitfield : 4;  /* Bit-field */
    } inner;
    
    union {  /* Nested union */
        long x;
        double y;
        void* ptr;
    } data;
    
    volatile int counter;  /* Volatile member */
};

/* Struct with flexible array member */
struct DynamicString {
    size_t length;
    char data[];  /* Flexible array member */
};

/* Complex struct with arrays and pointers */
struct GraphNode {
    int id;
    struct GraphNode** neighbors;  /* Pointer to pointer */
    float weights[10];
    struct {
        short x, y;
    } position;
};

/* ========== TYPE_UNION ========== */
/* Tagged union */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    double as_double;
    struct {  /* Anonymous struct in union */
        short len;
        char buf[];  /* Flexible array in union member */
    } as_string;
};

/* Union with bitfields */
union Register {
    unsigned int full;
    struct {
        unsigned low : 16;
        unsigned high : 16;
    } parts;
};

/* ========== TYPE_ARRAY ========== */
/* Multi-dimensional array of struct pointers */
struct Node* adjacency_matrix[10][10];  /* TYPE_ARRAY */

/* Array of function pointers */
transform_fn transforms[8];  /* TYPE_ARRAY of TYPE_CALLBACK */

/* Complex array declaration */
int (*(*array_of_ptrs_to_arrays[5])[10])[20];  /* TYPE_ARRAY */

/* ========== TYPE_POINTER ========== */
/* Multi-level pointers */
int*** triple_ptr;  /* TYPE_POINTER chain */

/* Pointer to array */
int (*ptr_to_array)[100];  /* TYPE_POINTER to TYPE_ARRAY */

/* Pointer to function returning pointer to array */
float (*(*ptr_to_func_returning_array)(int))[50];  /* TYPE_POINTER to TYPE_CALLBACK */

/* ========== TYPE_SCALAR ========== */
/* All fundamental scalar types */
char char_var;
signed char schar_var;
unsigned char uchar_var;
short short_var;
unsigned short ushort_var;
int int_var;
unsigned int uint_var;
long long_var;
unsigned long ulong_var;
long long llong_var;
unsigned long long ullong_var;
float float_var;
double double_var;
long double ldouble_var;
_Bool bool_var;

/* Complex numbers */
_Complex float cfloat_var;  /* TYPE_SCALAR */
_Complex double cdouble_var;  /* TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
/* String literals and arrays */
const char* error_messages[] = {  /* TYPE_ARRAY of TYPE_STRING */
    "Error",
    "Warning",
    "Info",
    NULL
};

char static_string[256] = "Static buffer";  /* TYPE_ARRAY of TYPE_SCALAR */

/* ========== GLOBAL VARIABLES ========== */
/* Ensure type visibility through global variables */
struct Container global_container;
union Variant global_variants[5];
struct Plugin plugin_registry[3];
float32x8_t global_vector;

/* Complex global with initialization */
struct GraphNode* node_graph[100] = {0};

/* ========== FUNCTION DEFINITIONS ========== */
/* Callback implementations */
static int plugin_init_default(void) {
    return 0;
}

static void plugin_process_default(int x) {
    /* Do nothing */
}

static void default_event_handler(int event, void* data) {
    /* Empty handler */
}

static float sum_transform(float* arr, size_t len) {
    float sum = 0.0f;
    for (size_t i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

static int int_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

/* Function using complex types */
void process_container(struct Container* c, event_handler cb) {
    if (cb) {
        cb(c->inner.a, &c->data);
    }
    c->counter++;
}

/* Function with multi-dimensional array parameter */
void init_matrix(struct Node* matrix[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Initialize pointer */
            matrix[i][j] = NULL;
        }
    }
}

/* Function using GCC vector type */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;  /* Vector operation */
}

/* Function processing union type */
int process_variant(union Variant* v, int type) {
    switch (type) {
        case 0:
            return v->as_int * 2;
        case 1:
            return (v->as_ptr != NULL) ? 1 : 0;
        case 2:
            return (int)v->as_float;
        default:
            return -1;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    volatile int result = 0;  /* Prevent optimization */
    
    /* 1. Initialize structs and unions */
    struct Container local_container = {
        .inner = { .a = 42, .b = 'X', .bitfield = 7 },
        .data = { .x = 1000 },
        .counter = 0
    };
    
    union Variant var;
    var.as_int = 12345;
    
    /* 2. Populate plugin registry and call through function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .init = plugin_init_default,
        .process = plugin_process_default,
        .on_error = default_event_handler
    };
    
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    /* 3. Use GCC vector type */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = vector_add(vec_a, vec_b);
    
    /* Access vector element (non-portable but works for coverage) */
    float* vec_ptr = (float*)&vec_sum;
    result += (int)vec_ptr[0];
    
    /* 4. Use multi-dimensional array */
    init_matrix(adjacency_matrix);
    
    /* 5. Process union with different types */
    result += process_variant(&var, 0);
    
    var.as_ptr = &local_container;
    result += process_variant(&var, 1);
    
    var.as_float = 3.14159f;
    result += process_variant(&var, 2);
    
    /* 6. Use complex pointer chain */
    int value = 100;
    int* ptr1 = &value;
    int** ptr2 = &ptr1;
    triple_ptr = &ptr2;
    
    result += ***triple_ptr;
    
    /* 7. Use array of function pointers */
    transforms[0] = sum_transform;
    float numbers[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    if (transforms[0]) {
        result += (int)transforms[0](numbers, 5);
    }
    
    /* 8. Use all scalar types */
    char_var = 'A';
    schar_var = -128;
    uchar_var = 255;
    short_var = -32768;
    ushort_var = 65535;
    int_var = -2147483647 - 1;
    uint_var = 4294967295U;
    long_var = -2147483647L - 1;
    ulong_var = 4294967295UL;
    llong_var = -9223372036854775807LL - 1;
    ullong_var = 18446744073709551615ULL;
    float_var = 3.14159f;
    double_var = 2.718281828459045;
    ldouble_var = 1.618033988749895L;
    bool_var = 1;
    
    cfloat_var = 1.0f + 2.0fi;
    cdouble_var = 3.0 + 4.0i;
    
    /* 9. Use string types */
    const char* message = error_messages[0];
    result += (int)strlen(message);
    
    /* 10. Use packed/aligned struct */
    struct PackedAligned pa;
    pa.a = 'Z';
    pa.b = 999;
    pa.c = 2.71828;
    result += pa.b;
    
    /* 11. Process container with callback */
    process_container(&local_container, default_event_handler);
    result += local_container.counter;
    
    /* 12. Use comparator function pointer */
    int arr[] = {5, 2, 8, 1, 9};
    comparator_fn cmp = int_comparator;
    /* Simulate qsort-like behavior */
    for (int i = 0; i < 4; i++) {
        if (cmp(&arr[i], &arr[i+1]) > 0) {
            result++;
        }
    }
    
    /* 13. Complex array/pointer type */
    int array_20[20];
    int (*array_10_of_20)[20] = &array_20;
    int (*(*array_5_of_ptrs)[10])[20] = &array_10_of_20;
    array_of_ptrs_to_arrays[0] = array_5_of_ptrs;
    
    /* Final deterministic result */
    return result % 256;  /* Return non-zero, bounded value */
}

/* Additional type definitions not used in main but present for coverage */
struct UnusedComplexType {
    int (*(*nested_callback[3])(float(**)(double)))(void);
    volatile _Atomic long atomic_var;
};

/* External reference to undefined type */
extern struct opaque* external_opaque;
