/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
/* This program defines a diverse set of type constructs to trigger all */
/* type category cases in gengtype-state.cc lines 1123-1154 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */

/* Forward declaration for undefined/incomplete type */
struct opaque;  /* TYPE_UNDEFINED candidate */

/* GCC-specific vector type extension */
typedef float __attribute__((vector_size(32))) float32x8_t;  /* TYPE_LANG_STRUCT */

/* GCC-specific packed struct */
struct __attribute__((packed)) PackedData {
    char flag;
    int value;
    double data;
};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */

/* Complex nested structure with anonymous struct */
struct Container {
    struct {  /* Anonymous struct */
        int a;
        char b;
        volatile short c;  /* Prevent optimization */
    } inner;
    
    union {  /* Nested union */
        long x;
        double y;
        void* z;
    } data;
    
    struct Container* next;  /* Self-referential pointer */
};

/* Bit-field structure */
struct Register {
    unsigned int enable : 1;
    unsigned int mode : 3;
    unsigned int : 4;  /* Padding */
    unsigned int value : 8;
    unsigned int status : 16;
};

/* Structure with flexible array member */
struct DynamicArray {
    size_t length;
    int data[];  /* Flexible array member */
};

/* Aligned structure */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
    int tag;
};

/* ========== TYPE_UNION ========== */

/* Complex union with nested struct */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    double as_double;
    
    struct {  /* Struct inside union */
        short len;
        char buf[];  /* Flexible array in union-struct */
    } as_string;
    
    long long as_longlong;
};

/* Tagged union */
struct TaggedVariant {
    enum { INT, FLOAT, STRING, PTR } type;
    union {
        int i;
        float f;
        char* s;
        void* p;
    } value;
};

/* ========== TYPE_CALLBACK ========== */

/* Function pointer typedefs */
typedef void (*event_handler)(int, void*);  /* TYPE_CALLBACK */
typedef int (*processor_func)(float**, int);
typedef union Variant* (*transform_func)(struct Container*, event_handler);

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    transform_func transform;
    event_handler on_error;
};

/* Another callback type */
typedef struct Container* (*allocator_func)(size_t, struct Plugin*);

/* ========== TYPE_ARRAY / TYPE_POINTER ========== */

/* Complex multi-dimensional array of pointers */
struct Node* adjacency_matrix[10][10];  /* Forward declared */

/* Pointer to array */
int (*array_ptr)[20];

/* Triple pointer */
int*** triple_ptr;

/* Array of function pointers */
processor_func processors[5];

/* Array of structs */
struct Plugin plugin_registry[3];

/* ========== TYPE_SCALAR ========== */

/* Use all fundamental scalar types */
volatile char v_char = 'A';
volatile signed char v_schar = -1;
volatile unsigned char v_uchar = 255;
volatile short v_short = -32768;
volatile unsigned short v_ushort = 65535;
volatile int v_int = -2147483647;
volatile unsigned int v_uint = 4294967295U;
volatile long v_long = -2147483647L;
volatile unsigned long v_ulong = 4294967295UL;
volatile long long v_llong = -9223372036854775807LL;
volatile unsigned long long v_ullong = 18446744073709551615ULL;
volatile float v_float = 3.14159f;
volatile double v_double = 2.718281828459045;
volatile long double v_ldouble = 1.4142135623730950488L;
volatile _Bool v_bool = 1;
volatile float _Complex v_cfloat = 1.0f + 2.0f * I;
volatile double _Complex v_cdouble = 3.0 + 4.0 * I;

/* ========== TYPE_STRING ========== */

/* String literals and arrays */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char global_string[] = "Global string literal";
volatile const char* volatile_string = "Volatile string";

/* ========== FUNCTION DEFINITIONS ========== */

/* Callback function implementations */
void sample_handler(int event, void* data) {
    volatile static int call_count = 0;
    call_count++;
    (void)event;
    (void)data;
}

int plugin_init(void) {
    return 42;
}

void plugin_process(int value) {
    volatile static int last_value = 0;
    last_value = value;
}

union Variant* sample_transform(struct Container* c, event_handler h) {
    static union Variant result;
    if (c && h) {
        h(0, c);
        result.as_int = c->inner.a;
    }
    return &result;
}

