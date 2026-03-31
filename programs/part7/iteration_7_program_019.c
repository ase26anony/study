/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
struct opaque;  /* Forward declaration - TYPE_UNDEFINED */
typedef struct opaque *opaque_ptr_t;

/* GCC-specific vector type - TYPE_LANG_STRUCT */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned and packed structs - TYPE_LANG_STRUCT extensions */
struct __attribute__((aligned(64))) cache_line {
    char data[64];
};

struct __attribute__((packed)) packed_data {
    uint16_t id;
    uint32_t value;
    uint8_t flags;
};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Complex nested structure with anonymous struct */
struct Container {
    struct {  /* Anonymous struct */
        int a;
        char b;
        unsigned bitfield : 4;
        unsigned : 4;  /* Unnamed bitfield */
    } inner;
    
    union {
        long x;
        double y;
        void *ptr;
    } data;
    
    volatile int counter;  /* Prevent optimization */
};

/* Structure with flexible array member */
struct dynamic_array {
    size_t length;
    size_t capacity;
    int elements[];
};

/* Structure with function pointer member */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void* context);
    void (*process)(int data, void* result);
    void (*cleanup)(struct Plugin* self);
};

/* ========== TYPE_UNION ========== */
/* Complex union with nested struct */
union Variant {
    int as_int;
    long as_long;
    double as_double;
    void* as_ptr;
    struct {
        short len;
        char buf[];  /* Flexible array member in union member */
    } as_string;
    
    struct {
        uint8_t type;
        uint8_t data[7];
    } as_packed;
};

/* Tagged union */
struct tagged_union {
    enum { INT, FLOAT, STRING, POINTER } tag;
    union {
        int i;
        float f;
        char *str;
        void *ptr;
    } value;
};

/* ========== TYPE_CALLBACK ========== */
/* Complex function pointer typedefs */
typedef void (*event_handler)(int event_id, void* user_data);
typedef int (*comparator_t)(const void*, const void*);
typedef void (*signal_processor)(float spectrum[][256], int** coefficients);

/* Callback with multiple parameters and return type */
typedef struct Result* (*complex_callback)(int a, float b, const char* c, void** d);

/* ========== TYPE_ARRAY / TYPE_POINTER ========== */
/* Multi-dimensional array of struct pointers */
struct Node* adjacency_matrix[10][10];

/* Array of function pointers */
event_handler event_handlers[5];

/* Pointer to array of pointers */
int** pointer_matrix;

/* Triple pointer */
int*** deep_indirection;

/* Array of unions */
union Variant variant_array[8];

/* ========== TYPE_SCALAR ========== */
/* Use all fundamental scalar types */
_Bool truth_value;
char character;
signed char s_char;
unsigned char u_char;
short short_int;
unsigned short u_short;
int integer;
unsigned int u_integer;
long long_int;
unsigned long u_long;
long long long_long;
unsigned long long u_long_long;
float single_float;
double double_float;
long double long_double;
_Complex float complex_float;
_Complex double complex_double;

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {"Error", "Warning", "Info", "Debug", NULL};
char* dynamic_strings[3];
const char static_string[] = "Static string literal";

/* ========== Global instances ========== */
struct Plugin plugin_registry[3];
struct Container global_container;
union Variant global_variant;
float32x8_t global_vector;

/* ========== Function implementations ========== */
int plugin_init_default(void* context) {
    static int initialized = 0;
    *(int*)context = ++initialized;
    return initialized;
}

void plugin_process_default(int data, void* result) {
    *(int*)result = data * 2;
}

void plugin_cleanup_default(struct Plugin* self) {
    if (self) {
        /* Simulate cleanup */
        self->version = 0;
    }
}

void handle_event(int event_id, void* user_data) {
    volatile int* counter = (int*)user_data;
    if (counter) {
        *counter += event_id;
    }
}

int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void process_signal(float spectrum[][256], int** coefficients) {
    /* Simple processing */
    for (int i = 0; i < 256; i++) {
        spectrum[0][i] = (float)i / 255.0f;
        if (coefficients && coefficients[0]) {
            coefficients[0][i] = i;
        }
    }
}

/* Function using GCC vector type */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;  /* Vector addition */
}

/* Function with complex pointer operations */
int*** create_deep_indirection(int depth, int size) {
    int*** ptr = NULL;
    if (depth > 0 && size > 0) {
        ptr = (int***)malloc(sizeof(int**) * size);
        if (ptr) {
            for (int i = 0; i < size; i++) {
                ptr[i] = (int**)malloc(sizeof(int*) * size);
                if (ptr[i]) {
                    for (int j = 0; j < size; j++) {
                        ptr[i][j] = (int*)malloc(sizeof(int) * size);
                        if (ptr[i][j]) {
                            ptr[i][j][0] = i * 100 + j * 10 + depth;
                        }
                    }
                }
            }
        }
    }
    return ptr;
}

/* Process union with type switching */
int process_variant(union Variant* v, int expected_type) {
    switch (expected_type) {
        case 0: return v->as_int;
        case 1: return (int)v->as_long;
        case 2: return (int)v->as_double;
        case 3: return v->as_ptr != NULL;
        default: return -1;
    }
}

