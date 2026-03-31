/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward declarations ========== */
struct opaque;  /* Incomplete type - TYPE_UNDEFINED */
struct forward_declared;

/* ========== TYPE_SCALAR - All fundamental types ========== */
volatile char v_char = 'A';
volatile signed char v_schar = -1;
volatile unsigned char v_uchar = 255;
volatile short v_short = -32768;
volatile unsigned short v_ushort = 65535;
volatile int v_int = -2147483648;
volatile unsigned int v_uint = 4294967295U;
volatile long v_long = -9223372036854775807L;
volatile unsigned long v_ulong = 18446744073709551615UL;
volatile long long v_llong = -9223372036854775807LL;
volatile unsigned long long v_ullong = 18446744073709551615ULL;
volatile float v_float = 3.14159f;
volatile double v_double = 2.718281828459045;
volatile long double v_ldouble = 1.618033988749895L;
volatile _Bool v_bool = 1;
volatile _Complex float v_cfloat = 1.0f + 2.0fi;
volatile _Complex double v_cdouble = 3.0 + 4.0i;
volatile _Complex long double v_cldouble = 5.0L + 6.0Li;

/* ========== TYPE_STRING ========== */
const char* global_string = "Global string literal";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable string";

/* ========== TYPE_CALLBACK - Function pointers ========== */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(int, const char*, void*);
typedef void (*event_handler)(int, void*);
typedef double (*math_func)(double);

/* Callback implementations */
void noop_callback(void) { v_int++; }
int logging_callback(int level, const char* msg, void* data) {
    printf("Level %d: %s\n", level, msg);
    return level * 2;
}
void event_processor(int event_id, void* user_data) {
    *(int*)user_data = event_id;
}
double square(double x) { return x * x; }

/* ========== TYPE_STRUCT with nested types ========== */
struct Inner {
    int id;
    char tag;
    long timestamp;
};

struct Container {
    struct {
        int a;
        char b;
        unsigned flags : 4;
        unsigned status : 2;
    } inner __attribute__((packed));
    
    union {
        long as_long;
        double as_double;
        void* as_ptr;
    } data;
    
    struct Inner* link;
    volatile int counter;
};

/* ========== TYPE_USER_STRUCT with attributes ========== */
struct __attribute__((aligned(64))) AlignedStruct {
    double matrix[8][8];
    char padding[64];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    double d;
};

/* ========== TYPE_UNION with complex members ========== */
union Variant {
    int as_int;
    long as_long;
    double as_double;
    void* as_ptr;
    struct {
        short length;
        char data[];
    } as_string;
    
    struct {
        int type;
        union {
            int i;
            float f;
            char* s;
        } value;
    } as_tagged;
};

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

struct WithVectors {
    float32x8_t vec1;
    int32x4_t vec2;
    __attribute__((aligned(32))) double aligned_double;
};

/* ========== TYPE_ARRAY - Multi-dimensional arrays ========== */
int matrix_3d[3][4][5];
struct Container container_array[10][5];
int* pointer_array[20][10];
float (*func_ptr_array[5])(float);

/* ========== TYPE_POINTER - Complex pointer chains ========== */
int**** quad_ptr;
struct Container*** container_ptr_ptr;
void (*signal_handler)(int, const char**, union Variant*);

/* ========== Struct with function pointers ========== */
struct Plugin {
    const char* name;
    int version;
    int (*init)(struct Plugin*);
    void (*process)(int, void*);
    void (*cleanup)(void);
    complex_callback user_callback;
};

int plugin_init(struct Plugin* p) {
    printf("Initializing %s v%d\n", p->name, p->version);
    return 0;
}

void plugin_process(int data, void* ctx) {
    printf("Processing data: %d\n", data);
}

void plugin_cleanup(void) {
    printf("Cleaning up plugin\n");
}

/* ========== Flexible array member struct ========== */
struct DynamicArray {
    size_t capacity;
    size_t length;
    int data[];
};

/* ========== Anonymous union within struct ========== */
struct AnonymousUnionHolder {
    int type;
    union {
        int num;
        char str[32];
        void* ptr;
    };
};

