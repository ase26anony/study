/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Forward declaration - TYPE_UNDEFINED */
typedef struct incomplete incomplete_t;

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */

/* Complex nested structure with bit-fields */
struct __attribute__((packed)) PackedContainer {
    struct {
        int a : 8;
        unsigned int b : 24;
        char c;
    } inner;
    
    union {
        long x;
        double y;
        struct {
            short tag;
            char data[8];
        } tagged;
    } data;
    
    volatile int counter;
};

/* Another struct with flexible array member */
struct DynamicString {
    size_t length;
    char str[];  /* Flexible array member */
};

/* Struct with anonymous union */
struct SensorData {
    int id;
    union {
        float temperature;
        int pressure;
        struct {
            short x, y, z;
        } acceleration;
    };
    char unit[4];
};

/* ========== TYPE_UNION ========== */

/* Complex union with nested struct */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    double as_double;
    struct {
        short len;
        char buf[];  /* Flexible array in union member */
    } as_string;
    
    struct {
        unsigned char type;
        unsigned char data[7];
    } as_packed;
};

/* Tagged union */
union TaggedUnion {
    struct {
        int type;
        union {
            int i;
            float f;
            void* p;
        } value;
    } tagged;
    
    long long raw;
};

/* ========== TYPE_CALLBACK / Function Pointers ========== */

/* Callback typedefs */
typedef void (*event_handler)(int event_id, void* user_data);
typedef int (*processor_func)(const char* input, char* output, size_t len);
typedef union Variant* (*variant_factory)(int type);

/* Struct with multiple function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
    event_handler on_event;
    
    /* Pointer to array of processors */
    processor_func (*get_processors)(void);
};

/* Complex function pointer type */
typedef int (*(*factory_func)(int arg_count))(float, double);

/* ========== TYPE_ARRAY / Multi-dimensional Arrays ========== */

/* Array of structs */
struct Node {
    int id;
    struct Node** neighbors;
    void* data;
};

/* Complex multi-dimensional array */
struct Node* adjacency_matrix[10][10];

/* Array of function pointers */
static processor_func processors[5];

/* Pointer to array */
int (*array_ptr)[20];

/* ========== TYPE_LANG_STRUCT / GCC Extensions ========== */

/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
};

/* Transparent union */
union __attribute__((transparent_union)) Number {
    int i;
    float f;
    double d;
};

/* ========== TYPE_SCALAR / Basic Types ========== */

/* Use all scalar types */
static char char_var;
static signed char schar_var;
static unsigned char uchar_var;
static short short_var;
static unsigned short ushort_var;
static int int_var;
static unsigned int uint_var;
static long long_var;
static unsigned long ulong_var;
static long long llong_var;
static unsigned long long ullong_var;
static float float_var;
static double double_var;
static long double ldouble_var;
static _Bool bool_var;

/* Complex types */
static float _Complex complex_float;
static double _Complex complex_double;

/* ========== TYPE_STRING ========== */

/* String literals and arrays */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
static char static_string[] = "Static string";
char* dynamic_string = "Dynamic string";

/* ========== Global Variables ========== */

/* Global instances of complex types */
struct PackedContainer global_container = {
    .inner = {.a = 42, .b = 1000, .c = 'X'},
    .data = {.x = 1234567890L}
};

union Variant global_variant = {.as_int = -1};

struct Plugin plugin_registry[3];

float32x8_t global_vector;

/* ========== Function Definitions ========== */

/* Callback function implementations */
static void sample_event_handler(int event_id, void* user_data) {
    *(int*)user_data = event_id * 2;
}

static int sample_processor(const char* input, char* output, size_t len) {
    int i;
    for (i = 0; input[i] && i < (int)len - 1; i++) {
        output[i] = input[i] + 1;
    }
    output[i] = '\0';
    return i;
}

static int plugin_init(void) {
    return 0;
}

static void plugin_process(int data) {
    int_var += data;
}

static void plugin_cleanup(struct Plugin* self) {
    self->name = NULL;
}

