/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;  /* Forward declaration - undefined type */
typedef struct incomplete incomplete_t;  /* Another undefined type */

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Complex nested structure with bit-fields */
struct Inner {
    int a;
    char b;
    unsigned int flag1:1;
    unsigned int flag2:3;
    unsigned int :4;  /* Padding bit-field */
};

/* User-defined structure with flexible array member */
struct DynamicArray {
    size_t capacity;
    size_t length;
    int data[];
};

/* Main complex structure */
struct Container {
    struct {
        int id;
        char tag;
        struct Inner nested;
    } metadata;
    
    union {
        long as_long;
        double as_double;
        void* as_ptr;
    } value;
    
    struct Container* next;  /* Self-referential pointer */
    volatile int ref_count;  /* Volatile to prevent optimization */
};

/* Packed structure with attributes */
struct __attribute__((packed, aligned(2))) PackedStruct {
    char a;
    int b;
    short c;
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    long as_long;
    double as_double;
    void* as_ptr;
    struct {
        short len;
        char buf[];  /* Flexible array member in union */
    } as_string;
    
    struct {
        float x, y, z;
    } as_vector;
};

/* Anonymous union within struct */
struct WithAnonymousUnion {
    int type;
    union {
        int i;
        float f;
        char* s;
    };  /* Anonymous union */
};

/* ========== TYPE_CALLBACK / Function Pointers ========== */
/* Callback typedefs */
typedef void (*simple_callback)(void);
typedef int (*processor_func)(int, const char*);
typedef void (*event_handler)(int, void* user_data);
typedef union Variant* (*transformer)(union Variant*, processor_func);

/* Structure containing function pointers */
struct Plugin {
    const char* name;
    int version;
    int (*init)(struct Plugin*);
    void (*process)(int*, size_t);
    void (*cleanup)(void);
    event_handler on_event;
};

/* Another callback structure */
struct SignalProcessor {
    float (*filter)(float*, size_t);
    int (*transform)(float complex*, int);
    transformer variant_transform;
};

/* ========== TYPE_LANG_STRUCT / GCC Extensions ========== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned types */
struct __attribute__((aligned(64))) CacheAligned {
    int data[16];
    float32x8_t vector;
};

/* Transparent union attribute */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int i;
    long l;
} transparent_t;

/* ========== TYPE_ARRAY / Multi-dimensional Arrays ========== */
/* Complex array types */
struct Node* adjacency_matrix[10][10];  /* 2D array of pointers */
int (*signal_table[5])(float[][256], int**);  /* Array of function pointers */

/* Array of structs with flexible array members */
struct DynamicArray* dynamic_arrays[8];

/* Multi-dimensional array with different types */
union Variant variant_grid[5][5][3];

/* ========== TYPE_POINTER / Pointer Chains ========== */
int ****quad_ptr;  /* Four-level pointer */
struct Container*** container_ptr_ptr;
float (*matrix_ptr)[256][256];  /* Pointer to 2D array */

/* ========== TYPE_SCALAR / Basic Types ========== */
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
char* dynamic_strings[4];
const char* const fixed_string = "Fixed string literal";

/* ========== Global Variables ========== */
struct Plugin plugin_registry[3];
struct Container global_container;
union Variant global_variant;
float32x8_t global_vector;
int32x4_t global_int_vector;

/* ========== Function Implementations ========== */
/* Callback implementations */
static int plugin_init(struct Plugin* p) {
    printf("Initializing plugin: %s\n", p->name);
    return p->version;
}

static void plugin_process(int* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data[i] *= 2;
    }
}

static void plugin_cleanup(void) {
    printf("Plugin cleanup\n");
}

static void handle_event(int event_id, void* user_data) {
    struct Container* c = (struct Container*)user_data;
    if (c) {
        c->ref_count++;
    }
    printf("Event %d handled\n", event_id);
}

/* Processor function implementation */
static int string_processor(int count, const char* str) {
    return count + (str ? strlen(str) : 0);
}

/* Transformer implementation */
static union Variant* variant_transformer(union Variant* v, processor_func pf) {
    static union Variant result;
    if (v && pf) {
        result.as_int = pf(42, "test");
    }
    return &result;
}

/* Vector operations */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Array traversal */
static int traverse_matrix(struct Node* matrix[10][10]) {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (matrix[i][j] != NULL) {
                count++;
            }
        }
    }
    return count;
}

/* Union processing */
static void process_variant(union Variant* v, int type) {
    switch (type) {
        case 0:
            v->as_int = 42;
            break;
        case 1:
            v->as_double = 3.14159;
            break;
        case 2:
            v->as_ptr = &global_container;
            break;
        default:
            v->as_vector.x = 1.0f;
            v->as_vector.y = 2.0f;
            v->as_vector.z = 3.0f;
            break;
    }
}

