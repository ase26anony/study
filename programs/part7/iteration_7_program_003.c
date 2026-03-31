/* test_rich_types.c - Comprehensive type coverage for GCC gengtype-state.cc */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
struct opaque;  /* Forward declaration - TYPE_UNDEFINED */
typedef struct opaque *opaque_ptr_t;

/* GCC vector extension - TYPE_LANG_STRUCT */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Complex nested anonymous struct */
struct Container {
    struct {  /* Anonymous struct */
        int a;
        char b;
        unsigned bitfield : 3;
        unsigned : 5;  /* Padding */
    } inner;
    
    union {  /* Anonymous union */
        long x;
        double y;
        void *ptr;
    } data;
    
    volatile int counter;
} __attribute__((aligned(64)));

/* Packed struct with bitfields */
struct __attribute__((packed)) PackedData {
    unsigned char type;
    unsigned int value : 24;
    unsigned short checksum;
    unsigned : 8;  /* Padding */
};

/* Struct with flexible array member */
struct DynamicString {
    size_t length;
    char data[];
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    void* as_ptr;
    double as_double;
    struct {
        short len;
        char buf[];  /* Flexible array in union member */
    } as_string;
    
    /* Nested anonymous struct in union */
    struct {
        int tag;
        union {
            float f;
            int i;
        } value;
    } tagged;
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*event_handler)(int event_id, void* user_data);
typedef int (*processor_func)(float data[][256], int** params);
typedef union Variant* (*variant_factory)(int type);

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int data);
    event_handler on_event;
    variant_factory create_variant;
};

/* Callback manager struct */
struct CallbackManager {
    struct Plugin* plugins[10];
    event_handler handlers[5];
    processor_func main_processor;
};

/* ========== TYPE_ARRAY / TYPE_POINTER ========== */
/* Multi-dimensional array of pointers */
struct Node* adjacency_matrix[10][10];

/* Array of function pointers */
static int (*math_ops[5])(int, int);

/* Pointer to array */
typedef int (*array_ptr_t)[20];

/* Complex pointer chain */
struct TriplePointer {
    int ****quad_ptr;
    struct Container*** container_ptr_ptr;
    array_ptr_t matrix_ptr;
};

/* ========== TYPE_SCALAR ========== */
/* Use all fundamental scalar types */
struct AllScalars {
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
    float _Complex fc;
    double _Complex dc;
    long double _Complex ldc;
    intptr_t iptr;
    uintptr_t uptr;
    ptrdiff_t pdiff;
    size_t sz;
};

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {
    "Fatal Error",
    "Warning",
    "Information",
    "Debug"
};

static const char* const static_strings[] = {
    "Constant string 1",
    "Constant string 2",
    NULL
};

/* ========== Global Variables ========== */
struct Container global_container = {
    .inner = { .a = 42, .b = 'X', .bitfield = 3 },
    .data = { .x = 123456789L },
    .counter = 0
};

union Variant global_variant = { .as_int = -1 };

struct Plugin plugin_registry[3];
struct CallbackManager cb_manager;

struct AllScalars all_scalars_instance;

float32x8_t global_vector1, global_vector2;
int32x4_t int_vector;

/* ========== Function Definitions ========== */
static void sample_event_handler(int event_id, void* user_data) {
    volatile int *counter = (int*)user_data;
    if (counter) (*counter)++;
    (void)event_id;  /* Prevent unused parameter warning */
}

static int sample_init(void) {
    return 0;
}

static void sample_process(int data) {
    global_container.counter += data;
}

static union Variant* create_int_variant(int type) {
    static union Variant v;
    v.as_int = type * 100;
    return &v;
}

static int add_func(int a, int b) { return a + b; }
static int sub_func(int a, int b) { return a - b; }
static int mul_func(int a, int b) { return a * b; }

static int process_matrix(float data[][256], int** params) {
    if (!data || !params) return -1;
    return (int)(data[0][0] + **params);
}

/* Function using GCC vector types */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex pointer parameters */
static void process_pointers(struct TriplePointer *tp, 
                            struct Node* matrix[10][10]) {
    if (!tp || !matrix) return;
    
    /* Access through pointer chain */
    volatile int dummy = 0;
    if (tp->quad_ptr && tp->container_ptr_ptr) {
        dummy = 1;
    }
    
    /* Use matrix */
    if (matrix[0][0]) {
        dummy = 2;
    }
    
    (void)dummy;  /* Prevent unused variable warning */
}

