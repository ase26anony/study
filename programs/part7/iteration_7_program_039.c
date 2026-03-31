/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward declarations ========== */
struct opaque;                     /* Incomplete/undefined type */
struct forward_declared;           /* Will be defined later */

/* ========== TYPE_SCALAR - All fundamental types ========== */
typedef char byte_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;
typedef _Complex float complex32_t;
typedef _Complex double complex64_t;

/* ========== TYPE_LANG_STRUCT - GCC-specific extensions ========== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Packed and aligned structs */
struct __attribute__((packed)) packed_struct {
    int a;
    char b;
    double c;
};

struct __attribute__((aligned(64))) aligned_struct {
    long data[8];
    char tag;
};

/* ========== TYPE_CALLBACK - Function pointers and callbacks ========== */
/* Basic callback typedef */
typedef void (*simple_callback)(int, void*);

/* Complex function pointer signature */
typedef int (*processor_func)(float**, int, const char*);

/* Struct with callback members */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void* config);
    void (*process)(int data);
    void (*cleanup)(void);
    simple_callback event_handler;
};

/* Nested callback type */
typedef void (*factory_func)(struct Plugin*(*)(const char*));

/* ========== TYPE_STRUCT and TYPE_USER_STRUCT ========== */
/* Simple struct */
struct Point {
    int x;
    int y;
    int z;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

/* Nested anonymous struct */
struct Container {
    struct {
        int id;
        char label[32];
    } metadata;
    
    union {
        long timestamp;
        double value;
    } data;
    
    struct Point position;
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t length;
    int elements[];
};

/* Complex user-defined struct with all features */
struct __attribute__((packed)) ComplexUserStruct {
    struct Container base;
    struct BitFieldStruct flags;
    processor_func processor;
    float32x8_t vector_data;
    volatile int counter;
    const char* description;
};

/* ========== TYPE_UNION ========== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Union with struct member */
union Variant {
    int type_id;
    struct {
        short length;
        char buffer[256];
    } string_data;
    struct {
        int x, y;
    } point_data;
    complex64_t complex_data;
};

/* Tagged union */
struct TaggedUnionContainer {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE, COMPLEX_TYPE } tag;
    union {
        int int_val;
        float float_val;
        char* string_val;
        complex64_t complex_val;
    } value;
};

/* ========== TYPE_ARRAY and TYPE_POINTER ========== */
/* Multi-dimensional array */
int matrix_3d[5][10][20];

/* Array of structs */
struct Point point_array[100];

/* Array of pointers */
struct Plugin* plugin_ptrs[10];

/* Pointer to array */
int (*array_ptr)[10];

/* Triple pointer */
int ***triple_ptr_chain;

/* Array of function pointers */
simple_callback callbacks[5];

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {
    "Success",
    "Error: Invalid argument",
    "Error: Out of memory",
    "Warning: Deprecated API",
    "Info: Operation complete"
};

const char* const static_strings[] = {
    "constant string 1",
    "constant string 2",
    "constant string 3"
};

/* ========== Global variables for visibility ========== */
struct ComplexUserStruct global_user_struct;
union Variant global_variant;
struct Plugin global_plugins[3];
float32x8_t global_vector;
int32x4_t global_int_vector;

/* ========== Function implementations ========== */
static int dummy_init(void* config) {
    (void)config;
    static int counter = 0;
    return ++counter;
}

static void dummy_process(int data) {
    volatile int temp = data * 2;
    (void)temp;
}

static void dummy_cleanup(void) {
    /* Do nothing */
}

static void event_handler_impl(int event, void* data) {
    volatile int* ptr = (int*)data;
    if (ptr) *ptr = event;
}

static int complex_processor(float** data, int count, const char* name) {
    volatile int result = 0;
    for (int i = 0; i < count && i < 10; i++) {
        if (data[i]) {
            result += (int)data[i][0];
        }
    }
    if (name) {
        result += strlen(name);
    }
    return result;
}

/* ========== Now define the forward declared struct ========== */
struct forward_declared {
    int finalized;
    struct ComplexUserStruct* user_data;
    union Variant variant;
};

