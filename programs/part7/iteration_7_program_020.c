/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== 1. USER-DEFINED STRUCTS AND UNIONS ========== */

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;
extern struct opaque *global_opaque_ptr;

/* Complex nested struct with anonymous members */
struct Container {
    struct {                    /* Nested anonymous struct */
        int a;
        char b;
        unsigned bitfield1 : 3;
        unsigned bitfield2 : 5;
    } inner;
    union {                     /* Anonymous union */
        long x;
        double y;
        void *ptr;
    } data;
    volatile int counter;
    const char *name;
} __attribute__((packed));

/* Union with flexible array member */
union Variant {
    int as_int;
    void* as_ptr;
    struct {
        short len;
        char buf[];             /* Flexible array member */
    } as_string;
    long double as_long_double;
};

/* Bitfield-heavy struct */
struct RegisterMap {
    unsigned int status : 4;
    unsigned int control : 8;
    unsigned int data : 16;
    unsigned int reserved : 4;
    unsigned char padding[3];
} __attribute__((aligned(16)));

/* Struct with function pointer member */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void);
    void (*process)(int);
    void (*cleanup)(struct Plugin*);
};

/* ========== 2. FUNCTION POINTERS AND CALLBACKS ========== */

/* Typedef for complex function pointer (TYPE_CALLBACK) */
typedef void (*event_handler)(int event_id, void* user_data);
typedef int (*comparator_t)(const void*, const void*);
typedef union Variant* (*transformer_t)(struct Container*, int);

/* Callback struct with multiple function pointers */
struct CallbackRegistry {
    event_handler on_start;
    event_handler on_data;
    event_handler on_end;
    void (*error_callback)(int, const char*);
};

/* Function returning function pointer */
static comparator_t get_comparator(int type) {
    return (comparator_t)0;  /* Simplified for example */
}

/* ========== 3. ARRAYS AND POINTER CHAINS ========== */

/* Multi-dimensional array of struct pointers */
struct Node* adjacency_matrix[10][10];

/* Triple pointer chain */
int ***triple_ptr_chain;

/* Array of function pointers */
event_handler handlers[5];

/* Complex array type */
int (*signal_processor)(float[][256], int**, struct Container*);

/* Pointer to array */
int (*ptr_to_array)[20];

/* ========== 4. LANGUAGE-SPECIFIC TYPES ========== */

/* GCC vector types (TYPE_LANG_STRUCT) */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Transparent union attribute */
typedef union {
    int i;
    float f;
    void *p;
} arg_t __attribute__((transparent_union));

/* Aligned struct */
struct CacheLine {
    char data[64];
} __attribute__((aligned(64)));

/* ========== 5. SCALAR AND STRING TYPES ========== */

/* All scalar types */
_Bool flag;
char byte;
signed char sbyte;
unsigned char ubyte;
short small;
unsigned short usmall;
int integer;
unsigned int uinteger;
long big;
unsigned long ubig;
long long huge;
unsigned long long uhuge;
float real;
double dbl;
long double ldbl;
float _Complex fcomplex;
double _Complex dcomplex;
long double _Complex ldcomplex;

/* String arrays */
const char* error_messages[] = {"Error", "Warning", "Info", "Debug", NULL};
char static_strings[][20] = {"Hello", "World", "Test"};

/* ========== 6. GLOBAL VARIABLES WITH COMPLEX TYPES ========== */

/* Ensure type visibility */
struct Plugin plugin_registry[3];
union Variant global_variants[5];
struct Container global_container;
struct CallbackRegistry callbacks;
float32x8_t global_vector;
struct CacheLine cache_lines[8];

/* ========== FUNCTION DEFINITIONS ========== */

/* Callback functions */
static int plugin_init_default(void) {
    return 0;
}

static void plugin_process_default(int value) {
    volatile int tmp = value * 2;
    (void)tmp;
}

static void event_handler_example(int id, void *data) {
    struct Container *c = (struct Container*)data;
    if (c) {
        c->counter++;
    }
}

/* Function using vector type */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex parameters */
static void process_matrix(struct Node* matrix[10][10], int depth) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (matrix[i][j] != NULL) {
                /* Simulate some operation */
                volatile int tmp = i + j + depth;
                (void)tmp;
            }
        }
    }
}

