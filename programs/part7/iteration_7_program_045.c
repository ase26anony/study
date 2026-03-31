/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED ========== */
struct opaque;  /* Forward declaration - undefined type */
extern struct opaque *global_opaque_ptr;

/* ========== TYPE_STRUCT and TYPE_USER_STRUCT ========== */
/* Simple struct */
struct Point {
    int x;
    int y;
};

/* Nested anonymous struct */
struct Container {
    struct {
        int a;
        char b;
        unsigned char : 4;  /* bitfield */
        unsigned char flag : 1;
    } inner;
    union {
        long x;
        double y;
        void *ptr;
    } data;
    volatile int counter;  /* Prevent optimization */
};

/* Struct with flexible array member */
struct DynamicString {
    size_t length;
    char data[];
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedData {
    uint16_t id;
    uint32_t value;
    uint8_t flags;
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    struct {
        short len;
        char buf[32];  /* Fixed size instead of flexible for portability */
    } as_string;
    long long as_llong;
};

/* Anonymous union within struct */
struct TaggedUnion {
    int tag;
    union {
        int i;
        float f;
        char *s;
    } value;
};

/* ========== TYPE_LANG_STRUCT (GCC extensions) ========== */
/* GCC vector type */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Struct with vector types */
struct SIMDData {
    float32x8_t vec1;
    float32x8_t vec2;
    int32x4_t ivec;
    __attribute__((aligned(64))) double aligned_double;
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*event_handler)(int event_id, void* user_data);
typedef int (*comparator_fn)(const void*, const void*);
typedef float (*transform_fn)(float, float);

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
    event_handler on_event;
};

/* More complex callback signature */
typedef union Variant* (*processor_fn)(struct Container*, int, ...);

/* ========== TYPE_ARRAY and TYPE_POINTER ========== */
/* Complex array types */
struct Node* adjacency_matrix[10][10];  /* 2D array of pointers */

/* Array of structs */
struct Point point_grid[5][5];

/* Array of function pointers */
transform_fn transforms[8];

/* Multi-level pointer */
int ****quadruple_ptr;

/* Pointer to array */
int (*array_ptr)[20];

/* Pointer to function returning pointer to array */
int (*(*complex_func_ptr)(void))[10];

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
const char* error_messages[] = {"Error", "Warning", "Info", "Debug", NULL};
char static_string[] = "Hello, GCC type system!";
const char* const_string = "Constant string literal";

/* ========== Global instances ========== */
struct Container global_container = {
    .inner = {42, 'A', 1},
    .data = {.y = 3.14159}
};

union Variant global_variant = {.as_int = 100};

struct SIMDData global_simd;

struct Plugin plugin_registry[3];

/* ========== Function implementations ========== */
int plugin1_init(void) {
    return 0;
}

void plugin1_process(int data) {
    /* Do something */
    (void)data;
}

void plugin1_cleanup(struct Plugin* self) {
    /* Cleanup */
    (void)self;
}

void sample_event_handler(int event_id, void* user_data) {
    /* Handle event */
    (void)event_id;
    (void)user_data;
}

int plugin2_init(void) { return 1; }
void plugin2_process(int data) { (void)data; }
void plugin2_cleanup(struct Plugin* self) { (void)self; }

float add_transform(float a, float b) { return a + b; }
float mul_transform(float a, float b) { return a * b; }

/* Function using pointer chain */
int process_pointer_chain(int ****ptr) {
    if (ptr && *ptr && **ptr && ***ptr) {
        return ****ptr + 1;
    }
    return 0;
}

/* Function using multi-dimensional array */
void init_adjacency_matrix(void) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            adjacency_matrix[i][j] = NULL;
        }
    }
}

/* Function processing union based on tag */
void process_variant(union Variant *v, int type) {
    switch(type) {
        case 0:
            v->as_int = 42;
            break;
        case 1:
            v->as_float = 3.14f;
            break;
        case 2:
            strncpy(v->as_string.buf, "test", 31);
            v->as_string.buf[31] = '\0';
            v->as_string.len = 4;
            break;
        default:
            v->as_ptr = NULL;
    }
}

/* Function using GCC vector types */
void simd_operations(void) {
    float32x8_t a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t b = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    float32x8_t c = a + b;  /* Vector addition */
    
    /* Store result in global */
    global_simd.vec1 = a;
    global_simd.vec2 = b;
    
    /* Use int vector */
    int32x4_t iv1 = {1, 2, 3, 4};
    int32x4_t iv2 = {5, 6, 7, 8};
    global_simd.ivec = iv1 + iv2;
}

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize struct and union variables */
    struct Container local_container = {
        .inner = {100, 'Z', 0},
        .data = {.x = 123456789L}
    };
    
    union Variant local_variant;
    process_variant(&local_variant, 0);
    process_variant(&global_variant, 1);
    
    /* 2. Initialize plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "Plugin1",
        .init = plugin1_init,
        .process = plugin1_process,
        .cleanup = plugin1_cleanup,
        .on_event = sample_event_handler
    };
    
    plugin_registry[1] = (struct Plugin){
        .name = "Plugin2",
        .init = plugin2_init,
        .process = plugin2_process,
        .cleanup = plugin2_cleanup,
        .on_event = NULL
    };
    
    /* Call function through pointer */
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    /* 3. Use GCC vector types */
    simd_operations();
    
    /* 4. Initialize and use multi-dimensional arrays */
    init_adjacency_matrix();
    
    /* Initialize point grid */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            point_grid[i][j].x = i * 10;
            point_grid[i][j].y = j * 10;
            result += point_grid[i][j].x + point_grid[i][j].y;
        }
    }
    
    /* 5. Initialize transform function array */
    transforms[0] = add_transform;
    transforms[1] = mul_transform;
    
    /* Use transforms */
    if (transforms[0]) {
        float transform_result = transforms[0](2.5f, 3.5f);
        result += (int)transform_result;
    }
    
    /* 6. Process union types */
    result += local_variant.as_int;
    result += (int)global_variant.as_float;
    
    /* 7. Use all scalar types */
    char_var = 'A';
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
    
    result += char_var + schar_var + uchar_var;
    result += (int)(float_var + double_var);
    result += (int)(creal(complex_float) + cimag(complex_double));
    
    /* 8. Use string types */
    result += (int)strlen(static_string);
    result += (int)strlen(error_messages[0]);
    
    /* 9. Create pointer chain */
    int value = 42;
    int *p1 = &value;
    int **p2 = &p1;
    int ***p3 = &p2;
    quadruple_ptr = &p3;
    
    result += process_pointer_chain(quadruple_ptr);
    
    /* 10. Use array pointer */
    int local_array[20];
    array_ptr = &local_array;
    for (int i = 0; i < 20; i++) {
        (*array_ptr)[i] = i * 2;
        result += (*array_ptr)[i];
    }
    
    /* 11. Use packed struct */
    struct PackedData packed = {.id = 0xABCD, .value = 0x12345678, .flags = 0x1};
    result += packed.id + packed.value + packed.flags;
    
    /* 12. Use tagged union */
    struct TaggedUnion tagged = {.tag = 1, .value = {.f = 3.14f}};
    result += (int)(tagged.value.f * 100);
    
    /* Return deterministic result for verification */
    printf("Result: %d\n", result);
    return result % 256;  /* Return value between 0-255 */
}
