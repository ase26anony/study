/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward declarations ========== */
struct opaque;  /* Incomplete/undefined type */
typedef struct opaque *opaque_ptr_t;

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Basic struct with various members */
struct Point {
    int x;
    int y;
    char label[32];
};

/* Nested struct with anonymous inner struct */
struct Container {
    struct {
        int a;
        char b;
        volatile short c;  /* Prevent optimization */
    } inner;
    
    union {
        long x;
        double y;
        _Complex float z;
    } data;
    
    struct Point *points[10];
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* Padding */
    signed int value : 8;
    unsigned long reserved : 16;
};

/* Struct with flexible array member */
struct FlexArray {
    size_t length;
    int data[];
};

/* Struct with GCC attributes */
struct __attribute__((packed, aligned(2))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((deprecated));

/* ========== TYPE_UNION ========== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* Complex union with nested struct */
union Variant {
    int as_int;
    void* as_ptr;
    double as_double;
    struct {
        short len;
        char buf[];  /* Flexible array member in union */
    } as_string;
    
    struct Point as_point;
    _Complex double as_complex;
};

/* Tagged union (discriminated) */
struct TaggedUnion {
    enum { INT, FLOAT, STRING, POINT } tag;
    union {
        int i;
        float f;
        char *s;
        struct Point p;
    } value;
};

/* ========== TYPE_CALLBACK / Function pointers ========== */
/* Simple callback typedef */
typedef void (*simple_callback)(int, void*);

/* Complex function pointer typedef */
typedef int (*complex_callback)(struct Point**, union Variant*, size_t);

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    complex_callback (*get_callback)(void);
    void (*cleanup)(struct Plugin*);
};

/* Another callback type for arrays */
typedef void (*array_processor)(float[][256], int**);

/* ========== TYPE_LANG_STRUCT / GCC extensions ========== */
/* GCC vector type */
typedef float __attribute__((vector_size(32))) float32x8_t;

/* GCC vector type for integers */
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Struct with vector members */
struct VectorStruct {
    float32x8_t vec_data;
    int32x4_t int_vec;
    __attribute__((aligned(64))) double aligned_double;
};

/* ========== TYPE_ARRAY / Complex arrays ========== */
/* Multi-dimensional array of structs */
struct Point point_grid[5][5];

/* Array of pointers to unions */
union Variant *variant_array[20];

/* Pointer to array */
int (*array_ptr)[10];

/* Complex function pointer with array parameter */
int (*signal_processor)(float[][256], int**, struct Point (*)[5]);

/* Array of function pointers */
simple_callback callbacks[8];

/* ========== TYPE_POINTER / Pointer chains ========== */
/* Triple pointer */
int ***triple_ptr;

/* Pointer to pointer to struct */
struct Container **container_pp;

/* Pointer to array of pointers */
struct Point *(*point_array_ptr)[10];

/* ========== TYPE_SCALAR / All scalar types ========== */
/* Global scalars of all types */
char global_char = 'A';
signed char global_schar = -1;
unsigned char global_uchar = 255;
short global_short = -32768;
unsigned short global_ushort = 65535;
int global_int = -2147483648;
unsigned int global_uint = 4294967295U;
long global_long = -9223372036854775807L;
unsigned long global_ulong = 18446744073709551615UL;
long long global_llong = -9223372036854775807LL;
unsigned long long global_ullong = 18446744073709551615ULL;
float global_float = 3.14159f;
double global_double = 2.718281828459045;
long double global_ldouble = 1.4142135623730950488L;
_Bool global_bool = 1;
_Complex float global_cfloat = 1.0f + 2.0fi;
_Complex double global_cdouble = 3.0 + 4.0i;
_Complex long double global_cldouble = 5.0L + 6.0Li;

/* ========== TYPE_STRING ========== */
/* String literals and arrays */
const char* error_messages[] = {"Error", "Warning", "Info", "Debug"};
char *string_array[] = {"Hello", "World", "GCC", "Coverage"};
const char *const constant_strings[] = {"Constant1", "Constant2"};

/* ========== Global instances ========== */
/* Ensure types are actually used */
struct Container global_container;
union Variant global_variant;
struct Plugin plugin_registry[3];
struct VectorStruct global_vector;
struct TaggedUnion global_tagged;

/* ========== Function implementations ========== */
/* Callback implementations */
static int plugin_init_default(void) {
    return 0;
}

static void plugin_process_default(int x) {
    global_int += x;
}

static complex_callback plugin_get_callback_default(void) {
    return NULL;
}

static void plugin_cleanup_default(struct Plugin* p) {
    p->name = NULL;
}

static void simple_callback_impl(int x, void* data) {
    *(int*)data = x * 2;
}

static int complex_callback_impl(struct Point** points, union Variant* v, size_t count) {
    if (count > 0 && points && points[0]) {
        return points[0]->x + (int)count;
    }
    return -1;
}

