/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== TYPE_UNDEFINED / Forward Declarations ==================== */
struct opaque;                     /* Incomplete/undefined type */
extern struct opaque *global_opaque_ptr;  /* TYPE_UNDEFINED via pointer */

/* ==================== TYPE_SCALAR - All fundamental types ==================== */
typedef _Complex float complex_float;   /* GCC complex type */
typedef _Complex double complex_double;
volatile _Bool global_flag = 1;
const long double pi_approx = 3.14159265358979323846L;

/* ==================== TYPE_STRING - String literals and pointers ==================== */
const char *error_messages[] = {"Error", "Warning", "Info", NULL};  /* Array of strings */
char global_buffer[] = "Global string data";

/* ==================== TYPE_CALLBACK - Function pointers and typedefs ==================== */
typedef void (*event_handler)(int event_id, void *user_data);
typedef int (*comparator_fn)(const void *, const void *);
typedef void (*void_fn_ptr)(void);

struct Plugin {
    const char *name;
    int (*init)(void *config);
    void (*process)(int data);
    event_handler on_error;
};

/* Callback implementations */
int default_init(void *config) { 
    (void)config; 
    return 0; 
}
void default_process(int data) { 
    printf("Processing: %d\n", data); 
}
void error_callback(int event_id, void *user_data) {
    printf("Error %d: %s\n", event_id, (char*)user_data);
}

/* ==================== TYPE_STRUCT / TYPE_USER_STRUCT ==================== */
/* Nested anonymous struct */
struct Container {
    struct {                    /* Anonymous struct */
        int a;
        char b;
        unsigned bitfield : 4;  /* Bit-field */
    } inner;
    union {                     /* Nested union */
        long x;
        double y;
        void *ptr;
    } data;
    volatile int counter;
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t length;
    int items[];                /* Flexible array member */
};

/* Packed struct with alignment attribute */
struct __attribute__((packed, aligned(2))) PackedData {
    char id;
    short count;
    int value;
};

/* Complex nested struct with function pointer */
struct System {
    struct Container container;
    struct Plugin *plugins[3];  /* Array of pointers to Plugin */
    struct System *next;        /* Linked list */
    void (*shutdown)(struct System*);
};

/* ==================== TYPE_UNION ==================== */
union Variant {
    int as_int;
    void *as_ptr;
    float as_float;
    struct {                    /* Anonymous struct inside union */
        short len;
        char buf[];             /* Flexible array in union-struct */
    } as_string;
    complex_double as_complex;
};

/* Tagged union */
struct TaggedVariant {
    enum { INT, PTR, FLOAT, STRING } type;
    union {
        int i;
        void *p;
        float f;
        char *s;
    } value;
};

/* ==================== TYPE_ARRAY - Multi-dimensional and complex arrays ==================== */
struct Node {
    int id;
    struct Node *edges[5];      /* Array of pointers */
};

/* Complex array types */
struct Node *adjacency_matrix[10][10];                 /* 2D array of pointers */
int (*signal_processor)(float[][256], int**);          /* Function pointer with array params */
const int *const multi_array[3][4][5];                 /* 3D array of const pointers to const int */

/* ==================== TYPE_LANG_STRUCT - GCC extensions ==================== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Struct with vector type */
struct VectorData {
    float32x8_t vec;
    int32x4_t mask;
    __attribute__((aligned(64))) double aligned_double;
};

/* ==================== TYPE_POINTER - Complex pointer chains ==================== */
int ****quadruple_ptr;           /* Four-level indirection */
void (* volatile volatile_fn_ptr)(int);  /* Volatile function pointer */

/* ==================== Global instances ==================== */
struct Plugin plugin_registry[3] = {
    {"PluginA", default_init, default_process, error_callback},
    {"PluginB", default_init, default_process, NULL},
    {"PluginC", NULL, NULL, error_callback}
};

