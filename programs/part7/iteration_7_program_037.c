/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Incomplete/undefined type */
struct forward_declared;           /* Will be defined later */

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
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
volatile const char* volatile_string = "Hello, GGC!";
char mutable_string[] = "Mutable string";

/* ========== TYPE_CALLBACK - Function pointers ========== */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(int, const char*, void*);
typedef void (*event_handler)(int, void*);
typedef double (*math_func)(double, double);

/* Callback implementations */
static void callback_impl(void) { v_int++; }
static int complex_callback_impl(int x, const char* s, void* p) { 
    return x + (int)((char*)p - (char*)0); 
}
static void event_handler_impl(int event, void* data) { 
    *(int*)data = event; 
}
static double add_func(double a, double b) { return a + b; }

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    complex_callback cb;
};

/* Array of function pointers */
math_func math_ops[] = {add_func, NULL};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */

/* Simple struct */
struct Point {
    int x;
    int y;
    int z;
};

/* Nested anonymous struct */
struct Container {
    struct {
        int a;
        char b;
        volatile long c;
    } inner;
    union {
        long x;
        double y;
        void* z;
    } data;
    struct Point point;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
    unsigned int : 16;  /* Padding */
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {
    char id;
    int count;
    short checksum;
};

/* Aligned struct */
struct __attribute__((aligned(64))) AlignedStruct {
    double data[8];
    int tag;
};

/* Struct with flexible array member */
struct FlexArray {
    size_t length;
    int data[];
};

/* Struct containing function pointers */
struct CallbackContainer {
    event_handler on_event;
    simple_callback cleanup;
    struct Plugin* plugin;
};

/* ========== TYPE_UNION ========== */

/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Union with struct member */
union Variant {
    int as_int;
    double as_double;
    void* as_ptr;
    struct {
        short type;
        char data[16];
    } as_struct;
    struct {
        short len;
        char buf[];
    } as_string;
};

/* Tagged union */
struct TaggedUnion {
    enum { INT, FLOAT, STRING, PTR } tag;
    union {
        int i;
        float f;
        char* s;
        void* p;
    } value;
};

/* ========== TYPE_ARRAY - Complex arrays ========== */

/* Multi-dimensional array of structs */
struct Point point_matrix[10][10];

/* Array of pointers to structs */
struct Container* container_array[20];

/* Array of function pointers */
simple_callback callback_array[5];

/* Array of unions */
union Variant variant_array[8];

/* Pointer to array */
int (*array_ptr)[10];

/* ========== TYPE_POINTER - Complex pointer chains ========== */

/* Triple pointer */
int*** triple_ptr;

/* Pointer to function pointer */
math_func* func_ptr_ptr;

/* Pointer to array of pointers */
struct Point* (*point_ptr_array)[10];

/* Complex callback pointer */
int (*(*complex_callback_ptr)(int))(void);

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */

/* Vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Transparent union */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int i;
    long l;
} TransparentUnion;

/* Struct with vector member */
struct VectorData {
    float32x8_t vec_data;
    int32x4_t int_vec;
    volatile int scalar;
};

/* ========== Global instances ========== */

struct Plugin plugin_registry[3] = {
    {"plugin1", NULL, NULL, complex_callback_impl},
    {"plugin2", NULL, NULL, NULL},
    {"plugin3", NULL, NULL, NULL}
};

struct CallbackContainer global_callback = {
    event_handler_impl,
    callback_impl,
    &plugin_registry[0]
};

union Variant global_variant;

float32x8_t global_vector;

/* ========== Functions using the types ========== */

/* Function using vector type */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Process union based on tag */
static void process_union(struct TaggedUnion* tu) {
    switch (tu->tag) {
        case INT:
            tu->value.i *= 2;
            break;
        case FLOAT:
            tu->value.f *= 1.5f;
            break;
        case STRING:
            if (tu->value.s) tu->value.s[0] = 'X';
            break;
        case PTR:
            tu->value.p = (void*)((char*)tu->value.p + 1);
            break;
    }
}

