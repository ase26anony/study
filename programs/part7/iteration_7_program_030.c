/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Incomplete/undefined type */
typedef struct opaque* opaque_ptr; /* Pointer to undefined type */

/* ========== TYPE_SCALAR - All fundamental scalar types ========== */
typedef char byte;
typedef short int16;
typedef int int32;
typedef long int64;
typedef long long int128;
typedef float float32;
typedef double float64;
typedef _Bool bool8;
typedef _Complex float complex32;
typedef _Complex double complex64;

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {"Error", "Warning", "Info", "Debug", "Trace"};
const wchar_t* wide_strings[] = {L"Hello", L"World", L"Test"};

/* ========== TYPE_CALLBACK - Function pointers and typedefs ========== */
typedef void (*simple_callback)(void);
typedef int (*processor_func)(int, float, void*);
typedef void (*event_handler)(int, const char*, void*);

struct Plugin {
    const char* name;
    int (*init)(void* context);
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
    event_handler on_event;
};

/* Callback implementations */
int plugin_init(void* ctx) { return 0; }
void plugin_process(int d) { (void)d; }
void plugin_cleanup(struct Plugin* p) { (void)p; }
void handle_event(int id, const char* msg, void* data) { (void)id; (void)msg; (void)data; }

/* ========== TYPE_STRUCT with various attributes ========== */
struct __attribute__((packed)) PackedStruct {
    byte a;
    int32 b;
    char c;
};

struct __attribute__((aligned(64))) AlignedStruct {
    float64 data[8];
    int32 counter;
};

