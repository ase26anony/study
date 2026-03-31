/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Incomplete/undefined type */
typedef struct opaque *opaque_ptr_t;

/* ========== TYPE_STRUCT / Complex Structures ========== */
/* Basic nested structure */
struct Inner {
    int a;
    char b;
    volatile short c;              /* Prevent optimization */
};

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;              /* Unnamed bit-field */
    signed int value : 8;
    unsigned long reserved : 16;
};

/* Structure with anonymous union */
struct WithAnonymousUnion {
    int type;
    union {
        int int_val;
        double dbl_val;
        void *ptr_val;
    };
    char tag;
};

/* Main container structure */
struct Container {
    struct Inner inner;
    struct BitFieldStruct bits;
    struct WithAnonymousUnion anon_union;
    const char *name;
};

/* ========== TYPE_USER_STRUCT / Typedef Structures ========== */
typedef struct Container Container_t;
typedef struct {
    Container_t *items[10];
    int count;
    _Bool active;
} ContainerArray;

/* ========== TYPE_UNION / Complex Unions ========== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* Union with flexible array member */
union Variant {
    int as_int;
    double as_double;
    void *as_pointer;
    struct {
        short len;
        char buf[];                /* Flexible array member */
    } as_string;
};

/* Union containing structures */
union ComplexUnion {
    struct {
        int x, y;
    } point;
    struct {
        float r, g, b, a;
    } color;
    long long bits64;
};

/* ========== TYPE_LANG_STRUCT / GCC Extensions ========== */
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
    int index;
};

/* ========== TYPE_CALLBACK / Function Pointers ========== */
/* Callback typedefs */
typedef void (*simple_callback)(void);
typedef int (*processor_t)(int, const char*);
typedef void (*event_handler)(int, void*);
typedef union Variant* (*variant_factory)(int);

/* Structure with function pointers */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void* context);
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
    event_handler on_event;
};

/* More complex callback signature */
typedef int (*(*factory_fn)(int))(void*, ...);

/* ========== TYPE_ARRAY / Complex Arrays ========== */
/* Multi-dimensional arrays */
int matrix_2d[5][5];
float matrix_3d[3][3][3];

/* Array of pointers */
struct Container* container_array[8];
union Variant* variant_ptr_array[16];

/* Array of function pointers */
processor_t processors[4];

/* Array of arrays */
typedef int int_matrix[4][4];
int_matrix matrices[2];

/* ========== TYPE_POINTER / Complex Pointers ========== */
/* Multi-level pointers */
int ***triple_ptr;
struct Container**** container_ptr_chain;

/* Pointer to array */
int (*ptr_to_array)[10];
float (*ptr_to_2d_array)[5][5];

/* Pointer to function pointer */
processor_t *processor_ptr;

/* ========== TYPE_SCALAR / All Scalar Types ========== */
/* Basic scalars */
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

/* Floating point */
float float_var;
double double_var;
long double ldouble_var;

/* Complex numbers */
float _Complex fcomplex_var;
double _Complex dcomplex_var;

/* Boolean */
_Bool bool_var;

/* Fixed width integers */
int8_t i8_var;
uint64_t u64_var;

/* ========== TYPE_STRING / String Types ========== */
const char* string_literal = "Hello, GCC!";
char string_array[] = "Test string";
const char* messages[] = {"Error", "Warning", "Info", "Debug"};
char* string_ptr_array[4];

/* ========== Global Variables ========== */
/* Ensure types are used globally */
Container_t global_container;
ContainerArray global_container_array;
union ComplexUnion global_union;
struct Plugin plugin_registry[3];
float32x8_t global_vector;

/* ========== Function Definitions ========== */
/* Functions to be called via function pointers */
static int plugin_init(void* context) {
    (void)context;
    return 42;
}

static void plugin_process(int data) {
    volatile int result = data * 2;  /* Prevent optimization */
    (void)result;
}

static void plugin_cleanup(struct Plugin* self) {
    if (self) {
        /* Mark as inactive */
        volatile char* name = (char*)self->name;
        (void)name;
    }
}

static void handle_event(int event_id, void* data) {
    volatile int id = event_id;
    volatile void* ptr = data;
    (void)id;
    (void)ptr;
}

static union Variant* create_variant(int type) {
    static union Variant v;
    v.as_int = type;
    return &v;
}

