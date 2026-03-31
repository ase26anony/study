/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declaration for undefined type */
struct opaque;

/* GCC-specific vector type (TYPE_LANG_STRUCT) */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Packed and aligned structs */
struct __attribute__((packed)) PackedData {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(64))) CacheLineAligned {
    double data[8];
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*event_handler)(int, void*);
typedef int (*comparator_fn)(const void*, const void*);
typedef float (*transform_fn)(float*, size_t);

/* Complex callback signature */
typedef struct Result* (*processor_cb)(int, float**, void (*)(void));

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void* config);
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
    event_handler on_error;
};

/* Another callback struct */
struct SignalProcessor {
    float (*filter)(float sample, float* state);
    void (*reset)(struct SignalProcessor* sp);
    transform_fn transform;
};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Nested anonymous struct */
struct Container {
    struct {  /* anonymous struct */
        int a;
        char b;
        unsigned flags : 4;  /* bit-field */
        unsigned mode : 3;
    } inner;
    
    union {  /* nested union */
        long x;
        double y;
        void* ptr;
    } data;
    
    struct Container* next;  /* self-referential */
};

/* Complex struct with flexible array member */
struct DynamicString {
    size_t length;
    size_t capacity;
    char data[];  /* flexible array member */
};

/* Struct with array of function pointers */
struct CallbackRegistry {
    char name[32];
    event_handler handlers[10];
    int handler_count;
};

/* Bit-field intensive struct */
struct HardwareRegister {
    unsigned enable : 1;
    unsigned mode : 3;
    unsigned reserved : 4;
    unsigned value : 8;
    unsigned status : 16;
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    long as_long;
    float as_float;
    double as_double;
    void* as_ptr;
    struct {
        short len;
        char buf[];  /* flexible array in union member */
    } as_string;
    
    struct {
        int type;
        union {
            int i;
            float f;
            char* s;
        } value;
    } as_tagged;
};

/* Tagged union */
union DataValue {
    int ival;
    float fval;
    double dval;
    char* sval;
    void* pval;
    int32x4_t vval;  /* vector type */
};

/* ========== TYPE_ARRAY / TYPE_POINTER ========== */
/* Multi-dimensional array of pointers */
struct Node* adjacency_matrix[10][10];

/* Array of function pointers */
int (*math_ops[5])(int, int);

/* Complex pointer to array */
float (*signal_buffer_ptr)[256];

/* Triple pointer */
int ***triple_ptr_chain;

/* Pointer to array of structs */
struct Container (*container_array_ptr)[5];

/* ========== TYPE_SCALAR ========== */
/* Use all fundamental scalar types */
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
_Complex float cfloat_var;
_Complex double cdouble_var;

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {"Error", "Warning", "Info", "Debug", "Trace"};
char* dynamic_strings[3];
const char static_strings[][20] = {"Hello", "World", "GCC", "Coverage"};

/* ========== GLOBAL VARIABLES ========== */
/* Ensure type visibility */
struct Plugin plugin_registry[3];
union Variant global_variants[5];
struct Container container_chain;
struct CallbackRegistry main_registry;
float32x8_t global_vector;
struct PackedData packed_global;
struct CacheLineAligned aligned_global;

/* ========== FUNCTION DEFINITIONS ========== */
/* Callback implementations */
static int plugin_init_default(void* config) {
    (void)config;
    return 0;
}

static void plugin_process_default(int data) {
    /* volatile to prevent optimization */
    volatile int result = data * 2;
    (void)result;
}

static void error_handler(int code, void* data) {
    volatile char* msg = (char*)data;
    (void)code;
    (void)msg;
}

static float lowpass_filter(float sample, float* state) {
    *state = *state * 0.9f + sample * 0.1f;
    return *state;
}

static void reset_processor(struct SignalProcessor* sp) {
    (void)sp;
}

static float sum_transform(float* arr, size_t len) {
    float sum = 0.0f;
    for (size_t i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Operations using complex types */
static void process_container(struct Container* c) {
    if (c) {
        c->inner.a = 42;
        c->inner.b = 'X';
        c->data.x = 123456789L;
        
        /* Access bit-fields */
        c->inner.flags = 0xF;
        c->inner.mode = 3;
    }
}

static void manipulate_variant(union Variant* v, int type) {
    switch (type) {
        case 0:
            v->as_int = 100;
            break;
        case 1:
            v->as_float = 3.14159f;
            break;
        case 2:
            v->as_double = 2.718281828459045;
            break;
        default:
            v->as_ptr = NULL;
    }
}

static int traverse_matrix(struct Node* matrix[10][10]) {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (matrix[i][j] != NULL) {
                count++;
            }
        }
    }
    return count;
}

