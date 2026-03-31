/* test_rich_types.c - Comprehensive type coverage for GCC gengtype-state.cc */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declaration for undefined type */
struct opaque;  /* TYPE_UNDEFINED candidate */

/* GCC-specific vector type (TYPE_LANG_STRUCT) */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned struct with GCC attributes */
struct __attribute__((aligned(64), packed)) PackedAligned {
    char a;
    int b;
    double c;
};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Nested anonymous struct */
struct Container {
    struct {  /* Anonymous struct */
        int a;
        char b;
        unsigned bitfield : 4;
    } inner;
    
    union {  /* Anonymous union */
        long x;
        double y;
        void* z;
    } data;
    
    volatile int counter;  /* Prevent optimization */
};

/* Struct with flexible array member */
struct DynamicString {
    size_t length;
    char data[];  /* Flexible array member */
};

/* Complex struct with function pointer */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void* context);
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
};

/* Bitfield-heavy struct */
struct BitFieldStruct {
    unsigned a : 3;
    unsigned b : 5;
    unsigned c : 8;
    unsigned d : 16;
    signed e : 10;
    int f : 20;
};

/* ========== TYPE_UNION ========== */
/* Complex union with nested struct */
union Variant {
    int as_int;
    long as_long;
    double as_double;
    void* as_ptr;
    struct {
        short len;
        char tag;
        char buf[32];  /* Fixed-size buffer */
    } as_string;
    
    struct {
        int type;
        union {
            float f;
            int i;
        } value;
    } as_tagged;
};

/* Union with array */
union ArrayUnion {
    int as_ints[8];
    float as_floats[8];
    char as_bytes[32];
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*event_handler)(int event_id, void* user_data);
typedef int (*comparator_t)(const void*, const void*);
typedef void (*void_func)(void);

/* Complex callback signature */
typedef union Variant* (*processor_t)(float** matrix, int depth, 
                                     struct Container* ctx);

/* Struct with multiple callback members */
struct CallbackRegistry {
    event_handler on_start;
    event_handler on_data;
    event_handler on_end;
    processor_t data_processor;
    comparator_t compare;
};

/* ========== TYPE_ARRAY / TYPE_POINTER ========== */
/* Multi-dimensional array of pointers */
struct Node* adjacency_matrix[10][10];  /* Forward declared */

/* Pointer to array */
typedef int (*array_ptr_t)[5];

/* Complex function pointer with array parameter */
int (*signal_processor)(float[][256], int**, struct Plugin*);

/* Triple pointer chain */
int*** triple_ptr_chain;

/* Array of structs containing arrays */
struct Matrix3x3 {
    float data[3][3];
};

struct Matrix3x3 transformation_stack[16];

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
_Complex long double cldouble_var;

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {
    "Error: Invalid input",
    "Warning: Deprecated API",
    "Info: Processing complete",
    "Debug: Entering function",
    "Fatal: Out of memory"
};

const char* const static_string = "Static string literal";
char mutable_string[] = "Mutable string buffer";

/* ========== GLOBAL VARIABLES ========== */
/* Ensure type visibility */
struct Container global_container = {
    .inner = {42, 'X', 7},
    .data = {.y = 3.14159},
    .counter = 0
};

union Variant global_variant = {.as_int = 100};

struct Plugin plugin_registry[3];
struct CallbackRegistry callbacks;

float32x8_t global_vector1, global_vector2;
int32x4_t int_vector;

struct Matrix3x3 identity_matrix = {
    .data = {{1,0,0}, {0,1,0}, {0,0,1}}
};

/* ========== FUNCTION DEFINITIONS ========== */
/* Dummy functions for callbacks */
int plugin_init(void* context) {
    (void)context;
    static int init_count = 0;
    return ++init_count;
}

void plugin_process(int data) {
    global_container.counter += data;
}

void plugin_cleanup(struct Plugin* self) {
    if (self) self->version = -1;
}

void handle_event(int event_id, void* user_data) {
    union Variant* v = (union Variant*)user_data;
    if (v) v->as_int = event_id;
}

int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

union Variant* process_matrix(float** matrix, int depth, 
                             struct Container* ctx) {
    static union Variant result;
    if (ctx && depth > 0 && matrix) {
        result.as_double = matrix[0][0] * depth + ctx->inner.a;
    } else {
        result.as_int = -1;
    }
    return &result;
}

