/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <string.h>

/* ==================== TYPE_UNDEFINED ==================== */
struct opaque;  /* Forward declaration - undefined type */
extern struct opaque *global_opaque_ptr;

/* ==================== TYPE_SCALAR ==================== */
/* All fundamental scalar types */
volatile char v_char = 'A';
volatile signed char v_schar = -1;
volatile unsigned char v_uchar = 255;
volatile short v_short = -32768;
volatile unsigned short v_ushort = 65535;
volatile int v_int = -2147483647-1;
volatile unsigned int v_uint = 4294967295U;
volatile long v_long = -9223372036854775807L-1;
volatile unsigned long v_ulong = 18446744073709551615UL;
volatile long long v_llong = -9223372036854775807LL-1;
volatile unsigned long long v_ullong = 18446744073709551615ULL;
volatile float v_float = 3.14159f;
volatile double v_double = 2.718281828459045;
volatile _Bool v_bool = 1;
volatile _Complex float v_cfloat = 1.0f + 2.0fi;
volatile _Complex double v_cdouble = 3.0 + 4.0i;

/* ==================== TYPE_STRING ==================== */
const char *global_string = "Hello, GGC Type System!";
const char *error_messages[] = {"Fatal", "Error", "Warning", "Info", "Debug"};
char mutable_string[] = "Mutable string buffer";

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(int, const char*);
typedef void* (*allocator_fn)(size_t);
typedef void (*cleanup_fn)(void*);

/* Complex callback signature */
typedef int (*signal_processor)(float[][256], int**, union Variant*);

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    void (*shutdown)(void);
    event_handler on_error;
};

/* Another callback structure */
struct MathOps {
    binary_op add;
    binary_op sub;
    binary_op mul;
    binary_op div;
};

/* ==================== TYPE_STRUCT ==================== */
/* Simple nested structure */
struct Inner {
    int counter;
    char tag;
    float precision;
};

/* Bit-field structure */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
    signed int value : 16;
};

/* Structure with anonymous struct */
struct Container {
    struct {
        int id;
        char name[32];
    } header;
    union {
        long timestamp;
        double weight;
        void* pointer;
    } data;
    struct Inner inner;
};

/* Flexible array member structure */
struct DynamicArray {
    size_t length;
    size_t capacity;
    int elements[];
};

/* Packed structure */
struct __attribute__((packed)) PackedData {
    char type;
    int value;
    short count;
};

/* Aligned structure */
struct __attribute__((aligned(64))) CacheLine {
    double matrix[8][8];
    int access_count;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined structure with typedef */
typedef struct Node {
    int value;
    struct Node* next;
    struct Node* prev;
} ListNode;

typedef struct {
    int x, y, z;
} Point3D;

/* Complex user structure */
typedef struct {
    Point3D position;
    Point3D velocity;
    float mass;
    char name[16];
    void* user_data;
} Particle;

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Tagged union */
union Variant {
    int as_int;
    double as_double;
    char* as_string;
    void* as_pointer;
    struct {
        short type;
        char data[32];
    } as_custom;
};

/* Union with anonymous struct */
union AnonymousUnion {
    struct {
        int x, y;
    } point;
    struct {
        float lat, lon;
    } coord;
    long long bits;
};

/* ==================== TYPE_ARRAY ==================== */
/* Multi-dimensional arrays */
int matrix_3d[4][4][4];
struct Container containers[10];
union Variant variants[20];

/* Array of pointers */
int* int_ptr_array[50];
struct Node* node_ptr_array[100];
void* void_ptr_matrix[5][5];

/* Pointer to array */
int (*array_ptr)[10];
float (*matrix_ptr)[20][20];

/* ==================== TYPE_POINTER ==================== */
/* Multi-level pointers */
int*** triple_ptr;
struct Container**** quad_container_ptr;
void (* volatile * volatile func_ptr_ptr)(void);

/* Function returning pointer to array */
int (*get_matrix(void))[10][10];

/* Pointer to function returning pointer */
int* (*get_int_ptr_fn)(void);

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;
typedef short __attribute__((vector_size(8))) int16x8_t;

/* Vector operations structure */
struct VectorOps {
    float32x8_t vec1;
    float32x8_t vec2;
    int32x4_t ivec;
    void (*process_vector)(float32x8_t*, const float32x8_t*);
};

/* ==================== FUNCTION DEFINITIONS ==================== */
/* Callback implementations */
int add_impl(int a, int b) { return a + b; }
int sub_impl(int a, int b) { return a - b; }
int mul_impl(int a, int b) { return a * b; }
int div_impl(int a, int b) { return b != 0 ? a / b : 0; }

void error_handler(int code, const char* msg) {
    volatile int dummy = code;
    const char* dummy2 = msg;
    (void)dummy; (void)dummy2;
}

int plugin_init(void) { return 0; }
void plugin_process(int x) { volatile int y = x * 2; (void)y; }
void plugin_shutdown(void) {}

/* Complex callback */
int process_signal(float signals[][256], int** coefficients, union Variant* config) {
    volatile float sum = 0.0f;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 256; j++) {
            sum += signals[i][j];
        }
    }
    return (int)sum;
}

/* Vector operation */
void vector_add(float32x8_t* result, const float32x8_t* a) {
    *result = *result + *a;
}

/* ==================== GLOBAL VARIABLES ==================== */
/* Global instances of complex types */
struct Plugin global_plugin = {
    .name = "TestPlugin",
    .init = plugin_init,
    .process = plugin_process,
    .shutdown = plugin_shutdown,
    .on_error = error_handler
};

