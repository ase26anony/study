/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declaration for undefined type */
struct opaque;  /* TYPE_UNDEFINED candidate */

/* GCC-specific vector type extension */
typedef float __attribute__((vector_size(32))) float32x8_t;  /* TYPE_LANG_STRUCT */

/* Aligned struct attribute */
struct __attribute__((aligned(64))) AlignedData {
    double values[8];
    int flags;
};

/* Packed struct attribute */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    double d;
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef int (*comparator_t)(const void*, const void*);  /* TYPE_CALLBACK */
typedef void (*event_handler)(int event_id, void* user_data);  /* TYPE_CALLBACK */
typedef float (*transform_fn)(float*, int);  /* TYPE_CALLBACK */

/* Complex callback signature */
typedef struct Result* (*processor_t)(int, float**, void (*)(int));  /* TYPE_CALLBACK */

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Nested anonymous struct */
struct Container {
    struct {  /* Anonymous struct */
        int a;
        char b;
        unsigned short:4;  /* Bit-field */
        unsigned short:12; /* Another bit-field */
    } inner;
    
    union {  /* Nested union */
        long x;
        double y;
        void* ptr;
    } data;
    
    volatile int counter;  /* Prevent optimization */
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t length;
    int data[];  /* Flexible array member */
};

/* Complex struct with function pointers */
struct Plugin {  /* TYPE_USER_STRUCT */
    const char* name;
    int version;
    int (*init)(void);  /* TYPE_CALLBACK member */
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
    
    /* Nested pointer to another complex type */
    struct Container* config;
};

/* Bit-field struct */
struct Register {
    unsigned int flag_a:1;
    unsigned int flag_b:1;
    unsigned int mode:3;
    unsigned int value:10;
    unsigned int:17;  /* Padding */
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    float as_float;
    void* as_ptr;
    struct {  /* Anonymous struct in union */
        short len;
        char buf[];  /* Flexible array in union member */
    } as_string;
    
    long long as_longlong;
    double as_double;
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

/* ========== TYPE_SCALAR ========== */
/* Use all fundamental scalar types */
struct Scalars {
    char c;
    signed char sc;
    unsigned char uc;
    short s;
    unsigned short us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;
    float f;
    double d;
    long double ld;
    _Bool b;
    _Complex float cf;   /* Complex types */
    _Complex double cd;
};

/* ========== TYPE_STRING ========== */
/* String literals and arrays */
const char* error_messages[] = {  /* TYPE_STRING array */
    "Error: Invalid operation",
    "Warning: Deprecated feature",
    "Info: Processing complete",
    NULL
};

const char program_name[] = "GCC Type Coverage Test";  /* TYPE_STRING */

/* ========== TYPE_ARRAY / TYPE_POINTER ========== */
/* Multi-dimensional array of structs */
struct Node* adjacency_matrix[10][10];  /* TYPE_POINTER in TYPE_ARRAY */

/* Array of function pointers */
transform_fn transforms[5];  /* TYPE_CALLBACK array */

/* Complex pointer chain */
int ****quad_ptr;  /* Multi-level indirection */

/* Pointer to array */
int (*array_ptr)[20];

/* Array of pointers to unions */
union Variant* variant_array[8];

/* ========== Global instances for visibility ========== */
struct Plugin plugin_registry[3];
struct Container global_container;
struct Scalars global_scalars;
union Variant global_variant;
struct AlignedData aligned_instance;
struct PackedStruct packed_instance;
struct Register status_register;
struct TaggedVariant tagged_data;

float32x8_t vector_data;  /* GCC vector type */

/* ========== Function implementations ========== */
static int default_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_event_handler(int event_id, void* user_data) {
    volatile int* counter = (int*)user_data;
    *counter += event_id;
}

static float sample_transform(float* data, int len) {
    float sum = 0.0f;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum / len;
}

static int plugin_init(void) {
    return 0;
}

static void plugin_process(int data) {
    global_container.counter += data;
}

static void plugin_cleanup(struct Plugin* self) {
    self->config = NULL;
}

/* Function using complex callback signature */
static struct Result* complex_processor(int count, float** data, void (*callback)(int)) {
    /* Dummy implementation */
    (void)count;
    (void)data;
    if (callback) callback(42);
    return NULL;
}

/* Function with complex parameter types */
static void process_variants(union Variant* vars, int count, event_handler handler) {
    for (int i = 0; i < count; i++) {
        if (handler) {
            handler(i, &vars[i]);
        }
    }
}

/* Function using GCC vector type */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;  /* Vector addition */
}

