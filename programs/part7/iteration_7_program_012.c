/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== 1. USER-DEFINED STRUCTURES AND UNIONS ========== */

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* Complex nested struct with anonymous inner struct */
struct Container {
    struct {
        int a;
        char b;
        volatile short c; /* Prevent optimization */
    } inner;
    union {
        long x;
        double y;
        void* ptr;
    } data;
    unsigned bitfield : 4;
    unsigned : 2; /* Unnamed bitfield */
    unsigned another_bit : 6;
};

/* Union with flexible array member */
union Variant {
    int as_int;
    void* as_ptr;
    struct {
        short len;
        char buf[]; /* Flexible array member */
    } as_string;
    _Complex double as_complex; /* Complex scalar */
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedData {
    char id;
    int count;
    short checksum;
};

/* Struct with function pointer member (for TYPE_CALLBACK) */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    void (*cleanup)(struct Plugin*);
};

/* Struct containing array of pointers */
struct Graph {
    int num_nodes;
    struct Node** nodes; /* Forward reference */
    float adjacency[10][10]; /* Multi-dimensional array */
};

/* Forward-declared node for linked structure */
struct Node {
    int id;
    struct Node* next;
    struct Node* prev;
};

/* ========== 2. FUNCTION POINTERS AND CALLBACKS ========== */

/* Typedef for callback function */
typedef void (*event_handler)(int, void*);
typedef int (*comparator)(const void*, const void*);

/* Complex function pointer signature */
typedef int (*(*signal_factory)(float[][256], int**))(void);

/* Callback implementations */
int plugin_init_default(void) {
    static volatile int counter = 0;
    return ++counter;
}

void plugin_process_default(int x) {
    volatile int result = x * 2;
    (void)result; /* Use result to prevent dead code elimination */
}

void sample_event_handler(int event, void* data) {
    volatile char marker = 'H';
    (void)marker;
    (void)event;
    (void)data;
}

/* ========== 3. ARRAYS AND POINTER CHAINS ========== */

/* Multi-level pointer chain */
int*** create_pointer_chain(int depth) {
    int*** ptr = malloc(sizeof(int**));
    *ptr = malloc(sizeof(int*));
    **ptr = malloc(sizeof(int));
    ***ptr = 42;
    return ptr;
}

/* Array of structs containing function pointers */
struct Plugin plugin_registry[3];

/* Pointer to array of unions */
union Variant* variant_array[5];

/* ========== 4. LANGUAGE-SPECIFIC AND OPAQUE TYPES ========== */

/* GCC vector extension for TYPE_LANG_STRUCT */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
};

/* Opaque type (forward declared) */
struct opaque* global_opaque;

/* ========== 5. SCALAR AND STRING TYPES ========== */

/* String literals and arrays */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char* dynamic_strings[2];

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
_Complex float cfloat_var;
_Complex double cdouble_var;
_Complex long double cldouble_var;

/* ========== 6. TYPE USAGE AND OPERATIONS ========== */

/* Function using complex types */
void simulate(union Variant *v, event_handler cb) {
    static int call_count = 0;
    
    if (v->as_int > 0) {
        cb(call_count++, (void*)v);
    } else {
        v->as_ptr = &call_count;
    }
}

/* Process multi-dimensional array */
float process_matrix(float matrix[][10], int n) {
    volatile float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            sum += matrix[i][j];
        }
    }
    return sum;
}

/* Initialize plugin registry */
void init_plugins(void) {
    for (int i = 0; i < 3; i++) {
        plugin_registry[i].name = error_messages[i % 3];
        plugin_registry[i].init = plugin_init_default;
        plugin_registry[i].process = plugin_process_default;
        plugin_registry[i].cleanup = NULL;
    }
}

/* Vector operations */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    volatile int result = 0; /* Prevent optimization */
    
    /* 1. Initialize structs and unions */
    struct Container container = {
        .inner = { .a = 1, .b = 'A', .c = 100 },
        .data = { .x = 1000 },
        .bitfield = 7,
        .another_bit = 31
    };
    
    union Variant variant;
    variant.as_int = 42;
    result += variant.as_int;
    
    /* 2. Initialize and use function pointers */
    init_plugins();
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    if (plugin_registry[1].process) {
        plugin_registry[1].process(result);
    }
    
    /* 3. Multi-dimensional array operations */
    float matrix[5][10];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (float)(i * j);
        }
    }
    result += (int)process_matrix(matrix, 5);
    
    /* 4. Pointer chain traversal */
    int*** ptr_chain = create_pointer_chain(3);
    result += ***ptr_chain;
    
    /* Free pointer chain */
    free(**ptr_chain);
    free(*ptr_chain);
    free(ptr_chain);
    
    /* 5. Vector type usage */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_c = vector_add(vec_a, vec_b);
    
    /* Access vector elements */
    volatile float* vec_ptr = (float*)&vec_c;
    for (int i = 0; i < 8; i++) {
        result += (int)vec_ptr[i];
    }
    
    /* 6. Union type processing */
    union Variant vars[3];
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            vars[i].as_int = i * 10;
            result += vars[i].as_int;
        } else {
            vars[i].as_ptr = &result;
            result += *(int*)vars[i].as_ptr;
        }
    }
    
    /* 7. String operations */
    dynamic_strings[0] = strdup("Hello");
    dynamic_strings[1] = strdup("World");
    for (int i = 0; i < 2; i++) {
        result += (int)strlen(dynamic_strings[i]);
        free(dynamic_strings[i]);
    }
    
    /* 8. Callback invocation */
    event_handler handler = sample_event_handler;
    simulate(&variant, handler);
    
    /* 9. Use all scalar types */
    char_var = 'Z';
    schar_var = -128;
    uchar_var = 255;
    short_var = -32768;
    ushort_var = 65535;
    int_var = -2147483647;
    uint_var = 4294967295U;
    long_var = -2147483647L;
    ulong_var = 4294967295UL;
    llong_var = -9223372036854775807LL;
    ullong_var = 18446744073709551615ULL;
    float_var = 3.14159f;
    double_var = 2.718281828459045;
    ldouble_var = 1.618033988749895L;
    bool_var = 1;
    cfloat_var = 1.0f + 2.0fi;
    cdouble_var = 3.0 + 4.0i;
    cldouble_var = 5.0L + 6.0Li;
    
    /* Use scalars in computation */
    result += char_var + schar_var + uchar_var;
    result += (int)(float_var + double_var + ldouble_var);
    result += (int)(__real__ cfloat_var + __imag__ cfloat_var);
    
    /* 10. Packed and aligned structs */
    struct PackedData packed = { .id = 'P', .count = 999, .checksum = 1234 };
    result += packed.count;
    
    struct CacheLine cache_line;
    memset(cache_line.data, 0xAA, sizeof(cache_line.data));
    result += cache_line.data[0];
    
    /* Return deterministic result for coverage verification */
    return result % 256;
}