/* Traverse multi-dimensional array */
static int sum_point_matrix(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += point_matrix[i][j].x;
            point_matrix[i][j].y = i + j;
        }
    }
    return sum;
}

/* Complex pointer chain traversal */
static void init_pointer_chain(void) {
    int **ptr2 = (int**)&v_int;
    int ***ptr3 = &ptr2;
    triple_ptr = ptr3;
    
    /* Create a small matrix */
    static int matrix[5][5];
    array_ptr = matrix;
    
    /* Initialize matrix */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 5 + j;
        }
    }
}

/* Call function through plugin registry */
static int call_plugin_callback(void) {
    if (plugin_registry[0].cb) {
        int dummy;
        return plugin_registry[0].cb(42, "test", &dummy);
    }
    return 0;
}

/* ========== Main function ========== */

int main(void) {
    int result = 0;
    
    /* 1. Initialize struct and union variables */
    struct Container container = {
        .inner = {1, 'B', 100L},
        .data = {.y = 3.14},
        .point = {10, 20, 30}
    };
    
    struct BitFieldStruct bfs = {1, 7, 15, -128};
    
    struct TaggedUnion tu = {INT, {.i = 42}};
    
    /* 2. Use vector type */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    global_vector = vector_add(vec_a, vec_b);
    
    /* 3. Populate array of structs with function pointers */
    callback_array[0] = callback_impl;
    callback_array[1] = NULL;
    
    /* Call function through pointer */
    if (callback_array[0]) {
        callback_array[0]();
    }
    
    /* 4. Initialize and traverse multi-dimensional array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            point_matrix[i][j].x = i * j;
            point_matrix[i][j].y = i + j;
            point_matrix[i][j].z = i - j;
        }
    }
    result += sum_point_matrix();
    
    /* 5. Process union type */
    process_union(&tu);
    result += tu.value.i;
    
    /* 6. Use pointers to traverse arrays */
    init_pointer_chain();
    
    /* Access through triple pointer */
    if (triple_ptr && *triple_ptr && **triple_ptr) {
        result += ***triple_ptr;
    }
    
    /* 7. Call plugin callback */
    result += call_plugin_callback();
    
    /* 8. Process variant union */
    global_variant.as_int = 100;
    variant_array[0].as_int = 200;
    variant_array[1].as_double = 3.14159;
    
    /* 9. Use all scalar types in computation */
    result += (int)v_char + (int)v_short + (int)v_int + (int)v_long;
    result += (int)v_float + (int)v_double;
    result += v_bool ? 1 : 0;
    
    /* 10. String operations */
    if (error_messages[0]) {
        result += error_messages[0][0];  /* 'E' = 69 */
    }
    
    /* 11. Use packed and aligned structs */
    struct PackedData pd = {'A', 100, 0x55AA};
    struct AlignedStruct as = {{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}, 99};
    result += pd.count + as.tag;
    
    /* 12. Complex callback through struct */
    if (global_callback.on_event) {
        int event_data = 0;
        global_callback.on_event(1, &event_data);
        result += event_data;
    }
    
    /* Return deterministic result for side effect */
    return result & 0xFF;  /* Return lower 8 bits */
}

/* ========== Additional type definitions ========== */

/* Define previously forward declared struct */
struct forward_declared {
    int id;
    struct opaque* next;  /* Still incomplete */
};

/* Array of forward declared structs */
struct forward_declared fd_array[5];

/* Complex type with all categories combined */
struct UltimateType {
    /* SCALAR */
    int scalar;
    
    /* STRUCT */
    struct {
        int nested;
    } inner;
    
    /* UNION */
    union {
        int a;
        float b;
    } value;
    
    /* POINTER */
    void* ptr;
    
    /* ARRAY */
    int arr[10];
    
    /* CALLBACK */
    void (*func)(void);
    
    /* STRING */
    const char* name;
    
    /* LANG_STRUCT (vector) */
    int32x4_t vec;
    
    /* Flexible array member */
    char extra[];
};

/* Global instance with flexible array */
struct UltimateType* ultimate_instance;