/* Nested anonymous struct/union */
struct Container {
    struct {                    /* Anonymous struct */
        int32 a;
        byte b;
        unsigned int bitfield1 : 4;
        unsigned int bitfield2 : 12;
    } inner;
    union {                     /* Anonymous union */
        int64 x;
        float64 y;
        struct {
            short len;
            char buf[];        /* Flexible array member */
        } str;
    } data;
    struct Container* next;     /* Self-referential pointer */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct Container Container;
typedef struct {
    int id;
    char name[32];
    Container* items;
} UserDefined;

/* ========== TYPE_UNION ========== */
union Variant {
    int32 as_int;
    float64 as_float;
    void* as_ptr;
    struct {
        int16 tag;
        union {
            int32 i;
            float64 f;
            char* s;
        } value;
    } tagged;
    complex64 as_complex;
};

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

struct VectorData {
    float32x8_t vectors[4];
    int32x4_t indices;
    __attribute__((aligned(32))) double aligned_double;
};

/* ========== TYPE_ARRAY - Complex arrays ========== */
struct Node {
    int id;
    struct Node* neighbors[10];
    float weights[10][10];
};

typedef struct Node* NodeMatrix[20][20];

/* Multi-dimensional array of function pointers */
processor_func signal_processors[3][4];

/* ========== TYPE_POINTER - Complex pointer chains ========== */
int**** quad_ptr;                    /* Four-level indirection */
struct Container*** container_grid;  /* 3D pointer array */

/* ========== Global variables for visibility ========== */
volatile struct Plugin plugin_registry[3] = {
    {"PluginA", plugin_init, plugin_process, plugin_cleanup, handle_event},
    {"PluginB", plugin_init, plugin_process, plugin_cleanup, handle_event},
    {"PluginC", plugin_init, plugin_process, plugin_cleanup, handle_event}
};

static union Variant global_variants[5];
static struct VectorData vector_data;
static NodeMatrix node_matrix;

/* ========== Helper functions ========== */
static int process_container(Container* c) {
    int sum = 0;
    volatile Container* current = c;
    
    while (current) {
        sum += current->inner.a;
        sum += current->inner.b;
        
        /* Access union based on runtime condition */
        if (current->inner.a > 0) {
            sum += (int)current->data.x;
        } else {
            sum += (int)current->data.y;
        }
        
        current = current->next;
    }
    return sum;
}

static void use_vector_types(void) {
    float32x8_t v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t v2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t result;
    
    /* Vector addition - will use GCC vector extensions */
    for (int i = 0; i < 8; i++) {
        result[i] = v1[i] + v2[i];
    }
    
    vector_data.vectors[0] = result;
}

static int traverse_pointer_chain(void) {
    int value = 42;
    int* p1 = &value;
    int** p2 = &p1;
    int*** p3 = &p2;
    int**** p4 = &p3;
    
    quad_ptr = p4;
    return ****quad_ptr;
}

static void initialize_node_matrix(void) {
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            node_matrix[i][j] = NULL;
        }
    }
    
    /* Create a simple linked structure */
    static struct Node nodes[5];
    for (int i = 0; i < 5; i++) {
        nodes[i].id = i * 10;
        for (int j = 0; j < 10; j++) {
            nodes[i].neighbors[j] = &nodes[(i + j) % 5];
            for (int k = 0; k < 10; k++) {
                nodes[i].weights[j][k] = (float)(i + j + k) / 100.0f;
            }
        }
    }
    
    node_matrix[0][0] = &nodes[0];
    node_matrix[10][10] = &nodes[2];
}

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize complex struct and union types */
    Container chain[3];
    for (int i = 0; i < 3; i++) {
        chain[i].inner.a = i * 100;
        chain[i].inner.b = (byte)(i + 65);
        chain[i].inner.bitfield1 = i;
        chain[i].inner.bitfield2 = i * 100;
        
        if (i % 2 == 0) {
            chain[i].data.x = i * 1000LL;
        } else {
            chain[i].data.y = i * 3.14;
        }
        
        chain[i].next = (i < 2) ? &chain[i + 1] : NULL;
    }
    
    /* 2. Process container chain */
    result += process_container(&chain[0]);
    
    /* 3. Use vector types (GCC extension) */
    use_vector_types();
    result += (int)vector_data.vectors[0][0];
    
    /* 4. Traverse pointer chain */
    result += traverse_pointer_chain();
    
    /* 5. Initialize and use node matrix */
    initialize_node_matrix();
    if (node_matrix[0][0]) {
        result += node_matrix[0][0]->id;
    }
    
    /* 6. Use union variants */
    for (int i = 0; i < 5; i++) {
        if (i % 3 == 0) {
            global_variants[i].as_int = i * 100;
            result += global_variants[i].as_int;
        } else if (i % 3 == 1) {
            global_variants[i].as_float = i * 1.5;
            result += (int)global_variants[i].as_float;
        } else {
            global_variants[i].tagged.tag = i;
            global_variants[i].tagged.value.i = i * 200;
            result += global_variants[i].tagged.value.i;
        }
    }
    
    /* 7. Call functions through struct function pointers */
    for (int i = 0; i < 3; i++) {
        if (plugin_registry[i].init) {
            result += plugin_registry[i].init(&result);
        }
        
        if (plugin_registry[i].process) {
            plugin_registry[i].process(i * 10);
        }
        
        if (plugin_registry[i].on_event) {
            plugin_registry[i].on_event(i, error_messages[i % 5], &result);
        }
    }
    
    /* 8. Use string types */
    const char* test_string = error_messages[result % 5];
    result += (int)strlen(test_string);
    
    /* 9. Use scalar types comprehensively */
    byte b = 0xFF;
    int16 s = -32768;
    int32 i = 2147483647;
    int64 l = 9223372036854775807LL;
    float32 f = 3.1415926535f;
    float64 d = 2.718281828459045;
    bool8 flag = 1;
    complex32 c1 = 1.0f + 2.0fi;
    complex64 c2 = 3.0 + 4.0i;
    
    result += b + s + (int)c1 + (int)c2;
    result += (int)f + (int)d + flag;
    result += (int)(l >> 32);  /* Use part of int64 */
    
    /* 10. Use array of function pointers */
    signal_processors[0][0] = (processor_func)plugin_init;
    if (signal_processors[0][0]) {
        result += signal_processors[0][0](&result, 1.0f, &chain[0]);
    }
    
    /* Return deterministic value based on all operations */
    return result % 256;  /* Ensure small, deterministic return value */
}
