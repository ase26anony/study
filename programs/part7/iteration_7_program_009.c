/* test_rich_types.c - Comprehensive type coverage for GCC gengtype-state.cc */

#include <stddef.h>
#include <string.h>

/* ========== 1. USER-DEFINED STRUCTURES AND UNIONS ========== */

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* Complex nested struct with bit-fields (TYPE_STRUCT) */
struct Container {
    struct {
        int a;
        char b;
        unsigned int flags : 4;
        unsigned int mode : 3;
    } inner;
    union {
        long x;
        double y;
        void* ptr;
    } data;
    volatile int counter;
};

/* Union with flexible array member (TYPE_UNION) */
union Variant {
    int as_int;
    void* as_ptr;
    struct {
        short len;
        char buf[];
    } as_string;
    _Complex double as_complex;
};

/* Packed struct with GCC attributes (TYPE_USER_STRUCT) */
struct __attribute__((packed, aligned(8))) PackedData {
    char id;
    int value;
    short checksum;
};

/* Anonymous struct/union */
struct AnonymousExample {
    struct {
        int x, y;
    };
    union {
        float f;
        int i;
    } coord;
};

/* ========== 2. FUNCTION POINTERS AND CALLBACKS ========== */

/* Function pointer typedefs (TYPE_CALLBACK) */
typedef void (*event_handler)(int, void*);
typedef int (*processor_func)(const char*, size_t);
typedef union Variant* (*variant_factory)(int);

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    event_handler on_event;
    variant_factory create_variant;
};

/* Complex function pointer */
int (*(*complex_callback)(float[][256], int**))(void);

/* ========== 3. ARRAYS AND POINTER CHAINS ========== */

/* Multi-dimensional array of struct pointers (TYPE_ARRAY, TYPE_POINTER) */
struct Node* adjacency_matrix[10][10];

/* Array of function pointers */
processor_func processors[5];

/* Triple pointer chain */
int ***triple_ptr_chain;

/* Pointer to array */
int (*array_ptr)[20];

/* ========== 4. LANGUAGE-SPECIFIC TYPES ========== */

/* GCC vector type (TYPE_LANG_STRUCT) */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
};

/* ========== 5. SCALAR AND STRING TYPES ========== */

/* All scalar types (TYPE_SCALAR) */
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
_Complex float cfloat_var;
_Complex double cdouble_var;
_Complex long double cldouble_var;

/* String types (TYPE_STRING) */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char static_string[] = "Static string literal";
const char* const const_string_ptr = "Constant pointer to string";

/* ========== 6. GLOBAL VARIABLES WITH ACTIVE USAGE ========== */

/* Global instances */
struct Container global_container = {
    .inner = {42, 'X', 3, 5},
    .data = {.y = 3.14159},
    .counter = 0
};

union Variant global_variants[3];

struct Plugin plugin_registry[3];

float32x8_t global_vector1, global_vector2;

/* ========== FUNCTION DEFINITIONS ========== */

/* Callback function implementations */
static void sample_event_handler(int event, void* data) {
    *(int*)data = event * 2;
}

static int sample_init(void) {
    return 0;
}

static void sample_process(int value) {
    global_container.counter += value;
}

static union Variant* create_int_variant(int value) {
    static union Variant v;
    v.as_int = value;
    return &v;
}

/* Function using vector types */
static void vector_operations(void) {
    float32x8_t a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    global_vector1 = a + b;
    global_vector2 = a * b;
}

/* Function using pointer chains */
static int process_pointer_chain(void) {
    int value = 100;
    int *p1 = &value;
    int **p2 = &p1;
    triple_ptr_chain = &p2;
    
    return ***triple_ptr_chain;
}

/* Function using multi-dimensional array */
static void init_adjacency_matrix(void) {
    static struct Node dummy_nodes[10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            adjacency_matrix[i][j] = &dummy_nodes[(i + j) % 10];
        }
    }
}

/* Function processing union types */
static int process_variant(union Variant *v, int type) {
    switch (type) {
        case 0:
            return v->as_int * 2;
        case 1:
            return (int)(long)v->as_ptr;
        case 2:
            return v->as_string.len;
        default:
            return 0;
    }
}

