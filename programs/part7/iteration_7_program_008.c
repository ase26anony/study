/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward declarations ========== */
struct opaque;  /* Incomplete/undefined type */
typedef struct incomplete incomplete_t;

/* ========== TYPE_STRUCT / Complex structures ========== */
struct Inner {
    int a;
    char b;
    volatile short c;
};

struct Container {
    struct {
        int x;
        char y;
        struct Inner nested;
    } inner_anon;
    
    union {
        long as_long;
        double as_double;
        void* as_ptr;
    } data_union;
    
    struct Inner* inner_ptr;
    _Bool flag;
};

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int mode : 3;
    unsigned int : 2;  /* Padding */
    signed int value : 10;
    unsigned long long large : 40;
};

/* Structure with flexible array member */
struct FlexArray {
    size_t length;
    int data[];
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct Container Container_t;
typedef struct BitFieldStruct BitFieldStruct_t;

/* Complex typedef chain */
typedef Container_t* ContainerPtr;
typedef ContainerPtr ContainerArray[5];

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    void* as_ptr;
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
    } tagged;
};

/* Anonymous union inside struct */
struct WithAnonymousUnion {
    int tag;
    union {
        int num;
        char str[16];
        float vec[3];
    };
};

/* ========== TYPE_LANG_STRUCT (GCC extensions) ========== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Packed structure */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Aligned structure */
struct __attribute__((aligned(64))) AlignedStruct {
    double data[8];
    int metadata;
};

/* ========== TYPE_CALLBACK / Function pointers ========== */
typedef void (*simple_cb)(void);
typedef int (*processor_t)(int, void*);
typedef void (*event_handler)(int, const char*, void*);

/* Complex callback signature */
typedef union Variant* (*transform_cb)(union Variant*, processor_t);

struct Plugin {
    const char* name;
    int version;
    int (*init)(void* context);
    void (*process)(int data);
    transform_cb transform;
    event_handler on_error;
};

/* Callback manager */
struct CallbackRegistry {
    struct Plugin plugins[3];
    simple_cb cleanup;
    processor_t* processors;
};

/* ========== TYPE_ARRAY / Complex arrays ========== */
/* Multi-dimensional array of structs */
struct Node {
    int id;
    struct Node** neighbors;
    float weight;
};

struct Node* adjacency_matrix[10][10];

/* Array of function pointers */
processor_t processor_array[5];

/* Complex array typedef */
typedef int (*array_of_funcs[3])(float, double);

/* ========== TYPE_POINTER / Pointer chains ========== */
int**** quad_ptr;  /* Four-level indirection */
struct Container*** container_ptr_ptr;

/* Pointer to array */
int (*ptr_to_array)[20];

/* Function returning pointer to array */
int (*func_returning_array_ptr(void))[10];

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

/* Complex types */
float _Complex fcomplex_var;
double _Complex dcomplex_var;

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {"Error", "Warning", "Info", "Debug", NULL};
char* dynamic_strings[3];
const char static_strings[][20] = {"Hello", "World", "Test"};

/* ========== Function implementations ========== */
int sample_processor(int x, void* ctx) {
    return x * 2 + (int)((intptr_t)ctx);
}

void sample_event_handler(int code, const char* msg, void* data) {
    volatile int dummy = code;
    (void)msg;
    (void)data;
    (void)dummy;
}

union Variant* sample_transform(union Variant* v, processor_t p) {
    static union Variant result;
    if (v && p) {
        result.as_int = p(v->as_int, NULL);
    }
    return &result;
}

int plugin_init(void* ctx) {
    return (ctx != NULL) ? 1 : 0;
}

void plugin_process(int data) {
    volatile int temp = data * 3;
    (void)temp;
}

/* Function using vector types */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex array/pointer operations */
void process_matrix(void) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (adjacency_matrix[i][j]) {
                adjacency_matrix[i][j]->id = i * 10 + j;
            }
        }
    }
}

/* Function using all type categories */
int process_variant(union Variant* v, event_handler eh) {
    static int counter = 0;
    
    if (!v) {
        if (eh) eh(-1, "Null variant", NULL);
        return -1;
    }
    
    switch (counter++ % 4) {
        case 0:
            v->as_int = 42;
            break;
        case 1:
            v->as_double = 3.14159;
            break;
        case 2:
            v->as_string.len = 5;
            for (int i = 0; i < 5; i++) {
                v->as_string.buf[i] = 'A' + i;
            }
            break;
        case 3:
            v->tagged.type = 1;
            v->tagged.value.i = 100;
            break;
    }
    
    return v->as_int;
}

