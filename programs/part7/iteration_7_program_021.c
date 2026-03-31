/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Incomplete type - TYPE_UNDEFINED */
typedef struct opaque* opaque_ptr_t;

/* ========== TYPE_SCALAR - All Fundamental Types ========== */
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
const char* error_messages[] = {"Error", "Warning", "Info", "Debug", "Trace"};
const wchar_t* wide_strings[] = {L"Hello", L"World", L"Test"};

/* ========== TYPE_CALLBACK - Function Pointers ========== */
typedef void (*event_handler)(int event_id, void* user_data);
typedef int (*processor_func)(float** data, size_t len);
typedef void (*cleanup_func)(void);

/* Complex callback signature */
typedef struct result* (*transform_cb)(const void* input, 
                                       int (*validator)(const void*), 
                                       void* context);

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */

/* Simple nested anonymous struct */
struct Container {
    struct {                    /* Anonymous struct */
        int a;
        char b;
        unsigned bitfield : 4;  /* Bit-field */
        unsigned : 4;           /* Unnamed bit-field */
    } inner;
    
    union {                     /* Anonymous union */
        long x;
        double y;
        void* ptr;
    } data;
    
    volatile int counter;       /* Volatile member */
};

/* Struct with flexible array member */
struct DynamicBuffer {
    size_t capacity;
    size_t length;
    char data[];                /* Flexible array member */
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedData {
    int16_t id;
    char tag;
    byte_t flags;
};

/* Struct with function pointer member */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    cleanup_func cleanup;
    event_handler on_event;
};

/* Complex nested struct */
struct TreeNode {
    struct TreeNode* left;
    struct TreeNode* right;
    struct TreeNode* parent;
    union {
        int int_value;
        double float_value;
        const char* string_value;
    } data;
    unsigned visited : 1;
    unsigned is_leaf : 1;
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    double as_double;
    struct {
        short len;
        char buf[];             /* Flexible array in union member */
    } as_string;
    
    struct {                    /* Anonymous struct in union */
        int type;
        union {                 /* Nested anonymous union */
            long l;
            double d;
        } value;
    } as_complex;
};

/* ========== TYPE_LANG_STRUCT - GCC Extensions ========== */
/* GCC Vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {
    byte_t data[64];
    int tag;
};

/* Transparent union */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int i;
    long l;
} trans_union_t;

/* ========== TYPE_ARRAY / TYPE_POINTER Complex Examples ========== */

/* Multi-dimensional array of structs */
struct Node* adjacency_matrix[10][10];

/* Array of function pointers */
processor_func processors[5];

/* Pointer to array */
int (*array_ptr)[20];

/* Triple pointer */
int ***triple_ptr_chain;

/* Complex function pointer with array parameter */
int (*signal_processor)(float[][256], int**, size_t);

/* ========== Global Variables for Type Visibility ========== */
struct Container global_container = {
    .inner = {42, 'X', 7},
    .data = {.y = 3.14159}
};

struct Plugin plugin_registry[3];
union Variant global_variants[5];

float32x8_t global_vector1, global_vector2;
struct CacheLine cache_lines[8];

/* Complex static initialization */
static struct TreeNode static_tree = {
    .left = NULL,
    .right = NULL,
    .data = {.int_value = 100},
    .visited = 0,
    .is_leaf = 1
};

/* ========== Function Definitions ========== */

/* Callback function implementations */
static int default_validator(const void* data) {
    return data != NULL;
}

static void simple_cleanup(void) {
    /* Do nothing */
}

static int plugin_init(void) {
    return 0;
}

static void plugin_process(int value) {
    global_container.counter += value;
}

static void handle_event(int event_id, void* user_data) {
    struct Container* c = (struct Container*)user_data;
    if (c) {
        c->inner.a += event_id;
    }
}

/* Function using vector type */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;  /* GCC vector operation */
}

/* Function with complex pointer chain */
static int process_triple_pointer(int ***ptr) {
    if (ptr && *ptr && **ptr) {
        return ***ptr;
    }
    return -1;
}