/* Function processing union with type tag */
static int process_variant(union Variant *v) {
    if (!v) return -1;
    
    switch (v->tagged.tag) {
        case 0:
            return (int)v->tagged.value.f;
        case 1:
            return v->tagged.value.i;
        default:
            return v->as_int;
    }
}

/* ========== Main Function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .init = sample_init,
        .process = sample_process,
        .on_event = sample_event_handler,
        .create_variant = create_int_variant
    };
    
    /* Call function through pointer */
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    /* 2. Initialize callback manager */
    cb_manager.plugins[0] = &plugin_registry[0];
    cb_manager.handlers[0] = sample_event_handler;
    cb_manager.main_processor = process_matrix;
    
    /* 3. Setup math operations array */
    math_ops[0] = add_func;
    math_ops[1] = sub_func;
    math_ops[2] = mul_func;
    
    /* Use function pointer array */
    if (math_ops[0]) {
        result += math_ops[0](10, 20);
    }
    
    /* 4. Initialize all scalar types */
    all_scalars_instance = (struct AllScalars){
        .c = 'A',
        .sc = -10,
        .uc = 200,
        .s = -1000,
        .us = 1000,
        .i = -100000,
        .ui = 100000,
        .l = -1000000L,
        .ul = 1000000UL,
        .ll = -1000000000LL,
        .ull = 1000000000ULL,
        .f = 3.14159f,
        .d = 2.718281828459045,
        .ld = 1.618033988749895L,
        .b = 1,
        .fc = 1.0f + 2.0f * I,
        .dc = 3.0 + 4.0 * I,
        .ldc = 5.0L + 6.0L * I,
        .iptr = (intptr_t)&global_container,
        .uptr = (uintptr_t)&global_variant,
        .pdiff = (ptrdiff_t)(&plugin_registry[1] - &plugin_registry[0]),
        .sz = sizeof(struct AllScalars)
    };
    
    /* 5. Use GCC vector types */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = vector_add(vec1, vec2);
    
    /* Extract value from vector */
    float vec_element = ((float*)&vec_sum)[0];
    result += (int)vec_element;
    
    /* 6. Process union variant */
    global_variant.tagged.tag = 0;
    global_variant.tagged.value.f = 3.14f;
    result += process_variant(&global_variant);
    
    global_variant.tagged.tag = 1;
    global_variant.tagged.value.i = 42;
    result += process_variant(&global_variant);
    
    /* 7. Use string arrays */
    const char* current_error = error_messages[0];
    while (*current_error) {
        result += *current_error;
        current_error++;
    }
    
    /* 8. Create and use pointer chains */
    int ****quad_ptr = NULL;
    struct Container** container_ptr_ptr = NULL;
    int matrix[20][20];
    array_ptr_t matrix_ptr = matrix;
    
    struct TriplePointer tp = {
        .quad_ptr = &quad_ptr,
        .container_ptr_ptr = &container_ptr_ptr,
        .matrix_ptr = matrix_ptr
    };
    
    struct Node* local_matrix[10][10] = {0};
    process_pointers(&tp, local_matrix);
    
    /* 9. Use packed struct */
    struct PackedData packed = {
        .type = 1,
        .value = 0x00ABCD,
        .checksum = 0x1234
    };
    result += packed.type + packed.value + packed.checksum;
    
    /* 10. Trigger callback through manager */
    if (cb_manager.handlers[0]) {
        cb_manager.handlers[0](1, &global_container.counter);
    }
    
    /* 11. Use complex numbers */
    double _Complex z = 1.0 + 2.0 * I;
    result += (int)(creal(z) + cimag(z));
    
    /* 12. Use all scalar types in computation */
    result += all_scalars_instance.i;
    result += (int)all_scalars_instance.f;
    result += all_scalars_instance.b;
    
    /* Return deterministic result based on all operations */
    return result & 0xFF;  /* Return lower 8 bits as observable result */
}

/* Additional unused types to ensure they're seen by the type system */
struct UnusedTypes {
    /* Multi-level pointer */
    int ********************** insane_ptr_chain;
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*func_ptr_array[5])(void))[10];
    
    /* Nested anonymous struct/union */
    struct {
        union {
            struct {
                int deep_a;
                int deep_b;
            };
            long long deep_ll;
        };
    } deeply_nested;
};

/* External reference to forward-declared type */
extern struct opaque* get_opaque(void);