/* ========== Global variables ========== */
struct Plugin plugin_registry[2];
struct CallbackRegistry cb_registry;
Container_t global_container;
union Variant global_variants[4];
float32x8_t global_vector1, global_vector2;
struct WithAnonymousUnion anonymous_union_var;

/* ========== Main function ========== */
int main(void) {
    volatile int result = 0;  /* Prevent optimization */
    
    /* 1. Initialize structs and unions */
    global_container.inner_anon.x = 10;
    global_container.inner_anon.y = 'X';
    global_container.inner_anon.nested.a = 100;
    global_container.inner_anon.nested.b = 'Y';
    global_container.data_union.as_double = 2.71828;
    global_container.flag = 1;
    
    anonymous_union_var.tag = 2;
    anonymous_union_var.num = 12345;
    
    /* 2. Initialize and use function pointers */
    plugin_registry[0].name = "TestPlugin";
    plugin_registry[0].version = 1;
    plugin_registry[0].init = plugin_init;
    plugin_registry[0].process = plugin_process;
    plugin_registry[0].transform = sample_transform;
    plugin_registry[0].on_error = sample_event_handler;
    
    cb_registry.plugins[0] = plugin_registry[0];
    cb_registry.cleanup = NULL;
    
    /* Call through function pointer */
    if (plugin_registry[0].init(&result)) {
        plugin_registry[0].process(42);
    }
    
    /* 3. Use vector types */
    float32x8_t v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t v2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vsum = vector_add(v1, v2);
    global_vector1 = vsum;
    
    /* Access vector elements to ensure usage */
    float* vp = (float*)&vsum;
    result += (int)vp[0];
    
    /* 4. Process multi-dimensional array */
    struct Node node_pool[5];
    for (int i = 0; i < 5; i++) {
        node_pool[i].id = i;
        node_pool[i].weight = i * 0.1f;
    }
    
    adjacency_matrix[0][1] = &node_pool[0];
    adjacency_matrix[1][2] = &node_pool[1];
    process_matrix();
    
    /* 5. Process union variants */
    for (int i = 0; i < 4; i++) {
        result += process_variant(&global_variants[i], 
                                 plugin_registry[0].on_error);
    }
    
    /* 6. Use pointer chains and arrays */
    int**** quad_ptr_local = &quad_ptr;
    (void)quad_ptr_local;
    
    int matrix[3][20];
    ptr_to_array = matrix;
    ptr_to_array[0][0] = result;
    
    /* 7. Use all scalar types */
    char_var = 'Z';
    schar_var = -10;
    uchar_var = 200;
    short_var = -1000;
    ushort_var = 5000;
    int_var = -100000;
    uint_var = 4000000;
    long_var = -1000000L;
    ulong_var = 4000000UL;
    llong_var = -1000000000LL;
    ullong_var = 4000000000ULL;
    float_var = 1.234f;
    double_var = 5.678;
    ldouble_var = 9.101112L;
    bool_var = 1;
    
    fcomplex_var = 1.0f + 2.0f * I;
    dcomplex_var = 3.0 + 4.0 * I;
    
    /* 8. Use strings */
    error_messages[0] = "Fatal Error";
    dynamic_strings[0] = (char*)"Dynamic";
    
    /* 9. Use bit-field struct */
    BitFieldStruct_t bf;
    bf.mode = 5;
    bf.value = -500;
    bf.large = 0x123456789AULL;
    result += bf.value;
    
    /* 10. Use packed and aligned structs */
    struct PackedStruct packed;
    packed.a = 'P';
    packed.b = 999;
    packed.c = 777;
    result += packed.b;
    
    struct AlignedStruct aligned;
    aligned.metadata = 888;
    result += aligned.metadata;
    
    /* 11. Complex callback through typedef chain */
    processor_array[0] = sample_processor;
    if (processor_array[0]) {
        result += processor_array[0](100, (void*)(intptr_t)result);
    }
    
    /* 12. Use opaque pointer (undefined type) */
    struct opaque* opaque_ptr = NULL;
    (void)opaque_ptr;
    
    /* Compute final deterministic result */
    result = (result & 0xFF) + 
             global_container.inner_anon.x + 
             anonymous_union_var.num +
             (int)(vp[0] * 100);
    
    return result;
}
