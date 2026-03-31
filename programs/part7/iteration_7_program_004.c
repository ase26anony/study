/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
/* This program defines a diverse set of type constructs to exercise all */
/* type categories in GCC's GGC type state serialization. */

#include <stddef.h>
#include <string.h>

/* ==================== TYPE_UNDEFINED / TYPE_LANG_STRUCT ==================== */

/* Forward declaration for undefined/incomplete type */
struct opaque;                     /* TYPE_UNDEFINED candidate */

/* GCC-specific language extensions for TYPE_LANG_STRUCT */
typedef float __attribute__((vector_size(32))) float32x8_t;  /* SIMD vector */
typedef int __attribute__((mode(TI))) int128_t;              /* 128-bit integer */

/* Aligned and packed structs with GCC attributes */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
};

struct __attribute__((packed)) PackedData {
    char a;
    int b;
    short c;
};

/* ==================== TYPE_STRUCT / TYPE_USER_STRUCT ==================== */

/* Simple nested anonymous struct */
struct Container {
    struct {                    /* Anonymous struct */
        int a;
        char b;
        unsigned bitfield : 3;  /* Bit-field */
    } inner;
    volatile long counter;      /* Volatile to prevent optimization */
};

/* Complex user-defined struct with flexible array member */
struct DynamicString {
    size_t length;
    char data[];                /* Flexible array member */
};

/* Struct with function pointer member (for TYPE_CALLBACK) */
struct Plugin {
    const char* name;
    int (*init)(void*);         /* Function pointer */
    void (*process)(int, float);
    void (*cleanup)(struct Plugin*);
};

/* Struct containing array of structs */
struct Node {
    int id;
    struct Node* neighbors[8];  /* Array of pointers */
};

/* ==================== TYPE_UNION ==================== */

/* Tagged union with anonymous union */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    struct {                    /* Anonymous struct inside union */
        short len;
        char buf[];             /* Flexible array in union member */
    } as_string;
};

/* Union of different scalar types */
union ScalarUnion {
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    _Bool b;
};

/* ==================== TYPE_POINTER ==================== */

/* Complex pointer chains */
typedef int**** QuadPointer;    /* Four-level indirection */

/* Pointer to array */
typedef int (*ArrayPointer)[10];

/* Pointer to function returning pointer to struct */
struct Plugin* (*plugin_factory)(const char*);

/* ==================== TYPE_ARRAY ==================== */

/* Multi-dimensional arrays */
struct Node* adjacency_matrix[10][10];  /* 2D array of pointers */

/* Array of unions */
union Variant variant_array[20];

/* Array of function pointers */
int (*callbacks[5])(void);

/* ==================== TYPE_SCALAR ==================== */

/* Use all fundamental scalar types */
char char_var;
short short_var;
int int_var;
long long_var;
long long long_long_var;
float float_var;
double double_var;
_Bool bool_var;

/* Complex numbers */
_Complex float complex_float;
_Complex double complex_double;

/* ==================== TYPE_STRING ==================== */

/* String literals and arrays */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char static_string[] = "Static string data";

/* ==================== TYPE_CALLBACK ==================== */

/* Typedef for complex function signature */
typedef void (*event_handler)(int, void*, const char*);

/* Struct with multiple callback types */
struct EventSystem {
    event_handler handlers[10];
    void (*error_callback)(int, const char*, ...);  /* Variadic function pointer */
};

/* ==================== FUNCTION DEFINITIONS ==================== */

/* Callback function implementations */
int plugin_init_default(void* data) {
    static int initialized = 0;
    initialized = !initialized;
    return initialized;
}

void plugin_process_default(int x, float y) {
    volatile float result = x * y;  /* Prevent optimization */
    (void)result;
}

void plugin_cleanup_default(struct Plugin* p) {
    if (p) p->name = NULL;
}

/* Function using vector type */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;  /* GCC vector addition */
}

/* Function with complex pointer chain */
int deref_quad_pointer(QuadPointer qp) {
    if (qp && *qp && **qp && ***qp) {
        return ****qp;
    }
    return 0;
}