/* Function using complex types */
static void process_matrix(int_matrix m) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[i][j] = i * j;
        }
    }
}

/* Function with complex parameter types */
static int complex_operation(struct Container* c, 
                            union ComplexUnion* u,
                            processor_t proc) {
    if (c && u && proc) {
        return proc(c->inner.a, c->name);
    }
    return -1;
}

/* ========== Main Function ========== */
int main(void) {
    /* 1. Initialize structs and unions */
    struct Container local_container = {
        .inner = { .a = 1, .b = 'A', .c = 100 },
        .bits = { .flag1 = 1, .flag2 = 3, .value = -42, .reserved = 0xABCD },
        .anon_union = { .type = 2, .dbl_val = 3.14159, .tag = 'X' },
        .name = "TestContainer"
    };
    
    union ComplexUnion local_union;
    local_union.point.x = 10;
    local_union.point.y = 20;
    
    /* 2. Use arrays and pointers */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix_2d[i][j] = i + j;
        }
    }
    
    /* 3. Initialize and use function pointers */
    struct Plugin test_plugin = {
        .name = "TestPlugin",
        .version = 1,
        .init = plugin_init,
        .process = plugin_process,
        .cleanup = plugin_cleanup,
        .on_event = handle_event
    };
    
    plugin_registry[0] = test_plugin;
    
    /* Call through function pointer */
    int init_result = plugin_registry[0].init(NULL);
    plugin_registry[0].process(init_result);
    
    /* 4. Use GCC vector type */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = vec_a + vec_b;  /* Vector addition */
    
    /* Store to global to prevent optimization */
    global_vector = vec_sum;
    
    /* 5. Use multi-dimensional array through pointer */
    int (*ptr)[5] = matrix_2d;
    volatile int element = (*ptr)[2];  /* Access element */
    (void)element;
    
    /* 6. Process union type with runtime condition */
    volatile int condition = 1;
    if (condition) {
        local_union.color.r = 1.0f;
        local_union.color.g = 0.5f;
        local_union.color.b = 0.25f;
        local_union.color.a = 1.0f;
    } else {
        local_union.bits64 = 0xDEADBEEFCAFEBABEULL;
    }
    
    /* 7. Use complex callback */
    variant_factory factory = create_variant;
    union Variant* v = factory(42);
    volatile int variant_value = v->as_int;
    (void)variant_value;
    
    /* 8. Process matrices */
    process_matrix(matrices[0]);
    process_matrix(matrices[1]);
    
    /* 9. Complex operation with multiple types */
    processors[0] = NULL;  /* Placeholder - would be real function */
    int op_result = complex_operation(&local_container, 
                                     &local_union, 
                                     processors[0]);
    
    /* 10. Use all scalar types */
    char_var = 'Z';
    int_var = 1000;
    float_var = 2.71828f;
    double_var = 3.14159265358979;
    bool_var = 1;
    fcomplex_var = 1.0f + 2.0f * I;
    
    /* 11. String operations */
    string_ptr_array[0] = (char*)messages[0];
    string_ptr_array[1] = (char*)messages[1];
    
    /* Compute deterministic return value using all manipulated data */
    int hash = 0;
    hash ^= local_container.inner.a;
    hash ^= local_container.inner.b;
    hash ^= local_container.inner.c;
    hash ^= matrix_2d[0][0];
    hash ^= init_result;
    hash ^= (int)vec_sum[0];
    hash ^= op_result;
    hash ^= char_var;
    hash ^= int_var;
    hash ^= (int)(float_var * 1000);
    hash ^= (int)bool_var;
    
    /* Ensure all types are referenced to prevent dead code elimination */
    volatile struct opaque* opaque_ref = NULL;
    (void)opaque_ref;
    
    volatile ContainerArray* ca_ref = &global_container_array;
    (void)ca_ref;
    
    volatile struct PackedStruct packed = {0};
    (void)packed;
    
    volatile struct AlignedStruct aligned = {0};
    (void)aligned;
    
    return hash & 0xFF;  /* Return deterministic value based on all operations */
}

/* Additional functions to ensure type usage */
void __attribute__((noinline)) use_more_types(void) {
    /* Force usage of more types */
    static int*** local_triple_ptr = NULL;
    (void)local_triple_ptr;
    
    static union SimpleUnion simple = {0};
    (void)simple;
    
    static _Complex double cd = 1.0 + 2.0 * I;
    (void)cd;
}
