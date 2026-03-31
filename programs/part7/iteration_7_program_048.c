/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== 1. User-Defined Structures and Unions ========== */

/* Forward declaration for undefined type (TYPE_UNDEFINED) */
struct opaque;
extern struct opaque *global_opaque;

/* Basic struct with bit-fields (TYPE_STRUCT) */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* unnamed bit-field */
    signed int value : 8;
    unsigned int mode : 2;
};

/* Nested anonymous struct/union (TYPE_STRUCT, TYPE_UNION) */
struct Container {
    struct {
        int a;
        char b;
        volatile short c;  /* volatile to prevent optimization */
    } inner;
    union {
        long x;
        double y;
        void *z;
    } data;
    struct BitFieldStruct flags;
};

/* Complex union with flexible array member (TYPE_UNION) */
union Variant {
    int as_int;
    void* as_ptr;
    long double as_float;
    struct {
        short len;
        char buf[];  /* flexible array member */
    } as_string;
};

/* Packed struct with alignment attribute */
struct __attribute__((packed, aligned(2))) PackedStruct {
    char a;
    int b;
    short c;
};

/* ========== 2. Function Pointers and Callbacks ========== */

/* Function pointer typedef (TYPE_CALLBACK) */
typedef void (*event_handler)(int event_id, void* user_data);
typedef int (*processor_func)(float**, int count);

/* Struct with function pointer members */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int data);
    event_handler on_event;
    processor_func compute;
};

/* Complex callback signature */
typedef union Variant* (*transformer_t)(struct Container*, int, ...);

/* ========== 3. Arrays and Pointer Chains ========== */

/* Multi-dimensional array of structs (TYPE_ARRAY) */
struct Node {
    int id;
    struct Node** neighbors;
    float weights[4];
};

/* Complex pointer chain (TYPE_POINTER) */
struct Node* adjacency_matrix[10][10];
int ***triple_ptr_chain;
float (*signal_processor)(float[][256], int**);

/* Array of unions */
union Variant variant_array[20];

/* ========== 4. Language-Specific Types ========== */

/* GCC vector type (TYPE_LANG_STRUCT) */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
};

/* ========== 5. Scalar and String Types ========== */

/* All scalar types (TYPE_SCALAR) */
_Bool global_bool = 1;
char global_char = 'A';
signed char global_schar = -1;
unsigned char global_uchar = 255;
short global_short = -32768;
unsigned short global_ushort = 65535;
int global_int = -2147483648;
unsigned int global_uint = 4294967295;
long global_long = -9223372036854775807L;
unsigned long global_ulong = 18446744073709551615UL;
long long global_llong = -9223372036854775807LL;
unsigned long long global_ullong = 18446744073709551615ULL;
float global_float = 3.14159f;
double global_double = 2.718281828459045;
long double global_ldouble = 1.618033988749895L;

/* Complex types */
float _Complex global_cfloat = 1.0f + 2.0f * I;
double _Complex global_cdouble = 3.0 + 4.0 * I;

/* String types (TYPE_STRING) */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char* dynamic_strings[5];
const char global_const_str[] = "Global string literal";

/* ========== 6. Global Variables with Complex Types ========== */

/* Global array of structs */
struct Plugin plugin_registry[5];

/* Global union */
union Variant global_variant;

/* Global vector variable */
float32x8_t global_vector;

/* Global callback variable */
transformer_t global_transformer = NULL;

/* ========== Helper Functions ========== */

/* Function to be called via function pointer */
int plugin_init_default(void) {
    static int counter = 0;
    return ++counter;
}

void plugin_process_default(int data) {
    global_int += data;
}

void event_handler_default(int event_id, void* user_data) {
    struct Container* cont = (struct Container*)user_data;
    if (cont) {
        cont->inner.a = event_id;
    }
}

int processor_default(float** data, int count) {
    int sum = 0;
    for (int i = 0; i < count && i < 10; i++) {
        if (data[i]) {
            sum += (int)data[i][0];
        }
    }
    return sum;
}

/* Function using vector type */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex parameter types */
union Variant* transform_data(struct Container* cont, int mode, ...) {
    static union Variant result;
    
    if (!cont) {
        result.as_int = -1;
        return &result;
    }
    
    switch (mode) {
        case 0:
            result.as_int = cont->inner.a;
            break;
        case 1:
            result.as_float = (long double)cont->inner.b;
            break;
        default:
            result.as_ptr = &cont->data;
            break;
    }
    