/* ========== Main function ========== */
int main(void) {
    /* 1. Initialize structs and unions */
    global_container.inner.a = 42;
    global_container.inner.b = 'X';
    global_container.data.x = 0xDEADBEEF;
    global_container.counter = 0;
    
    global_scalars.c = 'A';
    global_scalars.i = -100;
    global_scalars.ui = 100;
    global_scalars.f = 3.14159f;
    global_scalars.d = 2.718281828459045;
    global_scalars.ld = 1.618033988749895L;
    global_scalars.b = 1;
    global_scalars.cf = 1.0f + 2.0fi;
    global_scalars.cd = 3.0 + 4.0i;
    
    global_variant.as_int = 314159;
    
    /* 2. Initialize plugin registry with function pointers */
    plugin_registry[0].name = "PluginA";
    plugin_registry[0].version = 1;
    plugin_registry[0].init = plugin_init;
    plugin_registry[0].process = plugin_process;
    plugin_registry[0].cleanup = plugin_cleanup;
    plugin_registry[0].config = &global_container;
    
    plugin_registry[1].name = "PluginB";
    plugin_registry[1].version = 2;
    plugin_registry[1].init = plugin_init;
    plugin_registry[1].process = plugin_process;
    plugin_registry[1].cleanup = plugin_cleanup;
    plugin_registry[1].config = NULL;
    
    /* 3. Call function through pointer */
    if (plugin_registry[0].init) {
        plugin_registry[0].init();
    }
    if (plugin_registry[0].process) {
        plugin_registry[0].process(10);
    }
    
    /* 4. Use GCC vector type */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_result = vector_add(vec_a, vec_b);
    vector_data = vec_result;  /* Store to global */
    
    /* 5. Initialize and use multi-dimensional array */
    static struct Node* local_matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            local_matrix[i][j] = NULL;
        }
    }
    adjacency_matrix[0][0] = local_matrix[0][0];
    
    /* 6. Initialize function pointer array */
    transforms[0] = sample_transform;
    transforms[1] = sample_transform;
    
    /* 7. Create pointer chain */
    int val = 42;
    int *p1 = &val;
    int **p2 = &p1;
    int ***p3 = &p2;
    int ****p4 = &p3;
    quad_ptr = p4;
    
    /* 8. Process union types */
    union Variant local_variants[3];
    local_variants[0].as_int = 100;
    local_variants[1].as_float = 3.14f;
    local_variants[2].as_ptr = &global_container;
    
    process_variants(local_variants, 3, sample_event_handler);
    
    /* 9. Use all string types */
    const char* current_error = error_messages[0];
    const char* program_desc = program_name;
    (void)current_error;
    (void)program_desc;
    
    /* 10. Initialize aligned and packed structs */
    for (int i = 0; i < 8; i++) {
        aligned_instance.values[i] = i * 1.5;
    }
    aligned_instance.flags = 0xFF;
    
    packed_instance.a = 'Z';
    packed_instance.b = 65535;
    packed_instance.c = -32768;
    packed_instance.d = 1.23456789e10;
    
    /* 11. Use bit-field struct */
    status_register.flag_a = 1;
    status_register.flag_b = 0;
    status_register.mode = 4;
    status_register.value = 512;
    
    /* 12. Use tagged union */
    tagged_data.type = STRING;
    tagged_data.value.s = "Hello, GCC!";
    
    /* 13. Use complex callback */
    processor_t proc = complex_processor;
    float* data_array[2];
    float data1[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float data2[5] = {5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    data_array[0] = data1;
    data_array[1] = data2;
    
    /* Call through complex callback */
    struct Result* res = proc(2, data_array, sample_event_handler);
    (void)res;
    
    /* 14. Compute deterministic return value using all type categories */
    int hash = 0;
    
    /* Include contributions from all major type instances */
    hash += global_container.inner.a;
    hash += global_container.inner.b;
    hash += global_container.counter;
    hash += (int)global_scalars.f;
    hash += (int)global_scalars.d;
    hash += global_variant.as_int % 1000;
    hash += aligned_instance.flags;
    hash += packed_instance.b;
    hash += status_register.value;
    
    /* Include vector elements (first 4) */
    float* vec_elements = (float*)&vec_result;
    for (int i = 0; i < 4; i++) {
        hash += (int)vec_elements[i];
    }
    
    /* Include string lengths */
    for (int i = 0; error_messages[i] != NULL; i++) {
        hash += strlen(error_messages[i]);
    }
    hash += strlen(program_name);
    
    /* Ensure non-zero, deterministic result */
    return (hash > 0) ? (hash % 255) : 1;
}

/* Dummy struct definition for forward reference */
struct Result {
    int code;
    char* message;
};

/* Additional type to ensure TYPE_UNDEFINED gets resolved */
struct opaque {
    void* data;
    int size;
};