/* Pointer chain manipulation */
static void setup_pointer_chain(void) {
    int ****q = malloc(sizeof(int***));
    int ***r = malloc(sizeof(int**));
    int **s = malloc(sizeof(int*));
    int *t = malloc(sizeof(int));
    
    *t = 999;
    *s = t;
    *r = s;
    *q = r;
    
    quad_ptr = q;
}

/* ========== Main Function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize struct and union types */
    struct Container local_container = {
        .metadata = {1, 'A', {10, 'B', 1, 3}},
        .value = {.as_long = 1000},
        .next = NULL,
        .ref_count = 1
    };
    
    union Variant local_variant;
    process_variant(&local_variant, 0);
    
    /* 2. Initialize plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .version = 1,
        .init = plugin_init,
        .process = plugin_process,
        .cleanup = plugin_cleanup,
        .on_event = handle_event
    };
    
    /* Call function through pointer */
    int init_result = plugin_registry[0].init(&plugin_registry[0]);
    result += init_result;
    
    /* 3. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = vector_add(vec_a, vec_b);
    
    /* Extract value from vector */
    float vec_element;
    memcpy(&vec_element, &vec_sum, sizeof(float));
    result += (int)vec_element;
    
    /* 4. Setup and traverse multi-dimensional array */
    struct Node* test_matrix[10][10] = {0};
    /* Simulate some non-NULL entries */
    for (int i = 0; i < 5; i++) {
        test_matrix[i][i] = (struct Node*)0x1;  /* Dummy non-NULL pointer */
    }
    
    int node_count = traverse_matrix(test_matrix);
    result += node_count;
    
    /* 5. Setup pointer chain */
    setup_pointer_chain();
    if (quad_ptr && *quad_ptr && **quad_ptr && ***quad_ptr) {
        result += ***quad_ptr;
    }
    
    /* 6. Process union with different types */
    for (int i = 0; i < 4; i++) {
        process_variant(&local_variant, i);
        result += local_variant.as_int;
    }
    
    /* 7. Use all scalar types */
    char_var = 'Z';
    schar_var = -128;
    uchar_var = 255;
    short_var = -32768;
    ushort_var = 65535;
    int_var = -2147483647;
    uint_var = 4294967295U;
    long_var = -2147483647L;
    ulong_var = 4294967295UL;
    llong_var = -9223372036854775807LL;
    ullong_var = 18446744073709551615ULL;
    float_var = 3.14159f;
    double_var = 2.718281828459045;
    ldouble_var = 1.618033988749895L;
    bool_var = 1;
    complex_float = 1.0f + 2.0f * I;
    complex_double = 3.0 + 4.0 * I;
    
    result += char_var + schar_var + uchar_var + short_var + int_var + bool_var;
    
    /* 8. Use string types */
    dynamic_strings[0] = strdup("Dynamic string 1");
    dynamic_strings[1] = strdup("Dynamic string 2");
    
    for (int i = 0; i < 5; i++) {
        result += error_messages[i][0];  /* Add first character of each string */
    }
    
    /* 9. Use anonymous union */
    struct WithAnonymousUnion anon_union = {0};
    anon_union.type = 0;
    anon_union.i = 42;
    result += anon_union.i;
    
    /* 10. Use packed structure */
    struct PackedStruct packed = {'X', 1234, 5678};
    result += packed.a + packed.b + packed.c;
    
    /* 11. Setup signal processor with callbacks */
    struct SignalProcessor processor = {
        .filter = NULL,  /* Would be a real function in practice */
        .transform = NULL,
        .variant_transform = variant_transformer
    };
    
    /* 12. Use transparent union */
    transparent_t tu;
    tu.i = 100;
    result += tu.i;
    
    /* 13. Cleanup */
    for (int i = 0; i < 2; i++) {
        free(dynamic_strings[i]);
    }
    
    /* Cleanup pointer chain */
    if (quad_ptr) {
        if (*quad_ptr) {
            if (**quad_ptr) {
                if (***quad_ptr) {
                    free(***quad_ptr);
                }
                free(**quad_ptr);
            }
            free(*quad_ptr);
        }
        free(quad_ptr);
    }
    
    printf("Final result: %d\n", result);
    return result % 256;  /* Return deterministic value */
}

/* Dummy Node structure for adjacency matrix */
struct Node {
    int id;
    struct Node** neighbors;
    int neighbor_count;
};

/* Define previously opaque types */
struct opaque {
    int hidden_data;
    void* secret_ptr;
};

struct incomplete {
    int defined_now;
    struct opaque* link;
};