    return &result;
}

/* ========== Main Function ========== */

int main(void) {
    /* 1. Initialize complex struct and union types */
    struct Container container = {
        .inner = { .a = 42, .b = 'X', .c = 1000 },
        .data = { .y = 3.14159 },
        .flags = { .flag1 = 1, .flag2 = 5, .value = 127, .mode = 3 }
    };
    
    union Variant local_variant;
    local_variant.as_int = 12345;
    
    struct PackedStruct packed = { .a = 'Z', .b = 999, .c = -50 };
    
    /* 2. Populate array of structs with function pointers */
    for (int i = 0; i < 5; i++) {
        plugin_registry[i].name = error_messages[i % 3];
        plugin_registry[i].init = plugin_init_default;
        plugin_registry[i].process = plugin_process_default;
        plugin_registry[i].on_event = event_handler_default;
        plugin_registry[i].compute = processor_default;
    }
    
    /* Call function through pointer */
    if (plugin_registry[0].init) {
        int result = plugin_registry[0].init();
        plugin_registry[0].process(result);
    }
    
    /* 3. Use GCC vector type */
    float32x8_t vec_a = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
    float32x8_t vec_b = { 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
    float32x8_t vec_sum = vector_add(vec_a, vec_b);
    global_vector = vec_sum;
    
    /* 4. Use pointers and multi-dimensional arrays */
    struct Node* nodes[10];
    for (int i = 0; i < 10; i++) {
        nodes[i] = (struct Node*)&container;  /* Simplified for example */
        for (int j = 0; j < 10; j++) {
            adjacency_matrix[i][j] = nodes[i];
        }
    }
    
    /* Triple pointer chain */
    int value = 42;
    int *p1 = &value;
    int **p2 = &p1;
    triple_ptr_chain = &p2;
    
    /* 5. Process union type with runtime condition */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            variant_array[i].as_int = i * 10;
        } else if (i % 3 == 1) {
            variant_array[i].as_float = (long double)i / 3.0L;
        } else {
            variant_array[i].as_ptr = &variant_array[(i + 1) % 20];
        }
    }
    
    /* Access union based on condition */
    int condition = container.inner.a % 3;
    switch (condition) {
        case 0:
            local_variant.as_int = 100;
            break;
        case 1:
            local_variant.as_float = 2.71828L;
            break;
        case 2:
            local_variant.as_ptr = &container;
            break;
    }
    
    /* 6. Use callback type */
    global_transformer = transform_data;
    if (global_transformer) {
        union Variant* transformed = global_transformer(&container, 0);
        if (transformed) {
            local_variant.as_int = transformed->as_int;
        }
    }
    
    /* 7. Use string types */
    dynamic_strings[0] = (char*)error_messages[0];
    dynamic_strings[1] = (char*)global_const_str;
    
    /* 8. Complex array of pointers to functions */
    processor_func func_array[3];
    func_array[0] = processor_default;
    func_array[1] = processor_default;
    func_array[2] = NULL;
    
    /* 9. Use all scalar types in computation */
    long long hash = 0;
    hash += global_bool;
    hash += global_char;
    hash += global_schar;
    hash += global_uchar;
    hash += global_short;
    hash += global_ushort;
    hash += global_int;
    hash += global_uint;
    hash += global_long;
    hash += global_ulong;
    hash += global_llong;
    hash += global_ullong;
    hash += (long long)global_float;
    hash += (long long)global_double;
    hash += (long long)global_ldouble;
    
    /* Use complex numbers */
    hash += (long long)creal(global_cfloat);
    hash += (long long)cimag(global_cfloat);
    hash += (long long)creal(global_cdouble);
    hash += (long long)cimag(global_cdouble);
    
    /* Add vector elements */
    for (int i = 0; i < 8; i++) {
        hash += (long long)vec_sum[i];
    }
    
    /* Process container data */
    hash += container.inner.a;
    hash += container.inner.b;
    hash += container.inner.c;
    hash += (long long)container.data.y;
    hash += container.flags.flag1;
    hash += container.flags.flag2;
    hash += container.flags.value;
    hash += container.flags.mode;
    
    /* Process packed struct */
    hash += packed.a;
    hash += packed.b;
    hash += packed.c;
    
    /* Process variant array */
    for (int i = 0; i < 20; i++) {
        hash += variant_array[i].as_int;
    }
    
    /* Final deterministic return value */
    return (int)(hash % 1000);
}
