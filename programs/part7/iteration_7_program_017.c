/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                    /* Forward declaration - TYPE_UNDEFINED */
struct incomplete;                /* Another forward declaration */
typedef struct opaque* opaque_ptr_t;

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

/* ========== TYPE_STRING ========== */
const char* global_strings[] = {
    "Error: Invalid operation",
    "Warning: Deprecated API",
    "Info: Processing complete",
    "Debug: Entering function",
    NULL
};

/* ========== TYPE_CALLBACK - Function pointers ========== */
typedef void (*simple_callback)(void);
typedef int (*data_processor)(const void* data, size_t len);
typedef void (*event_handler)(int event_id, void* user_data);
typedef float* (*allocator_t)(size_t count);

/* Complex callback signature with multiple parameters */
typedef union Variant* (*variant_processor_t)(
    int operation,
    union Variant* input,
    event_handler on_complete
);

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */

/* Simple struct with basic types */
struct Point {
    float32_t x;
    float32_t y;
    float32_t z;
} __attribute__((aligned(16)));

/* Struct with bit-fields */
struct PacketHeader {
    unsigned int version : 4;
    unsigned int flags : 8;
    unsigned int length : 20;
    unsigned int checksum : 16;
    unsigned int reserved : 4;
} __attribute__((packed));

/* Nested anonymous struct */
struct Container {
    struct {
        int32_t id;
        char tag[32];
    } metadata;
    
    union {
        int64_t timestamp;
        double epoch;
    } time_data;
    
    struct Point position;
} __attribute__((aligned(64)));

/* Struct with flexible array member */
struct DynamicArray {
    size_t capacity;
    size_t length;
    int32_t data[];
};

/* Complex struct with function pointers */
struct Plugin {
    const char* name;
    int32_t version;
    int (*init)(struct Plugin* self);
    void (*process)(void* data, size_t len);
    void (*cleanup)(struct Plugin* self);
    struct Plugin* next;
};

/* Struct with array of function pointers */
struct Dispatcher {
    char name[64];
    event_handler handlers[16];
    data_processor processors[8];
    int handler_count;
};

/* ========== TYPE_UNION ========== */

/* Simple union */
union Number {
    int32_t as_int;
    float32_t as_float;
    void* as_ptr;
};

/* Complex union with nested struct */
union Variant {
    int64_t integer;
    float64_t floating;
    char* string;
    struct {
        int16_t type;
        int16_t subtype;
        void* payload;
    } custom;
    struct Point point;
    complex64_t complex;
};

/* Union with anonymous struct */
union AnonymousUnion {
    struct {
        int a, b;
    };
    struct {
        long x, y;
    };
    double array[2];
};

/* ========== TYPE_ARRAY - Multi-dimensional arrays ========== */

/* Array of structs */
struct Point point_grid[10][10];

/* Array of pointers */
struct Plugin* plugin_registry[32];

/* Complex multi-dimensional array */
int32_t*** deep_array[5][5];  /* TYPE_ARRAY of TYPE_POINTER of TYPE_POINTER of TYPE_POINTER of TYPE_SCALAR */

/* Array of unions */
union Variant variant_array[100];

/* Array of arrays of function pointers */
simple_callback callback_matrix[8][4];

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */

/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Struct with vector types */
struct VectorData {
    float32x8_t simd_data[4];
    int32x4_t indices;
    __attribute__((aligned(128))) char padding[128];
};

/* Transparent union attribute */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int* as_int_ptr;
    float* as_float_ptr;
    void* as_void_ptr;
} transparent_union_t;

/* ========== TYPE_POINTER - Complex pointer chains ========== */

/* Multi-level pointers */
int**** quad_ptr;

/* Pointer to array */
int (*array_ptr)[10][20];

/* Pointer to function returning pointer to array */
int (*(*complex_func_ptr)(void))[10];

/* Pointer to struct containing pointer to union */
struct Container** container_ptr_ptr;

/* ========== Global variables for visibility ========== */

volatile struct VectorData global_vector __attribute__((used));
volatile union Variant global_variant __attribute__((used));
volatile struct Dispatcher global_dispatcher __attribute__((used));
volatile opaque_ptr_t global_opaque_ptr __attribute__((used));

/* ========== Function implementations ========== */

/* Callback function implementations */
static int plugin_init_default(struct Plugin* self) {
    if (self && self->name) {
        return 0;
    }
    return -1;
}

static void plugin_process_default(void* data, size_t len) {
    volatile char* ptr = (char*)data;
    for (size_t i = 0; i < len && i < 16; i++) {
        ptr[i] = (char)(i & 0xFF);
    }
}

static void event_handler_example(int event_id, void* user_data) {
    volatile int* counter = (int*)user_data;
    if (counter) {
        *counter += event_id;
    }
}

static int data_processor_example(const void* data, size_t len) {
    const char* str = (const char*)data;
    int sum = 0;
    for (size_t i = 0; i < len && i < 64; i++) {
        sum += str[i];
    }
    return sum;
}

/* Function using vector types */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex parameter types */
static void process_variant(union Variant* v, variant_processor_t processor) {
    if (v && processor) {
        union Variant* result = processor(1, v, event_handler_example);
        if (result) {
            v->integer = result->integer;
        }
    }
}