/* Main function with comprehensive type usage */
int main(void) {
    int result = 0;
    volatile int counter = 0;  /* Prevent optimization */
    
    /* 1. Initialize structs and unions */
    global_container.inner.a = 42;
    global_container.inner.b = 'X';
    global_container.inner.bitfield = 7;
    global_container.data.x = 123456789L;
    global_container.counter = 0;
    
    global_variant.as_int = 65537;
    
    /* 2. Initialize plugin registry with function pointers */
    for (int i = 0; i < 3; i++) {
        plugin_registry[i].name = error_messages[i];
        plugin_registry[i].version = i + 1;
        plugin_registry[i].init = plugin_init_default;
        plugin_registry[i].process = plugin_process_default;
        plugin_registry[i].cleanup = plugin_cleanup_default;
        
        /* Call function through pointer */
        int context = 0;
        if (plugin_registry[i].init) {
            result += plugin_registry[i].init(&context);
        }
    }
    
    /* 3. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    global_vector = vector_add(vec_a, vec_b);
    
    /* Access vector elements */
    float* vec_ptr = (float*)&global_vector;
    for (int i = 0; i < 8; i++) {
        result += (int)vec_ptr[i];
    }
    
    /* 4. Complex pointer chains and arrays */
    pointer_matrix = (int**)malloc(sizeof(int*) * 5);
    if (pointer_matrix) {
        for (int i = 0; i < 5; i++) {
            pointer_matrix[i] = (int*)malloc(sizeof(int) * 5);
            if (pointer_matrix[i]) {
                for (int j = 0; j < 5; j++) {
                    pointer_matrix[i][j] = i * 10 + j;
                    result += pointer_matrix[i][j];
                }
            }
        }
    }
    
    /* Create deep indirection */
    deep_indirection = create_deep_indirection(3, 2);
    if (deep_indirection && deep_indirection[0] && deep_indirection[0][0]) {
        result += deep_indirection[0][0][0];
    }
    
    /* 5. Process union types */
    for (int i = 0; i < 8; i++) {
        variant_array[i].as_int = i * 100;
        result += process_variant(&variant_array[i], 0);
    }
    
    /* Switch union type */
    global_variant.as_double = 3.1415926535;
    result += (int)(global_variant.as_double * 100);
    
    /* 6. Use function pointer arrays */
    event_handlers[0] = handle_event;
    event_handlers[1] = handle_event;
    
    for (int i = 0; i < 2; i++) {
        if (event_handlers[i]) {
            event_handlers[i](i + 1, &counter);
        }
    }
    result += counter;
    
    /* 7. Multi-dimensional array simulation */
    int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    int* row_ptr = matrix[0];
    for (int i = 0; i < 9; i++) {
        result += row_ptr[i];
    }
    
    /* 8. String operations */
    dynamic_strings[0] = strdup("Dynamic string 1");
    dynamic_strings[1] = strdup("Dynamic string 2");
    
    for (int i = 0; error_messages[i] != NULL; i++) {
        result += (int)error_messages[i][0];  /* Add first character */
    }
    
    /* 9. Use all scalar types */
    truth_value = 1;
    character = 'A';
    integer = 1000;
    long_int = 1000000L;
    long_long = 1000000000LL;
    single_float = 1.5f;
    double_float = 3.14159;
    long_double = 2.718281828459045L;
    complex_float = 1.0f + 2.0fi;
    complex_double = 3.0 + 4.0i;
    
    result += truth_value;
    result += character;
    result += integer;
    result += (int)long_int;
    result += (int)long_long;
    result += (int)single_float;
    result += (int)double_float;
    result += (int)long_double;
    result += (int)(__real__ complex_float + __imag__ complex_float);
    
    /* 10. Use comparator function pointer */
    int values[] = {5, 2, 8, 1, 9};
    comparator_t comp = compare_ints;
    /* Simulate sorting by comparing elements */
    for (int i = 0; i < 4; i++) {
        result += comp(&values[i], &values[i + 1]);
    }
    
    /* 11. Signal processing with 2D array */
    float spectrum[4][256];
    int* coeffs[4];
    int coeff_data[4][256];
    
    for (int i = 0; i < 4; i++) {
        coeffs[i] = coeff_data[i];
    }
    
    signal_processor processor = process_signal;
    if (processor) {
        processor(spectrum, coeffs);
        result += (int)spectrum[0][255];
        result += coeff_data[0][255];
    }
    
    /* 12. Use packed and aligned structs */
    struct cache_line aligned_data;
    memset(aligned_data.data, 0xAA, sizeof(aligned_data.data));
    
    struct packed_data packed = {123, 456789, 1};
    result += packed.id + packed.value + packed.flags;
    
    /* Cleanup */
    for (int i = 0; i < 3; i++) {
        if (plugin_registry[i].cleanup) {
            plugin_registry[i].cleanup(&plugin_registry[i]);
        }
    }
    
    /* Free allocated memory */
    if (pointer_matrix) {
        for (int i = 0; i < 5; i++) {
            free(pointer_matrix[i]);
        }
        free(pointer_matrix);
    }
    
    if (deep_indirection) {
        for (int i = 0; i < 2; i++) {
            if (deep_indirection[i]) {
                for (int j = 0; j < 2; j++) {
                    free(deep_indirection[i][j]);
                }
                free(deep_indirection[i]);
            }
        }
        free(deep_indirection);
    }
    
    for (int i = 0; i < 2; i++) {
        free(dynamic_strings[i]);
    }
    
    /* Return deterministic result based on all operations */
    return result % 256;  /* Ensure result fits in return value */
}
