/* test_rich_types.c - Comprehensive type coverage for GCC gengtype-state.cc */

#include <stdio.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declaration for undefined type */
struct opaque;  /* TYPE_UNDEFINED candidate */

/* GCC-specific vector type (TYPE_LANG_STRUCT) */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Complex nested structure with bitfields */
struct Inner {
    int a;
    char b;
    unsigned int flag : 3;
    unsigned int : 5;  /* padding */
};

/* Packed structure with GCC attributes */
struct __attribute__((packed, aligned(8))) PackedData {
    uint16_t id;
    uint8_t version;
    uint8_t checksum;
    struct Inner inner;
};

/* Structure with flexible array member */
struct DynamicBuffer {
    size_t length;
    char data[];
};

/* Main complex structure */
struct Container {
    struct {
        int counter;
        char tag;
    } anonymous_inner;  /* anonymous struct */
    
    union {
        long as_long;
        double as_double;
        void *as_ptr;
    } data_union;
    
    struct Inner inner_struct;
    struct PackedData *packed_ref;
};

/* User-defined structure with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    void (*cleanup)(struct Plugin*);
};

/* ========== TYPE_UNION ========== */
/* Complex union with nested struct */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    double as_double;
    struct {
        short len;
        char buf[32];
    } as_string;
    struct {
        int type;
        union {
            int i;
            float f;
        } value;
    } as_tagged;
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*event_handler)(int, void*);
typedef int (*comparator_fn)(const void*, const void*);
typedef union Variant* (*transformer_fn)(union Variant*, int);

/* Structure containing multiple callback types */
struct EventSystem {
    event_handler handlers[10];
    void* user_data[10];
    int (*filter)(int, event_handler);
};

/* ========== TYPE_ARRAY / TYPE_POINTER ========== */
/* Multi-dimensional array of pointers */
struct Node* adjacency_matrix[10][10];

/* Array of function pointers */
transformer_fn transformers[5];

/* Complex pointer chain */
int ***triple_ptr_chain;

/* Array of complex structures */
struct Container container_array[8];
union Variant variant_pool[20];

/* ========== TYPE_SCALAR ========== */
/* Use all fundamental scalar types */
char char_var;
signed char schar_var;
unsigned char uchar_var;
short short_var;
unsigned short ushort_var;
int int_var;
unsigned int uint_var;
long long_var;
unsigned long ulong_var;
long long llong_var;
unsigned long long ullong_var;
float float_var;
double double_var;
long double ldouble_var;
_Bool bool_var;
float _Complex complex_float;
double _Complex complex_double;

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {"Error", "Warning", "Info", "Debug", "Trace"};
char* dynamic_strings[5];
const char static_strings[][20] = {"Hello", "World", "GCC", "Coverage"};

/* ========== GLOBAL VARIABLES ========== */
/* Ensure type visibility with global instances */
struct Plugin plugin_registry[3];
struct EventSystem global_event_system;
volatile int volatile_counter = 0;  /* Prevent optimization */

/* ========== FUNCTION DEFINITIONS ========== */
/* Callback function implementations */
int plugin_init_default(void) {
    volatile_counter++;
    return 0;
}

void plugin_process_default(int x) {
    volatile_counter += x;
}

void plugin_cleanup_default(struct Plugin* p) {
    if (p) volatile_counter--;
}

void event_handler_example(int event, void* data) {
    union Variant* v = (union Variant*)data;
    if (v) {
        v->as_int = event;
    }
    volatile_counter += event;
}

int filter_positive(int val, event_handler handler) {
    return val > 0;
}

union Variant* transform_increment(union Variant* v, int amount) {
    if (v) {
        v->as_int += amount;
    }
    return v;
}

/* Function using GCC vector types */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex parameter types */
void process_container(struct Container* c, 
                      struct Plugin* plugins, 
                      int count,
                      event_handler callback) {
    if (!c || !plugins) return;
    
    /* Access nested structure */
    c->anonymous_inner.counter++;
    
    /* Use union */
    if (volatile_counter % 2) {
        c->data_union.as_long = 100;
    } else {
        c->data_union.as_double = 3.14159;
    }
    
    /* Call through function pointer */
    for (int i = 0; i < count && i < 3; i++) {
        if (plugins[i].init) {
            plugins[i].init();
        }
    }
    
    /* Invoke callback */
    if (callback) {
        callback(c->anonymous_inner.counter, &c->data_union);
    }
}

/* Function using multi-dimensional arrays */
void init_adjacency_matrix(void) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Simulate pointer assignment */
            adjacency_matrix[i][j] = (struct Node*)(intptr_t)(i * 10 + j);
        }
    }
}