/* Function processing union type */
int process_variant(union Variant* v, int type) {
    switch (type) {
        case 0: return v->as_int;
        case 1: return (int)(long)v->as_ptr;
        case 2: return (int)v->as_float;
        default: return -1;
    }
}

/* Function using multi-dimensional array */
void init_adjacency_matrix(void) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            adjacency_matrix[i][j] = NULL;
        }
    }
}

/* Event handler callback */
void sample_event_handler(int id, void* data, const char* msg) {
    volatile int x = id + (int)(long)data;
    (void)x;
    (void)msg;
}

/* ==================== GLOBAL VARIABLES ==================== */

/* Ensure all types are used globally */
struct Container global_container = {{1, 'A', 5}, 1000L};
union ScalarUnion global_scalar_union = {.d = 3.14159};
struct Plugin global_plugins[3];
struct EventSystem global_event_system;
float32x8_t global_vector1, global_vector2;

/* Opaque pointer (undefined type) */
struct opaque* opaque_ptr = NULL;

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    int result = 0;
    
    /* 1. Initialize struct with function pointers */
    for (int i = 0; i < 3; i++) {
        global_plugins[i].name = error_messages[i];
        global_plugins[i].init = plugin_init_default;
        global_plugins[i].process = plugin_process_default;
        global_plugins[i].cleanup = plugin_cleanup_default;
    }
    
    /* 2. Call function through pointer */
    if (global_plugins[0].init != NULL) {
        result += global_plugins[0].init(&global_container);
    }
    
    /* 3. Use GCC vector type */
    global_vector1 = (float32x8_t){1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    global_vector2 = (float32x8_t){8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vector_sum = vector_add(global_vector1, global_vector2);
    
    /* Extract element from vector for result */
    float vector_elements[8];
    memcpy(vector_elements, &vector_sum, sizeof(vector_sum));
    result += (int)vector_elements[0];
    
    /* 4. Use multi-dimensional array */
    init_adjacency_matrix();
    result += (adjacency_matrix[0][0] == NULL);
    
    /* 5. Process union type */
    union Variant local_variant;
    local_variant.as_int = 42;
    result += process_variant(&local_variant, 0);
    
    local_variant.as_float = 3.14f;
    result += (int)process_variant(&local_variant, 2);
    
    /* 6. Complex pointer chain */
    int value = 100;
    int* p1 = &value;
    int** p2 = &p1;
    int*** p3 = &p2;
    int**** p4 = &p3;
    
    result += deref_quad_pointer(p4);
    
    /* 7. Use all scalar types */
    char_var = 'Z';
    short_var = 1000;
    int_var = -42;
    long_var = 100000L;
    long_long_var = 0xFFFFFFFFFFFFFFFFLL;
    float_var = 2.71828f;
    double_var = 1.41421356;
    bool_var = 1;
    
    result += char_var + short_var + int_var + (int)long_var + 
              (int)long_long_var + (int)float_var + (int)double_var + bool_var;
    
    /* 8. Use complex numbers */
    complex_float = 1.0f + 2.0fi;
    complex_double = 3.0 + 4.0i;
    
    /* 9. String operations */
    result += (int)strlen(static_string);
    result += (int)strlen(error_messages[0]);
    
    /* 10. Initialize event system with callback */
    global_event_system.handlers[0] = sample_event_handler;
    if (global_event_system.handlers[0] != NULL) {
        global_event_system.handlers[0](1, &result, "test");
    }
    
    /* 11. Use packed and aligned structs */
    struct CacheLine cache_line;
    memset(cache_line.data, 0xAA, sizeof(cache_line.data));
    result += cache_line.data[0];
    
    struct PackedData packed;
    packed.a = 'X';
    packed.b = 999;
    packed.c = -1;
    result += packed.a + packed.b + packed.c;
    
    /* 12. Use 128-bit integer type */
    int128_t big_int = (int128_t)0x123456789ABCDEF0LL << 64;
    big_int |= 0xFEDCBA9876543210LL;
    result += (int)big_int;  /* Truncate, but ensures usage */
    
    /* Return deterministic result based on all operations */
    return result % 256;  /* Ensure result fits in return value */
}