/* Function using complex pointer chain */
static int*** create_3d_pointer_chain(int depth1, int depth2, int depth3) {
    int i, j;
    int*** ptr3 = NULL;
    
    if (depth1 > 0) {
        ptr3 = (int***)__builtin_alloca(depth1 * sizeof(int**));
        for (i = 0; i < depth1; i++) {
            ptr3[i] = (int**)__builtin_alloca(depth2 * sizeof(int*));
            for (j = 0; j < depth2; j++) {
                ptr3[i][j] = (int*)__builtin_alloca(depth3 * sizeof(int));
                ptr3[i][j][0] = i * j;
            }
        }
    }
    
    return ptr3;
}

/* Function using GCC vector type */
static float32x8_t vector_operation(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Process union with type tag */
static void process_variant(union Variant* v, int use_float) {
    if (use_float) {
        v->as_float = 3.14159f;
    } else {
        v->as_int = 42;
    }
}

/* Function with complex array parameter */
static int process_matrix(struct Node* matrix[10][10]) {
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

/* ========== Main Function ========== */

int main(void) {
    int result = 0;
    
    /* 1. Initialize and use struct types */
    struct SensorData sensor = {
        .id = 1,
        .temperature = 23.5f,
        .unit = "C"
    };
    
    struct PackedContainer local_container;
    local_container.inner.a = 10;
    local_container.inner.b = 500;
    local_container.data.y = 2.71828;
    result += local_container.inner.a;
    
    /* 2. Use union types */
    union Variant local_variant;
    process_variant(&local_variant, 0);
    result += local_variant.as_int;
    
    union TaggedUnion tagged;
    tagged.tagged.type = 1;
    tagged.tagged.value.i = 100;
    result += tagged.tagged.value.i;
    
    /* 3. Setup and use function pointers/callbacks */
    plugin_registry[0].name = "TestPlugin";
    plugin_registry[0].init = plugin_init;
    plugin_registry[0].process = plugin_process;
    plugin_registry[0].cleanup = plugin_cleanup;
    plugin_registry[0].on_event = sample_event_handler;
    
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    int event_data = 0;
    if (plugin_registry[0].on_event) {
        plugin_registry[0].on_event(21, &event_data);
        result += event_data;
    }
    
    /* 4. Use arrays and pointers */
    processors[0] = sample_processor;
    char output[100];
    if (processors[0]) {
        result += processors[0]("test", output, sizeof(output));
    }
    
    /* 5. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    float32x8_t vec_c = vector_operation(vec_a, vec_b);
    
    /* Access vector elements to ensure they're used */
    float* vec_ptr = (float*)&vec_c;
    result += (int)vec_ptr[0];
    
    /* 6. Complex pointer chains */
    int*** ptr_chain = create_3d_pointer_chain(3, 4, 5);
    if (ptr_chain && ptr_chain[0] && ptr_chain[0][0]) {
        result += ptr_chain[0][0][0];
    }
    
    /* 7. Multi-dimensional array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if ((i + j) % 3 == 0) {
                adjacency_matrix[i][j] = (struct Node*)&result;
            }
        }
    }
    result += process_matrix(adjacency_matrix);
    
    /* 8. Use all scalar types */
    char_var = 'A';
    schar_var = -10;
    uchar_var = 200;
    short_var = -1000;
    ushort_var = 5000;
    int_var = -100000;
    uint_var = 4000000;
    long_var = -1000000L;
    ulong_var = 4000000UL;
    llong_var = -1000000000LL;
    ullong_var = 4000000000ULL;
    float_var = 1.234f;
    double_var = 3.1415926535;
    ldouble_var = 2.718281828459045L;
    bool_var = 1;
    
    complex_float = 1.0f + 2.0f * I;
    complex_double = 3.0 + 4.0 * I;
    
    result += char_var + schar_var + uchar_var + short_var + int_var;
    result += (int)float_var + (int)double_var + bool_var;
    
    /* 9. String operations */
    for (int i = 0; error_messages[i]; i++) {
        result += error_messages[i][0];  /* Add first char of each string */
    }
    
    dynamic_string = static_string;
    result += dynamic_string[0];
    
    /* 10. Use aligned struct */
    struct CacheLine cache_line;
    for (int i = 0; i < 64; i++) {
        cache_line.data[i] = i;
        result += cache_line.data[i];
    }
    
    /* 11. Use transparent union */
    union Number num;
    num.i = 42;
    result += num.i;
    
    /* Final deterministic result */
    return result & 0xFF;  /* Return lower 8 bits as observable result */
}