/* Function using vector types */
float32x8_t add_vectors(float32x8_t a, float32x8_t b) {
    return a + b;  /* GCC vector addition */
}

/* Function with complex pointer operations */
int*** create_triple_pointer(int levels, int size) {
    int*** ptr = NULL;
    if (levels > 0 && size > 0) {
        ptr = (int***)malloc(sizeof(int**) * size);
        if (ptr) {
            for (int i = 0; i < size; i++) {
                ptr[i] = (int**)malloc(sizeof(int*) * size);
                if (ptr[i]) {
                    for (int j = 0; j < size; j++) {
                        ptr[i][j] = (int*)malloc(sizeof(int) * size);
                        if (ptr[i][j]) {
                            ptr[i][j][0] = i * 100 + j * 10 + levels;
                        }
                    }
                }
            }
        }
    }
    return ptr;
}

/* Process union with type switching */
int process_variant(union Variant* v, int expected_type) {
    switch (expected_type) {
        case 0: return v->as_int;
        case 1: return (int)v->as_long;
        case 2: return (int)v->as_double;
        case 3: return v->as_string.len;
        case 4: return v->as_tagged.type;
        default: return -1;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize struct with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .version = 1,
        .init = plugin_init,
        .process = plugin_process,
        .cleanup = plugin_cleanup
    };
    
    /* Call through function pointer */
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init(&global_container);
    }
    
    /* 2. Use vector types */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = add_vectors(vec1, vec2);
    
    /* Extract first element */
    float first_element;
    memcpy(&first_element, &vec_sum, sizeof(float));
    result += (int)first_element;
    
    /* 3. Setup callback registry */
    callbacks = (struct CallbackRegistry){
        .on_start = handle_event,
        .on_data = handle_event,
        .on_end = handle_event,
        .data_processor = process_matrix,
        .compare = compare_ints
    };
    
    /* Call callback */
    if (callbacks.on_start) {
        callbacks.on_start(100, &global_variant);
        result += global_variant.as_int;
    }
    
    /* 4. Complex pointer chain */
    triple_ptr_chain = create_triple_pointer(3, 2);
    if (triple_ptr_chain && triple_ptr_chain[0] && triple_ptr_chain[0][0]) {
        result += triple_ptr_chain[0][0][0];
    }
    
    /* 5. Process union type */
    union Variant local_var;
    local_var.as_tagged.type = 42;
    local_var.as_tagged.value.f = 3.14f;
    
    result += process_variant(&local_var, 4);
    result += process_variant(&global_variant, 0);
    
    /* 6. Multi-dimensional array operations */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            transformation_stack[0].data[i][j] = 
                identity_matrix.data[i][j] * (i + j + 1);
            result += (int)transformation_stack[0].data[i][j];
        }
    }
    
    /* 7. Use all scalar types */
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
    
    /* Add to result to prevent dead code elimination */
    result += char_var + schar_var + uchar_var + short_var + bool_var;
    result += (int)float_var + (int)double_var;
    
    /* 8. String operations */
    for (int i = 0; i < 5; i++) {
        result += error_messages[i][0];  /* First char of each string */
    }
    
    result += static_string[0];
    result += mutable_string[0];
    
    /* 9. Bitfield operations */
    struct BitFieldStruct bfs = {0};
    bfs.a = 7;
    bfs.b = 31;
    bfs.c = 255;
    bfs.d = 65535;
    bfs.e = -512;
    bfs.f = 524287;
    
    result += bfs.a + bfs.b + bfs.c + bfs.d + bfs.e + bfs.f;
    
    /* 10. Array union usage */
    union ArrayUnion au;
    for (int i = 0; i < 8; i++) {
        au.as_ints[i] = i * 100;
        result += au.as_ints[i];
    }
    
    /* 11. Use the aligned/packed struct */
    struct PackedAligned pa = {'Z', 999, 2.71828};
    result += pa.a + pa.b + (int)pa.c;
    
    /* Cleanup */
    if (plugin_registry[0].cleanup) {
        plugin_registry[0].cleanup(&plugin_registry[0]);
    }
    
    /* Return deterministic result */
    return result % 256;  /* Ensure result fits in return value */
}