struct System global_system = {
    .container = {{1, 'X', 7}, {.x = 100}, 42},
    .plugins = {&plugin_registry[0], &plugin_registry[1], NULL},
    .next = NULL,
    .shutdown = NULL
};

union Variant global_variant = {.as_int = -1};

float32x8_t global_vector = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};

/* ==================== Helper functions ==================== */
int process_adjacency(struct Node *matrix[10][10]) {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (matrix[i][j] != NULL) count++;
        }
    }
    return count;
}

void traverse_pointer_chain(int ****ptr) {
    if (ptr && *ptr && **ptr && ***ptr) {
        ****ptr = 42;
    }
}

complex_double complex_operation(complex_double a, complex_double b) {
    return a + b;
}

/* ==================== main() - Exercise all types ==================== */
int main(void) {
    /* 1. Struct and union operations */
    struct Container local_container = {
        .inner = {2, 'Y', 9},
        .data = {.y = 3.14159},
        .counter = 1000
    };
    
    union Variant local_variant;
    local_variant.as_float = 2.71828f;
    
    struct TaggedVariant tagged = {.type = STRING, .value.s = "Hello"};
    
    /* 2. Array operations */
    struct Node *local_matrix[10][10] = {0};
    local_matrix[0][1] = &(struct Node){1, {NULL}};
    local_matrix[3][3] = &(struct Node){2, {NULL}};
    int edge_count = process_adjacency(local_matrix);
    
    /* 3. Function pointer calls */
    if (plugin_registry[0].init) {
        plugin_registry[0].init(&local_container);
    }
    if (plugin_registry[0].process) {
        plugin_registry[0].process(edge_count);
    }
    if (plugin_registry[0].on_error) {
        plugin_registry[0].on_error(1, "Test error");
    }
    
    /* 4. Vector operations (GCC extension) */
    float32x8_t vec_a = global_vector;
    float32x8_t vec_b = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    float32x8_t vec_sum = vec_a + vec_b;  /* Vector addition */
    
    /* 5. Pointer chain traversal */
    int value = 0;
    int *p1 = &value;
    int **p2 = &p1;
    int ***p3 = &p2;
    int ****p4 = &p3;
    quadruple_ptr = p4;
    traverse_pointer_chain(quadruple_ptr);
    
    /* 6. Union type switching */
    int result = 0;
    switch (tagged.type) {
        case INT:    result = tagged.value.i; break;
        case PTR:    result = (tagged.value.p != NULL); break;
        case FLOAT:  result = (int)tagged.value.f; break;
        case STRING: result = strlen(tagged.value.s); break;
    }
    
    /* 7. Complex number operations */
    complex_double c1 = 1.0 + 2.0 * I;
    complex_double c2 = 3.0 - 4.0 * I;
    complex_double c3 = complex_operation(c1, c2);
    
    /* 8. Flexible array member simulation */
    size_t dyn_size = 5;
    struct DynamicArray *dyn = malloc(sizeof(struct DynamicArray) + dyn_size * sizeof(int));
    if (dyn) {
        dyn->length = dyn_size;
        for (size_t i = 0; i < dyn_size; i++) {
            dyn->items[i] = (int)i * 10;
        }
        free(dyn);
    }
    
    /* 9. Packed struct access */
    struct PackedData packed = {'A', 255, 65536};
    int packed_sum = packed.id + packed.count + packed.value;
    
    /* 10. String array processing */
    int total_chars = 0;
    for (int i = 0; error_messages[i] != NULL; i++) {
        total_chars += (int)strlen(error_messages[i]);
    }
    
    /* Compute deterministic return value using all manipulated data */
    int hash = (local_container.inner.a + 
                (int)local_variant.as_float + 
                edge_count + 
                value + 
                result + 
                (int)__real__ c3 + 
                packed_sum + 
                total_chars) & 0x7FFFFFFF;
    
    printf("Final hash: %d\n", hash);
    return hash == 0 ? 1 : 0;  /* Non-zero return */
}