/* Main function with comprehensive type usage */
int main(void) {
    int result = 0;
    
    /* 1. Initialize struct and union variables */
    struct Container local_container = {
        .inner = {100, 'A', 7, 2},
        .data = {.x = 123456789L},
        .counter = 1
    };
    
    union Variant local_variant;
    local_variant.as_int = 42;
    
    /* 2. Populate and use struct with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .init = sample_init,
        .process = sample_process,
        .on_event = sample_event_handler,
        .create_variant = create_int_variant
    };
    
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    int event_data = 0;
    if (plugin_registry[0].on_event) {
        plugin_registry[0].on_event(21, &event_data);
        result += event_data;
    }
    
    /* 3. Use vector types */
    vector_operations();
    
    /* Use vector elements to affect result */
    float* vec_ptr = (float*)&global_vector1;
    for (int i = 0; i < 8; i++) {
        result += (int)vec_ptr[i];
    }
    
    /* 4. Use pointer chains */
    result += process_pointer_chain();
    
    /* 5. Initialize and use multi-dimensional array */
    init_adjacency_matrix();
    
    /* 6. Process union types */
    global_variants[0].as_int = 100;
    global_variants[1].as_ptr = &result;
    
    for (int i = 0; i < 2; i++) {
        result += process_variant(&global_variants[i], i);
    }
    
    /* 7. Use all scalar types */
    char_var = 'Z';
    schar_var = -10;
    uchar_var = 200;
    short_var = -1000;
    ushort_var = 5000;
    int_var = -50000;
    uint_var = 100000;
    long_var = -1000000L;
    ulong_var = 2000000UL;
    llong_var = -5000000000LL;
    ullong_var = 10000000000ULL;
    float_var = 3.14f;
    double_var = 2.71828;
    ldouble_var = 1.41421356L;
    bool_var = 1;
    cfloat_var = 1.0f + 2.0fi;
    cdouble_var = 3.0 + 4.0i;
    
    /* Use scalars in computation */
    result += char_var + schar_var + uchar_var;
    result += short_var + ushort_var;
    result += int_var + (int)uint_var;
    result += (int)long_var + (int)ulong_var;
    result += (int)llong_var + (int)ullong_var;
    result += (int)float_var + (int)double_var + (int)ldouble_var;
    result += bool_var;
    
    /* 8. Use string types */
    for (int i = 0; error_messages[i] != NULL; i++) {
        result += (int)error_messages[i][0]; /* Use first char */
    }
    result += (int)static_string[0];
    result += (int)const_string_ptr[0];
    
    /* 9. Use packed struct */
    struct PackedData packed = {.id = 'P', .value = 999, .checksum = 123};
    result += packed.value;
    
    /* 10. Use anonymous struct */
    struct AnonymousExample anon = {.x = 10, .y = 20, .coord.f = 30.5f};
    result += anon.x + anon.y + (int)anon.coord.f;
    
    /* 11. Use array pointer */
    int local_array[20] = {0};
    array_ptr = &local_array;
    for (int i = 0; i < 20; i++) {
        (*array_ptr)[i] = i;
        result += (*array_ptr)[i];
    }
    
    /* 12. Use complex function pointer */
    int simple_func(void) { return 42; }
    int (*func_ptr)(void) = simple_func;
    int (**func_ptr_ptr)(void) = &func_ptr;
    complex_callback = (int (*(*)(float[][256], int**))(void))func_ptr_ptr;
    
    /* 13. Reference undefined type */
    struct opaque* opaque_ptr = NULL;
    (void)opaque_ptr; /* Suppress unused warning */
    
    /* 14. Use aligned struct */
    struct CacheLine cache_line;
    memset(cache_line.data, 0xAA, sizeof(cache_line.data));
    result += (int)cache_line.data[0];
    
    /* Return deterministic result */
    return result % 256; /* Ensure small, deterministic output */
}

/* Additional type definitions for linkage */
struct Node {
    int id;
    struct Node* next;
};

/* Global definition */
struct Node dummy_nodes[10];