/* Function processing pointer chains */
int process_pointer_chain(int ***ptr) {
    if (!ptr) return 0;
    
    int sum = 0;
    /* Simulate complex pointer traversal */
    for (int i = 0; i < 3; i++) {
        if (ptr[i]) {
            for (int j = 0; j < 3; j++) {
                if (ptr[i][j]) {
                    sum += *(ptr[i][j]);
                }
            }
        }
    }
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize plugin registry with function pointers */
    for (int i = 0; i < 3; i++) {
        plugin_registry[i].name = error_messages[i];
        plugin_registry[i].init = plugin_init_default;
        plugin_registry[i].process = plugin_process_default;
        plugin_registry[i].cleanup = plugin_cleanup_default;
    }
    
    /* 2. Initialize event system */
    for (int i = 0; i < 10; i++) {
        global_event_system.handlers[i] = event_handler_example;
        global_event_system.user_data[i] = &variant_pool[i];
    }
    global_event_system.filter = filter_positive;
    
    /* 3. Initialize transformers array */
    for (int i = 0; i < 5; i++) {
        transformers[i] = transform_increment;
    }
    
    /* 4. Initialize container array */
    for (int i = 0; i < 8; i++) {
        container_array[i].anonymous_inner.counter = i;
        container_array[i].anonymous_inner.tag = 'A' + i;
        container_array[i].inner_struct.a = i * 10;
        container_array[i].inner_struct.b = 'a' + i;
        
        if (i % 3 == 0) {
            container_array[i].data_union.as_long = i * 100;
        } else if (i % 3 == 1) {
            container_array[i].data_union.as_double = i * 1.5;
        } else {
            container_array[i].data_union.as_ptr = &container_array[(i + 1) % 8];
        }
    }
    
    /* 5. Initialize variant pool */
    for (int i = 0; i < 20; i++) {
        if (i % 4 == 0) {
            variant_pool[i].as_int = i;
        } else if (i % 4 == 1) {
            variant_pool[i].as_float = i * 0.5f;
        } else if (i % 4 == 2) {
            variant_pool[i].as_string.len = i % 32;
            for (int j = 0; j < variant_pool[i].as_string.len && j < 31; j++) {
                variant_pool[i].as_string.buf[j] = 'a' + j;
            }
            variant_pool[i].as_string.buf[variant_pool[i].as_string.len] = '\0';
        } else {
            variant_pool[i].as_tagged.type = i % 3;
            if (variant_pool[i].as_tagged.type == 0) {
                variant_pool[i].as_tagged.value.i = i * 2;
            } else {
                variant_pool[i].as_tagged.value.f = i * 0.25f;
            }
        }
    }
    
    /* 6. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_c = vector_add(vec_a, vec_b);
    
    /* Extract result from vector */
    float vec_result[8];
    __builtin_memcpy(vec_result, &vec_c, sizeof(vec_c));
    for (int i = 0; i < 8; i++) {
        result += (int)vec_result[i];
    }
    
    /* 7. Process containers with callbacks */
    for (int i = 0; i < 8; i += 2) {
        process_container(&container_array[i], 
                         plugin_registry, 
                         3,
                         global_event_system.handlers[i % 10]);
    }
    
    /* 8. Call functions through plugin registry */
    for (int i = 0; i < 3; i++) {
        if (plugin_registry[i].init) {
            result += plugin_registry[i].init();
        }
        if (plugin_registry[i].process) {
            plugin_registry[i].process(i * 10);
        }
    }
    
    /* 9. Process variants through transformers */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (transformers[i]) {
                union Variant* v = transformers[i](&variant_pool[j], i + j);
                if (v && (j % 4 == 0)) {
                    result += v->as_int;
                }
            }
        }
    }
    
    /* 10. Initialize and use multi-dimensional array */
    init_adjacency_matrix();
    
    /* 11. Create and process pointer chain */
    int val1 = 10, val2 = 20, val3 = 30;
    int *p1 = &val1, *p2 = &val2, *p3 = &val3;
    int **arr1[3] = {&p1, &p2, &p3};
    triple_ptr_chain = arr1;
    result += process_pointer_chain(triple_ptr_chain);
    
    /* 12. Use all scalar types */
    char_var = 'Z';
    schar_var = -128;
    uchar_var = 255;
    short_var = -32768;
    ushort_var = 65535;
    int_var = -2147483647 - 1;
    uint_var = 4294967295U;
    long_var = -2147483647L - 1;
    ulong_var = 4294967295UL;
    llong_var = -9223372036854775807LL - 1;
    ullong_var = 18446744073709551615ULL;
    float_var = 3.14159f;
    double_var = 2.718281828459045;
    ldouble_var = 1.618033988749895L;
    bool_var = 1;
    complex_float = 1.0f + 2.0f * I;
    complex_double = 3.0 + 4.0 * I;
    
    /* Add scalar values to result */
    result += char_var + schar_var + uchar_var + short_var + int_var;
    result += (int)float_var + (int)double_var + bool_var;
    
    /* 13. Use string types */
    for (int i = 0; i < 5; i++) {
        dynamic_strings[i] = (char*)&variant_pool[i].as_string.buf[0];
        result += error_messages[i][0];  /* Add first character of each string */
    }
    
    /* Final deterministic result */
    printf("Result: %d\n", result);
    return result % 256;
}