/* ========== Complex function signature ========== */
int (*complex_processor)(
    float[][256], 
    int**, 
    struct Container*,
    union Variant (*)(int, const char*)
);

/* ========== Global registry ========== */
struct Plugin plugin_registry[3] = {
    {"alpha", 1, plugin_init, plugin_process, plugin_cleanup, logging_callback},
    {"beta", 2, plugin_init, plugin_process, plugin_cleanup, NULL},
    {"gamma", 3, plugin_init, plugin_process, plugin_cleanup, logging_callback}
};

/* ========== Helper functions ========== */
void populate_matrix(void) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                matrix_3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
}

void process_container(struct Container* c, int idx) {
    c->inner.a = idx;
    c->inner.b = 'A' + (idx % 26);
    c->inner.flags = idx % 16;
    
    if (idx % 2) {
        c->data.as_long = idx * 1000L;
    } else {
        c->data.as_double = idx * 3.14159;
    }
    
    c->counter++;
}

union Variant create_variant(int type, int value) {
    union Variant v;
    if (type == 0) {
        v.as_int = value;
    } else if (type == 1) {
        v.as_tagged.type = 1;
        v.as_tagged.value.i = value;
    }
    return v;
}

float32x8_t vector_operation(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Use scalar types */
    result += v_char + v_int + (int)v_float;
    v_cfloat = v_cfloat * 2.0f;
    
    /* 2. Process strings */
    result += strlen(global_string);
    result += error_messages[0][0];
    
    /* 3. Call through function pointers */
    simple_callback cb1 = noop_callback;
    cb1();
    
    event_handler eh = event_processor;
    int event_data = 0;
    eh(42, &event_data);
    result += event_data;
    
    /* 4. Use structs and unions */
    struct Container main_container;
    process_container(&main_container, 1);
    result += main_container.inner.a;
    
    union Variant var = create_variant(0, 100);
    result += var.as_int;
    
    /* 5. Process arrays */
    populate_matrix();
    result += matrix_3d[0][0][0];
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            process_container(&container_array[i][j], i * 5 + j);
            result += container_array[i][j].counter;
        }
    }
    
    /* 6. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_c = vector_operation(vec_a, vec_b);
    result += (int)vec_c[0];
    
    /* 7. Use plugin registry with function pointers */
    for (int i = 0; i < 3; i++) {
        plugin_registry[i].init(&plugin_registry[i]);
        plugin_registry[i].process(i * 10, &result);
        
        if (plugin_registry[i].user_callback) {
            result += plugin_registry[i].user_callback(i, "Test", &result);
        }
    }
    
    /* 8. Complex pointer chain */
    int*** triple_ptr = malloc(sizeof(int**));
    *triple_ptr = malloc(sizeof(int*));
    **triple_ptr = &result;
    result += ***triple_ptr;
    
    free(*triple_ptr);
    free(triple_ptr);
    
    /* 9. Anonymous union */
    struct AnonymousUnionHolder auh;
    auh.type = 0;
    auh.num = 42;
    result += auh.num;
    
    /* 10. Flexible array member simulation */
    size_t dyn_size = 10;
    struct DynamicArray* dyn = malloc(sizeof(struct DynamicArray) + dyn_size * sizeof(int));
    dyn->capacity = dyn_size;
    dyn->length = 5;
    for (size_t i = 0; i < dyn->length; i++) {
        dyn->data[i] = (int)i * 2;
        result += dyn->data[i];
    }
    free(dyn);
    
    /* 11. Packed and aligned structs */
    struct PackedStruct ps;
    ps.a = 'X';
    ps.b = 1234;
    ps.c = 5678;
    ps.d = 9.876;
    result += ps.b;
    
    struct AlignedStruct as;
    as.matrix[0][0] = 1.0;
    result += (int)as.matrix[0][0];
    
    /* 12. Pointer to array of function pointers */
    math_func mf = square;
    result += (int)mf(4.0);
    
    /* Final deterministic result */
    printf("Final result: %d\n", result);
    
    /* Ensure all volatile variables are used */
    result += v_bool + (int)v_cdouble + (int)v_ldouble;
    
    return result % 256;
}