/* ========== Main function with active type usage ========== */
int main(void) {
    /* 1. Initialize structs and unions */
    struct Container container = {
        .metadata = { .id = 1, .label = "Test Container" },
        .data = { .timestamp = 1234567890 },
        .position = { .x = 10, .y = 20, .z = 30 }
    };
    
    struct BitFieldStruct bits = { .flag1 = 1, .flag2 = 3, .flag3 = 7 };
    
    struct ComplexUserStruct complex = {
        .base = container,
        .flags = bits,
        .processor = complex_processor,
        .counter = 0,
        .description = "Complex user struct instance"
    };
    
    /* Initialize vector data */
    for (int i = 0; i < 8; i++) {
        complex.vector_data[i] = i * 1.5f;
    }
    
    /* 2. Use unions */
    union Variant local_variant;
    local_variant.type_id = 1;
    strcpy(local_variant.string_data.buffer, "Test string");
    local_variant.string_data.length = (short)strlen("Test string");
    
    struct TaggedUnionContainer tagged = {
        .tag = COMPLEX_TYPE,
        .value = { .complex_val = 1.0 + 2.0 * I }
    };
    
    /* 3. Setup and use function pointers */
    struct Plugin plugin = {
        .name = "Test Plugin",
        .version = 1,
        .init = dummy_init,
        .process = dummy_process,
        .cleanup = dummy_cleanup,
        .event_handler = event_handler_impl
    };
    
    int callback_data = 0;
    plugin.event_handler(42, &callback_data);
    
    /* Call through processor function pointer */
    float* data_ptrs[3];
    float data1[5] = {1.0f, 2.0f, 3.0f};
    float data2[5] = {4.0f, 5.0f, 6.0f};
    data_ptrs[0] = data1;
    data_ptrs[1] = data2;
    data_ptrs[2] = NULL;
    
    int proc_result = complex.processor(data_ptrs, 3, "test");
    
    /* 4. Use arrays and pointers */
    for (int i = 0; i < 100; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].z = i * 3;
    }
    
    /* Setup pointer chain */
    int value = 42;
    int* ptr1 = &value;
    int** ptr2 = &ptr1;
    triple_ptr_chain = &ptr2;
    
    /* Use triple pointer */
    volatile int deref = ***triple_ptr_chain;
    (void)deref;
    
    /* 5. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = vec_a + vec_b;
    
    global_vector = vec_sum;
    
    /* 6. Multi-dimensional array access */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 20; k++) {
                matrix_3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* 7. String operations */
    const char* selected_message = error_messages[proc_result % 5];
    volatile size_t msg_len = strlen(selected_message);
    
    /* 8. Use all scalar types */
    byte_t byte_val = 0xFF;
    int16_t short_val = -32768;
    int32_t int_val = 2147483647;
    int64_t long_val = 9223372036854775807LL;
    float32_t float_val = 3.14159f;
    float64_t double_val = 2.718281828459045;
    bool_t bool_val = 1;
    complex32_t complex_float = 1.0f + 2.0f * I;
    complex64_t complex_double = 3.0 + 4.0 * I;
    
    /* Prevent optimization */
    volatile byte_t v_byte = byte_val;
    volatile int16_t v_short = short_val;
    volatile int64_t v_long = long_val;
    volatile complex64_t v_cd = complex_double;
    
    /* 9. Define and use the forward declared struct */
    struct forward_declared fd = {
        .finalized = 1,
        .user_data = &complex,
        .variant = local_variant
    };
    
    /* 10. Compute deterministic return value */
    int hash = 0;
    hash += container.metadata.id;
    hash += container.position.x;
    hash += bits.flag3;
    hash += proc_result;
    hash += (int)vec_sum[0];
    hash += matrix_3d[0][0][0];
    hash += (int)msg_len;
    hash += fd.finalized;
    hash += v_byte;
    hash += (int)(creal(v_cd) + cimag(v_cd));
    
    /* Ensure all globals are touched */
    global_user_struct = complex;
    global_variant = local_variant;
    global_plugins[0] = plugin;
    
    return hash % 256;
}