/* Function processing union with runtime condition */
static int process_variant(union Variant *v, int type) {
    switch (type) {
        case 0:
            return v->as_int;
        case 1:
            return (int)(intptr_t)v->as_ptr;
        case 2:
            return v->as_string.len;
        default:
            return -1;
    }
}

/* Function with pointer chain */
static int follow_pointer_chain(int ***chain, int levels) {
    if (levels > 0 && chain && *chain && **chain) {
        return ***chain;
    }
    return 0;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* 1. Initialize structs and unions */
    struct Container local_container = {
        .inner = {.a = 42, .b = 'X'},
        .data = {.x = 123456789L},
        .counter = 0,
        .name = "TestContainer"
    };
    
    union Variant local_variant;
    local_variant.as_int = 100;
    
    struct RegisterMap regs = {
        .status = 0xF,
        .control = 0xA5,
        .data = 0xDEAD,
        .reserved = 0
    };
    
    /* 2. Setup plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "PluginA",
        .version = 1,
        .init = plugin_init_default,
        .process = plugin_process_default,
        .cleanup = NULL
    };
    
    plugin_registry[1].name = "PluginB";
    plugin_registry[1].version = 2;
    plugin_registry[1].init = plugin_init_default;
    plugin_registry[1].process = plugin_process_default;
    
    /* Call function through pointer */
    if (plugin_registry[0].init) {
        plugin_registry[0].init();
    }
    if (plugin_registry[0].process) {
        plugin_registry[0].process(42);
    }
    
    /* 3. Setup callbacks */
    callbacks.on_start = event_handler_example;
    callbacks.on_data = event_handler_example;
    callbacks.on_end = NULL;
    
    if (callbacks.on_start) {
        callbacks.on_start(1, &local_container);
    }
    
    /* 4. Use vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_result = vector_add(vec_a, vec_b);
    global_vector = vec_result;
    
    /* 5. Setup pointer chain */
    int value = 999;
    int *p1 = &value;
    int **p2 = &p1;
    triple_ptr_chain = &p2;
    
    int chain_value = follow_pointer_chain(triple_ptr_chain, 3);
    
    /* 6. Setup multi-dimensional array */
    struct Node* dummy_node = NULL;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            adjacency_matrix[i][j] = dummy_node;
        }
    }
    process_matrix(adjacency_matrix, 2);
    
    /* 7. Process union with runtime condition */
    int variant_result = process_variant(&local_variant, 0);
    
    /* 8. Use all scalar types */
    flag = 1;
    byte = 'A';
    integer = 12345;
    real = 3.14159f;
    dbl = 2.718281828459045;
    fcomplex = 1.0f + 2.0f * I;
    
    /* 9. String operations */
    const char *msg = error_messages[0];
    volatile char first_char = msg[0];
    (void)first_char;
    
    /* 10. Use aligned struct */
    cache_lines[0].data[0] = 0xFF;
    
    /* 11. Complex array/pointer operations */
    int array_2d[5][20];
    ptr_to_array = array_2d;
    
    int (*complex_array)[10][20];
    complex_array = (int(*)[10][20])0;
    
    /* 12. Setup handlers array */
    handlers[0] = event_handler_example;
    handlers[1] = NULL;
    
    /* Compute deterministic return value using all manipulated data */
    int hash = 0;
    hash ^= local_container.inner.a;
    hash ^= local_container.counter;
    hash ^= chain_value;
    hash ^= variant_result;
    hash ^= regs.data;
    hash ^= (int)vec_result[0];
    hash ^= plugin_registry[0].version;
    
    /* Ensure all globals are touched */
    global_container = local_container;
    global_variants[0] = local_variant;
    
    return hash & 0xFF;  /* Return deterministic value */
}

/* Additional type definitions for completeness */

/* Opaque struct pointer usage */
struct opaque* global_opaque_ptr = NULL;

/* Node structure for adjacency matrix */
struct Node {
    int id;
    struct Node** neighbors;
    int neighbor_count;
};

/* Function using transparent union */
static void use_transparent_union(arg_t arg) {
    /* Can pass int, float, or pointer directly */
    (void)arg;
}

/* Static initialization of complex types */
static struct Container static_container = {
    .inner = {.a = -1, .b = 'Z', .bitfield1 = 7, .bitfield2 = 31},
    .data = {.y = 3.141592653589793},
    .counter = 1000,
    .name = "StaticContainer"
};

static union Variant static_variants[2] = {
    {.as_int = 42},
    {.as_ptr = &static_container}
};