struct MathOps global_math_ops = {
    .add = add_impl,
    .sub = sub_impl,
    .mul = mul_impl,
    .div = div_impl
};

struct VectorOps global_vec_ops = {
    .vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f},
    .vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f},
    .ivec = {1, 2, 3, 4},
    .process_vector = vector_add
};

/* Array of structs with function pointers */
struct Plugin plugin_registry[3] = {
    {"PluginA", plugin_init, plugin_process, plugin_shutdown, error_handler},
    {"PluginB", plugin_init, plugin_process, plugin_shutdown, error_handler},
    {"PluginC", plugin_init, plugin_process, plugin_shutdown, error_handler}
};

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* 1. Initialize and use struct types */
    struct Container container = {
        .header = {1, "TestContainer"},
        .data = {.timestamp = 1234567890},
        .inner = {42, 'X', 3.14f}
    };
    
    struct BitFieldStruct bfs = {1, 7, 15, 0xFFFFFF, -32768};
    
    /* 2. Use union types with different members */
    union Variant var;
    var.as_int = 42;
    volatile int vi = var.as_int;
    
    var.as_double = 3.14159;
    volatile double vd = var.as_double;
    
    var.as_string = "Union string";
    volatile const char* vcs = var.as_string;
    
    /* 3. Process multi-dimensional arrays */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                matrix_3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* 4. Use pointer chains */
    int value = 42;
    int* p1 = &value;
    int** p2 = &p1;
    int*** p3 = &p2;
    triple_ptr = p3;
    
    volatile int deref = ***triple_ptr;
    
    /* 5. Call functions through function pointers */
    int result = global_math_ops.add(10, 20);
    result = global_math_ops.mul(result, 2);
    
    /* Call plugin functions */
    for (int i = 0; i < 3; i++) {
        if (plugin_registry[i].init) {
            plugin_registry[i].init();
        }
        if (plugin_registry[i].process) {
            plugin_registry[i].process(i);
        }
    }
    
    /* 6. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = vec_a + vec_b;
    
    /* Process vector through callback */
    if (global_vec_ops.process_vector) {
        global_vec_ops.process_vector(&global_vec_ops.vec1, &global_vec_ops.vec2);
    }
    
    /* 7. Complex callback with multi-dimensional parameters */
    float signals[4][256];
    int* coeffs[10];
    int coeff_data[10][20];
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 256; j++) {
            signals[i][j] = (float)(i * j);
        }
    }
    
    for (int i = 0; i < 10; i++) {
        coeffs[i] = coeff_data[i];
        for (int j = 0; j < 20; j++) {
            coeff_data[i][j] = i + j;
        }
    }
    
    union Variant config;
    config.as_custom.type = 1;
    strcpy(config.as_custom.data, "Signal processing config");
    
    signal_processor processor = process_signal;
    volatile int proc_result = processor(signals, coeffs, &config);
    
    /* 8. Use all scalar types in computation */
    volatile long long scalar_sum = 
        (long long)v_char + v_schar + v_uchar +
        v_short + v_ushort + v_int + v_uint +
        v_long + v_ulong + v_llong + v_ullong +
        (long long)v_float + (long long)v_double +
        v_bool;
    
    /* 9. String operations */
    volatile size_t str_len = strlen(global_string);
    volatile const char* second_msg = error_messages[1];
    
    /* 10. Create and use linked list (user struct) */
    ListNode node1 = {100, NULL, NULL};
    ListNode node2 = {200, NULL, &node1};
    ListNode node3 = {300, NULL, &node2};
    node1.next = &node2;
    node2.next = &node3;
    
    /* Traverse list */
    volatile int list_sum = 0;
    ListNode* current = &node1;
    while (current) {
        list_sum += current->value;
        current = current->next;
    }
    
    /* 11. Use aligned and packed structures */
    struct CacheLine cache_line;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            cache_line.matrix[i][j] = (double)(i * j);
        }
    }
    cache_line.access_count++;
    
    struct PackedData packed = {'Z', 0xABCDEF, 999};
    volatile int packed_value = packed.value;
    
    /* 12. Compute final deterministic result using all type categories */
    int final_result = 
        result +                  /* From callback operations */
        deref +                   /* From pointer chain */
        proc_result +             /* From complex callback */
        (int)scalar_sum % 1000 +  /* From scalar types */
        (int)str_len +            /* From string types */
        list_sum +                /* From user struct (linked list) */
        (int)vec_sum[0] +         /* From vector type */
        packed_value % 1000 +     /* From packed struct */
        cache_line.access_count;  /* From aligned struct */
    
    /* Ensure all volatile variables are used */
    (void)vi; (void)vd; (void)vcs; (void)deref;
    
    return final_result % 256;  /* Return deterministic value */
}

/* Additional function to ensure type visibility */
void unused_function_to_keep_types_alive(void) {
    /* Reference all types to ensure they're not optimized away */
    volatile struct opaque* op = global_opaque_ptr;
    (void)op;
    
    volatile struct DynamicArray* da = NULL;
    (void)da;
    
    volatile union AnonymousUnion au;
    (void)au;
    
    volatile int (*array_ptr_local)[10] = array_ptr;
    (void)array_ptr_local;
    
    volatile float (*matrix_ptr_local)[20][20] = matrix_ptr;
    (void)matrix_ptr_local;
    
    volatile int* (*get_int_ptr_fn_local)(void) = get_int_ptr_fn;
    (void)get_int_ptr_fn_local;
    
    volatile void (* volatile * volatile fpp)(void) = func_ptr_ptr;
    (void)fpp;
}