/* Function using vector types */
static float32x8_t vector_operation(float32x8_t a, float32x8_t b) {
    return a + b;  /* Vector addition */
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* 1. Initialize structs and unions */
    struct Container local_container = {
        .inner = {.a = 1, .b = 'A', .flags = 1, .mode = 2},
        .data = {.x = 1000L},
        .next = NULL
    };
    
    union Variant local_variant;
    manipulate_variant(&local_variant, 0);
    
    /* 2. Setup plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .version = 1,
        .init = plugin_init_default,
        .process = plugin_process_default,
        .cleanup = NULL,
        .on_error = error_handler
    };
    
    /* Call function through pointer */
    if (plugin_registry[0].init) {
        plugin_registry[0].init(NULL);
    }
    if (plugin_registry[0].process) {
        plugin_registry[0].process(42);
    }
    
    /* 3. Use vector types */
    float32x8_t vec_a, vec_b, vec_c;
    /* Initialize vectors (simplified - actual initialization would require element-wise) */
    vec_c = vector_operation(vec_a, vec_b);
    
    /* 4. Multi-dimensional array and pointer chains */
    struct Node* local_matrix[10][10];
    memset(local_matrix, 0, sizeof(local_matrix));
    
    /* Create some pointer chains */
    int value = 42;
    int *p1 = &value;
    int **p2 = &p1;
    int ***p3 = &p2;
    triple_ptr_chain = p3;
    
    /* Traverse matrix */
    int node_count = traverse_matrix(local_matrix);
    
    /* 5. Process union types */
    union DataValue data_values[3];
    data_values[0].ival = 100;
    data_values[1].fval = 3.14f;
    data_values[2].dval = 2.71828;
    
    /* Runtime condition for union access */
    for (int i = 0; i < 3; i++) {
        volatile double temp;
        if (i == 0) temp = data_values[i].ival;
        else if (i == 1) temp = data_values[i].fval;
        else temp = data_values[i].dval;
        (void)temp;
    }
    
    /* 6. Use all scalar types */
    char_var = 'Z';
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
    ldouble_var = 3.14159265358979323846L;
    bool_var = 1;
    
    cfloat_var = 1.0f + 2.0fi;
    cdouble_var = 3.0 + 4.0i;
    
    /* 7. String operations */
    dynamic_strings[0] = "Dynamic";
    dynamic_strings[1] = "String";
    dynamic_strings[2] = "Array";
    
    /* 8. Signal processor with callback */
    struct SignalProcessor sp = {
        .filter = lowpass_filter,
        .reset = reset_processor,
        .transform = sum_transform
    };
    
    float state = 0.0f;
    if (sp.filter) {
        float sample = 1.0f;
        float filtered = sp.filter(sample, &state);
        (void)filtered;
    }
    
    /* 9. Packed and aligned structs */
    packed_global.a = 'P';
    packed_global.b = 0xABCDEF;
    packed_global.c = 12345;
    
    for (int i = 0; i < 8; i++) {
        aligned_global.data[i] = i * 1.0;
    }
    
    /* 10. Complex array of structs */
    struct Container container_array[3][2];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            container_array[i][j].inner.a = i * 10 + j;
            container_array[i][j].inner.b = 'A' + i + j;
        }
    }
    container_array_ptr = container_array;
    
    /* 11. Math operations array */
    math_ops[0] = NULL; /* Would be actual functions in real usage */
    
    /* 12. Callback registry */
    main_registry.handler_count = 2;
    main_registry.handlers[0] = error_handler;
    
    /* Compute deterministic return value using all manipulated data */
    int hash = 0;
    hash ^= local_container.inner.a;
    hash ^= local_variant.as_int;
    hash ^= node_count;
    hash ^= (int)char_var;
    hash ^= (int)float_var;
    hash ^= packed_global.b;
    hash ^= (int)(aligned_global.data[0] * 100);
    
    /* Ensure all types are referenced to prevent dead code elimination */
    volatile struct opaque* opaque_ptr = NULL;
    (void)opaque_ptr;
    (void)adjacency_matrix;
    (void)signal_buffer_ptr;
    (void)global_vector;
    (void)global_variants;
    (void)container_chain;
    
    return hash & 0xFF; /* Return non-zero, deterministic value */
}

/* Additional type definitions for completeness */
struct Node {
    int id;
    struct Node** neighbors;
    int neighbor_count;
};

/* Dummy function using the opaque type */
void use_opaque(struct opaque* op) {
    (void)op;
}