/* Function using vector types */
static float32x8_t add_vectors(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function to process multi-dimensional array */
static int process_grid(struct Point grid[5][5]) {
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            sum += grid[i][j].x + grid[i][j].y;
        }
    }
    return sum;
}

/* Function using pointer chains */
static int follow_triple_pointer(int ***ptr) {
    if (ptr && *ptr && **ptr) {
        return ***ptr;
    }
    return 0;
}

/* Function processing unions */
static double process_union(union Variant *v, int type) {
    switch (type) {
        case 0: return (double)v->as_int;
        case 1: return v->as_double;
        case 2: return (double)v->as_point.x;
        default: return 0.0;
    }
}

/* ========== Main function ========== */
int main(void) {
    volatile int result = 0;  /* Prevent optimization */
    
    /* 1. Initialize structs and unions */
    struct Container local_container = {
        .inner = { .a = 42, .b = 'X', .c = 100 },
        .data = { .y = 3.14159 }
    };
    
    union Variant local_variant;
    local_variant.as_int = 12345;
    
    struct TaggedUnion tagged = {
        .tag = POINT,
        .value = { .p = { .x = 10, .y = 20, .label = "Test" } }
    };
    
    /* 2. Initialize plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .init = plugin_init_default,
        .process = plugin_process_default,
        .get_callback = plugin_get_callback_default,
        .cleanup = plugin_cleanup_default
    };
    
    /* Call function through pointer */
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    /* 3. Use vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = add_vectors(vec_a, vec_b);
    global_vector.vec_data = vec_sum;
    
    /* 4. Initialize and use multi-dimensional array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            point_grid[i][j].x = i * 10;
            point_grid[i][j].y = j * 10;
        }
    }
    result += process_grid(point_grid);
    
    /* 5. Use pointer chains */
    int value = 999;
    int *p1 = &value;
    int **p2 = &p1;
    triple_ptr = &p2;
    result += follow_triple_pointer(triple_ptr);
    
    /* 6. Process union type */
    result += (int)process_union(&local_variant, 0);
    result += (int)process_union(&global_variant, 1);
    
    /* 7. Use callbacks */
    int callback_data = 0;
    callbacks[0] = simple_callback_impl;
    if (callbacks[0]) {
        callbacks[0](21, &callback_data);
        result += callback_data;
    }
    
    /* 8. Use array of pointers */
    struct Point p1_struct = {1, 2, "A"};
    struct Point p2_struct = {3, 4, "B"};
    struct Point *point_ptrs[2] = {&p1_struct, &p2_struct};
    variant_array[0] = &local_variant;
    
    /* 9. Use all scalar types in computation */
    result += global_char + global_schar + global_uchar;
    result += global_short + global_ushort;
    result += (int)global_float + (int)global_double;
    result += (int)creal(global_cfloat) + (int)cimag(global_cfloat);
    result += global_bool ? 1 : 0;
    
    /* 10. Use string arrays */
    for (int i = 0; i < 4; i++) {
        if (error_messages[i]) {
            result += error_messages[i][0];  /* Use first char */
        }
    }
    
    /* 11. Use bit-field struct */
    struct BitFieldStruct bfs = { .flag1 = 1, .flag2 = 3, .value = -42, .reserved = 0xABCD };
    result += bfs.value;
    
    /* 12. Use opaque pointer type */
    opaque_ptr_t opaque_ptr = NULL;
    result += (opaque_ptr == NULL) ? 0 : 1;
    
    /* 13. Complex array/pointer combination */
    int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int (*matrix_ptr)[3] = matrix;
    result += matrix_ptr[1][1];  /* Center value (5) */
    
    /* 14. Use packed struct */
    struct PackedStruct packed = { .a = 'Z', .b = 1234, .c = 5678 };
    result += packed.b;
    
    /* Return deterministic result based on all operations */
    return result % 256;  /* Ensure result fits in return value */
}

/* Additional functions to ensure type usage */
void __attribute__((noinline)) use_types_extensively(void) {
    /* Force usage of various types */
    static struct FlexArray *flex_ptr = NULL;
    static array_processor proc = NULL;
    static struct Point (*grid_ptr)[5] = point_grid;
    
    /* Use vector type in computation */
    int32x4_t int_vec = {1, 2, 3, 4};
    global_vector.int_vec = int_vec + int_vec;
    
    /* Complex pointer indirection */
    struct Container *containers[5];
    container_pp = &containers[0];
    
    /* Use all string types */
    const char *dynamic_string = string_array[0];
    volatile const char *volatile_string = constant_strings[1];
    
    /* Prevent unused variable warnings */
    (void)flex_ptr;
    (void)proc;
    (void)grid_ptr;
    (void)dynamic_string;
    (void)volatile_string;
}