/* Function using multi-dimensional array */
static void init_adjacency_matrix(void) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            adjacency_matrix[i][j] = NULL;
        }
    }
}

/* Process variant union */
static void process_variant(union Variant* v, int type) {
    switch (type) {
        case 0:
            v->as_int = 42;
            break;
        case 1:
            v->as_float = 3.14f;
            break;
        case 2:
            /* Access flexible array member through pointer */
            if (v->as_string.len > 0) {
                /* Would need actual allocation in real code */
            }
            break;
    }
}

/* Function with complex signature matching transform_cb */
static struct result* complex_transform(const void* input,
                                        int (*validator)(const void*),
                                        void* context) {
    static struct result dummy_result;
    if (validator && validator(input)) {
        return &dummy_result;
    }
    return NULL;
}

/* Main function with comprehensive type usage */
int main(void) {
    /* 1. Initialize structs and unions */
    struct Container local_container = {
        .inner = {100, 'A', 3},
        .data = {.x = 123456789L},
        .counter = 0
    };
    
    union Variant local_variant;
    local_variant.as_int = 42;
    
    /* 2. Initialize plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .init = plugin_init,
        .process = plugin_process,
        .cleanup = simple_cleanup,
        .on_event = handle_event
    };
    
    /* 3. Call function through pointer */
    if (plugin_registry[0].init) {
        plugin_registry[0].init();
    }
    
    if (plugin_registry[0].on_event) {
        plugin_registry[0].on_event(1, &local_container);
    }
    
    /* 4. Use vector types */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_result = vector_add(vec1, vec2);
    
    /* Store in global for visibility */
    global_vector1 = vec1;
    global_vector2 = vec_result;
    
    /* 5. Complex pointer operations */
    int value = 42;
    int *p1 = &value;
    int **p2 = &p1;
    int ***p3 = &p2;
    triple_ptr_chain = p3;
    
    int extracted = process_triple_pointer(p3);
    
    /* 6. Array operations */
    init_adjacency_matrix();
    
    int matrix[20];
    array_ptr = &matrix;
    
    /* 7. Process union with different types */
    process_variant(&local_variant, 0);
    process_variant(&global_variants[0], 1);
    
    /* 8. Use all scalar types */
    byte_t b = 0xFF;
    int16_t s = -32768;
    int32_t i = 2147483647;
    int64_t l = 9223372036854775807LL;
    float32_t f = 3.14159f;
    float64_t d = 2.718281828459045;
    bool_t flag = 1;
    complex32_t c1 = 1.0f + 2.0if;
    complex64_t c2 = 3.0 + 4.0i;
    
    /* 9. String operations */
    const char* current_msg = error_messages[1];
    size_t msg_len = strlen(current_msg);
    
    /* 10. Use packed struct */
    struct PackedData packed = {.id = 1001, .tag = 'P', .flags = 0x0F};
    
    /* 11. Use aligned cache line */
    cache_lines[0].tag = 1;
    memset(cache_lines[0].data, 0, sizeof(cache_lines[0].data));
    
    /* 12. Complex callback usage */
    transform_cb transformer = complex_transform;
    struct result* res = transformer(&local_container, default_validator, NULL);
    
    /* 13. Compute deterministic result from all operations */
    int final_result = 
        local_container.inner.a + 
        extracted + 
        (int)msg_len + 
        packed.id + 
        (int)(vec_result[0] * 100) + 
        (int)b + 
        (flag ? 1 : 0);
    
    /* Prevent dead code elimination */
    volatile int keep_alive = final_result;
    
    /* Return deterministic value */
    return (final_result > 0) ? 0 : 1;
}

/* Dummy struct for transform callback */
struct result {
    int status;
    void* data;
};

/* Definition of previously opaque type */
struct opaque {
    int secret;
    void* hidden_data;
};

/* Additional functions to ensure type usage */
void __attribute__((constructor)) init_types(void) {
    /* Ensure types are processed at startup */
    static struct opaque revealed = {42, NULL};
    opaque_ptr_t ptr = &revealed;
    (void)ptr;
}

void __attribute__((destructor)) cleanup_types(void) {
    /* Cleanup if needed */
}