/* Function using vector type */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;  /* GCC vector operation */
}

/* Function with complex pointer chain */
int process_triple_pointer(int*** ppp) {
    if (ppp && *ppp && **ppp) {
        return ***ppp;
    }
    return 0;
}

/* Function processing multi-dimensional array */
void init_adjacency_matrix(void) {
    /* This would normally allocate, but for coverage we just access */
    volatile struct Node* dummy = adjacency_matrix[0][0];
    (void)dummy;
}

/* Function demonstrating union usage */
int process_variant(union Variant* v, int use_as_int) {
    volatile int result = 0;
    if (use_as_int) {
        result = v->as_int;
    } else {
        result = (int)v->as_float;
    }
    return result;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    volatile int coverage_marker = 0;
    
    /* 1. Initialize structs and unions */
    struct Container container1 = {
        .inner = { .a = 1, .b = 'X', .c = 100 },
        .data = { .x = 1000L },
        .next = NULL
    };
    
    struct Container container2;
    container2.inner.a = 2;
    container2.inner.b = 'Y';
    container2.inner.c = 200;
    container2.data.y = 3.14159;
    container2.next = &container1;
    
    union Variant variant1;
    variant1.as_int = 42;
    
    union Variant variant2;
    variant2.as_float = 2.71828f;
    
    struct TaggedVariant tagged = {
        .type = STRING,
        .value = { .s = "Hello" }
    };
    
    /* 2. Initialize plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .init = plugin_init,
        .process = plugin_process,
        .transform = sample_transform,
        .on_error = sample_handler
    };
    
    /* Call function through pointer */
    int init_result = plugin_registry[0].init();
    plugin_registry[0].process(init_result);
    
    /* 3. Use GCC vector type */
    float32x8_t vec_a = {0};
    float32x8_t vec_b = {0};
    float32x8_t vec_c = vector_add(vec_a, vec_b);
    (void)vec_c;
    
    /* 4. Complex pointer operations */
    int value = 100;
    int* p1 = &value;
    int** p2 = &p1;
    int*** p3 = &p2;
    coverage_marker += process_triple_pointer(p3);
    
    /* Initialize adjacency matrix reference */
    init_adjacency_matrix();
    
    /* 5. Process union types */
    coverage_marker += process_variant(&variant1, 1);
    coverage_marker += process_variant(&variant2, 0);
    
    /* Switch on tagged union */
    switch (tagged.type) {
        case INT:
            coverage_marker += tagged.value.i;
            break;
        case FLOAT:
            coverage_marker += (int)tagged.value.f;
            break;
        case STRING:
            coverage_marker += strlen(tagged.value.s);
            break;
        case PTR:
            coverage_marker += (tagged.value.p != NULL);
            break;
    }
    
    /* 6. Use all scalar types in computation */
    coverage_marker += v_char + v_short + v_int + (int)v_float;
    coverage_marker += (int)creal(v_cfloat) + (int)cimag(v_cdouble);
    
    /* 7. String operations */
    for (int i = 0; error_messages[i] != NULL; i++) {
        coverage_marker += error_messages[i][0];  /* Use first char */
    }
    
    /* 8. Use packed and aligned structs */
    struct PackedData packed = { .flag = 1, .value = 999, .data = 1.234 };
    coverage_marker += packed.value;
    
    struct CacheLine cacheline;
    memset(&cacheline, 0, sizeof(cacheline));
    cacheline.tag = 0xABCD;
    coverage_marker += cacheline.tag;
    
    /* 9. Create and use flexible array member structure */
    size_t flex_size = 5;
    struct DynamicArray* flex = malloc(sizeof(struct DynamicArray) + flex_size * sizeof(int));
    if (flex) {
        flex->length = flex_size;
        for (size_t i = 0; i < flex_size; i++) {
            flex->data[i] = (int)i * 10;
            coverage_marker += flex->data[i];
        }
        free(flex);
    }
    
    /* 10. Bit-field operations */
    struct Register reg = { .enable = 1, .mode = 3, .value = 127, .status = 0xFFFF };
    coverage_marker += reg.value;
    
    /* Return deterministic value based on all operations */
    return coverage_marker % 256;
}