/* Function using multi-dimensional arrays */
static int traverse_matrix(struct Point grid[10][10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += (int)(grid[i][j].x + grid[i][j].y + grid[i][j].z);
        }
    }
    return sum;
}

/* Function with pointer chains */
static int dereference_quad_ptr(int**** ptr) {
    if (ptr && *ptr && **ptr && ***ptr) {
        return ****ptr;
    }
    return 0;
}

/* ========== Main function ========== */

int main(void) {
    int result = 0;
    volatile int event_counter = 0;
    
    /* 1. Initialize structs and unions */
    struct Container container = {
        .metadata = { .id = 1001, .tag = "test_container" },
        .time_data = { .timestamp = 1234567890 },
        .position = { .x = 1.0f, .y = 2.0f, .z = 3.0f }
    };
    
    union Variant variant = {
        .custom = { .type = 1, .subtype = 2, .payload = &container }
    };
    
    /* 2. Use vector types */
    float32x8_t vec_a = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
    float32x8_t vec_b = { 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
    float32x8_t vec_sum = vector_add(vec_a, vec_b);
    
    /* Store vector result in global */
    for (int i = 0; i < 8; i++) {
        global_vector.simd_data[0][i] = vec_sum[i];
        result += (int)vec_sum[i];
    }
    
    /* 3. Populate and use struct with function pointers */
    struct Plugin plugin = {
        .name = "test_plugin",
        .version = 1,
        .init = plugin_init_default,
        .process = plugin_process_default,
        .cleanup = 0,
        .next = 0
    };
    
    if (plugin.init(&plugin) == 0) {
        char buffer[32];
        plugin.process(buffer, sizeof(buffer));
        result += buffer[0];
    }
    
    /* 4. Use multi-dimensional arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            point_grid[i][j].x = (float)i;
            point_grid[i][j].y = (float)j;
            point_grid[i][j].z = (float)(i * j);
        }
    }
    result += traverse_matrix(point_grid);
    
    /* 5. Process union with different type accesses */
    variant.integer = 42;
    result += (int)variant.integer;
    
    variant.floating = 3.14159;
    result += (int)variant.floating;
    
    variant.string = (char*)global_strings[0];
    if (variant.string) {
        result += variant.string[0];
    }
    
    /* 6. Use callback through function pointer */
    global_dispatcher.handlers[0] = event_handler_example;
    if (global_dispatcher.handlers[0]) {
        global_dispatcher.handlers[0](5, &event_counter);
        result += event_counter;
    }
    
    /* 7. Use data processor callback */
    global_dispatcher.processors[0] = data_processor_example;
    if (global_dispatcher.processors[0]) {
        result += global_dispatcher.processors[0]("test", 4);
    }
    
    /* 8. Process variant with callback processor */
    process_variant(&variant, 0);
    
    /* 9. Use pointer chains */
    int val = 999;
    int* p1 = &val;
    int** p2 = &p1;
    int*** p3 = &p2;
    quad_ptr = &p3;
    
    result += dereference_quad_ptr(quad_ptr);
    
    /* 10. Use array of pointers */
    plugin_registry[0] = &plugin;
    if (plugin_registry[0]) {
        result += plugin_registry[0]->version;
    }
    
    /* 11. Use complex array types */
    int32_t value = 123;
    int32_t* ptr1 = &value;
    int32_t** ptr2 = &ptr1;
    int32_t*** ptr3 = &ptr2;
    deep_array[0][0] = ptr3;
    
    if (deep_array[0][0] && *deep_array[0][0] && **deep_array[0][0]) {
        result += ***deep_array[0][0];
    }
    
    /* 12. Use callback matrix */
    callback_matrix[0][0] = (simple_callback)0;
    callback_matrix[1][1] = (simple_callback)plugin.process;
    
    /* 13. Process variant array */
    for (int i = 0; i < 10; i++) {
        variant_array[i].integer = i * 100;
        result += (int)variant_array[i].integer;
    }
    
    /* 14. Use transparent union */
    transparent_union_t tu;
    int int_val = 456;
    tu.as_int_ptr = &int_val;
    result += *tu.as_int_ptr;
    
    /* 15. Use anonymous union */
    union AnonymousUnion au;
    au.a = 10;
    au.b = 20;
    result += au.x + au.y;
    
    /* 16. Ensure all global strings are referenced */
    for (int i = 0; global_strings[i]; i++) {
        result += global_strings[i][0];
    }
    
    /* Return deterministic result based on all operations */
    return result & 0xFF;  /* Return lower 8 bits as observable output */
}

/* Additional unused types to ensure they appear in type tables */
struct __attribute__((packed)) ExtraPackedStruct {
    char a;
    int b;
    char c;
};

typedef enum {
    STATE_IDLE,
    STATE_ACTIVE,
    STATE_ERROR
} StateEnum;

union __attribute__((packed)) PackedUnion {
    struct {
        unsigned char a : 2;
        unsigned char b : 3;
        unsigned char c : 3;
    } bits;
    unsigned char byte;
};
