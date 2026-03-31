/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Forward declaration - TYPE_UNDEFINED */
typedef struct incomplete incomplete_t;  /* Another undefined type */

/* ========== TYPE_SCALAR - All fundamental scalar types ========== */
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
volatile float v_float = 3.1415926535f;
volatile double v_double = 2.718281828459045;
volatile long double v_ldouble = 1.6180339887498948482L;
volatile _Bool v_bool = 1;
volatile _Complex float v_cfloat = 1.0f + 2.0fi;
volatile _Complex double v_cdouble = 3.0 + 4.0i;
volatile _Complex long double v_cldouble = 5.0L + 6.0Li;

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
volatile const char* volatile_string = "Volatile string literal";
char mutable_string[] = "Mutable string";

/* ========== TYPE_CALLBACK - Function pointers and typedefs ========== */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(int, const char*, void*);
typedef void (*nested_callback)(complex_callback);

/* Callback function implementations */
static void callback_impl(void) { v_int++; }
static int complex_callback_impl(int x, const char* s, void* p) { 
    return x + (int)((char*)p - (char*)0); 
}

/* Struct with function pointer members - targets TYPE_CALLBACK through struct */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    nested_callback (*get_callback)(void);
    void (*cleanup)(struct Plugin*);
};

/* Union with function pointer */
union FunctionHolder {
    simple_callback simple;
    complex_callback complex;
    void* raw_ptr;
};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Simple struct */
struct Point {
    int x;
    int y;
    int z;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 24;
    unsigned int : 4;  /* Unnamed bit-field */
};

/* Struct with anonymous union */
struct WithAnonymousUnion {
    int type;
    union {
        int int_val;
        float float_val;
        void* ptr_val;
    };
    char tag;
};

/* Nested struct */
struct Outer {
    struct {
        int depth;
        struct Point coordinates;
    } inner;
    struct BitFieldStruct flags;
    char name[32];
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    double d;
};

/* Aligned struct */
struct __attribute__((aligned(64))) AlignedStruct {
    int data[16];
    struct Point* points;
};

/* Struct with flexible array member */
struct FlexArray {
    size_t length;
    int data[];
};

/* ========== TYPE_UNION ========== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Union with struct member */
union ComplexUnion {
    struct {
        short type;
        char data[8];
    } as_struct;
    long long as_llong;
    double as_double;
    struct Point* as_point;
};

/* Union with anonymous struct */
union WithAnonymousStruct {
    struct {
        int x, y;
    };
    long long position;
};

/* ========== TYPE_ARRAY ========== */
/* Multi-dimensional arrays */
int matrix_2d[10][20];
float matrix_3d[5][10][15];

/* Array of structs */
struct Point point_array[100];
struct Plugin plugin_registry[5];

/* Array of pointers */
int* ptr_array[50];
struct Point* point_ptr_array[25];

/* Array of arrays of pointers */
int* ptr_matrix[10][10];

/* Array with typedef */
typedef int vec3_t[3];
vec3_t vectors[100];

/* ========== TYPE_POINTER ========== */
/* Multi-level pointers */
int*** triple_ptr;
struct Point**** quad_point_ptr;

/* Pointer to array */
int (*ptr_to_array)[20];
float (*ptr_to_3d_array)[10][15];

/* Pointer to function pointer */
simple_callback* callback_ptr_array[10];

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */
/* GCC vector types */
typedef int __attribute__((vector_size(16))) int32x4_t;
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef double __attribute__((vector_size(64))) double64x8_t;

/* Transparent union attribute */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int i;
    long l;
    void* p;
} transparent_union_t;

/* Struct with cleanup attribute */
struct WithCleanup {
    FILE* file __attribute__((cleanup(fclose)));
    void* buffer;
};

/* ========== Complex Type Combinations ========== */
/* Struct containing all major type categories */
struct TypeKitchenSink {
    /* Scalar types */
    int id;
    double weight;
    _Complex float cf;
    
    /* String type */
    const char* name;
    
    /* Struct type */
    struct Point location;
    
    /* Union type */
    union {
        int int_option;
        float float_option;
    } option;
    
    /* Array type */
    int history[10];
    
    /* Pointer types */
    struct TypeKitchenSink* next;
    int* dynamic_data;
    
    /* Callback type */
    simple_callback on_event;
    
    /* Nested array of structs */
    struct Point trajectory[5];
    
    /* Pointer to array */
    float (*samples)[100];
    
    /* GCC vector type */
    int32x4_t vector_data;
    
    /* Flexible array member */
    size_t extra_count;
    struct Point extra_points[];
};

/* ========== Function Definitions ========== */
static int plugin_init_1(void) { return 0; }
static void plugin_process_1(int x) { v_int += x; }
static nested_callback plugin_get_callback_1(void) { return NULL; }
static void plugin_cleanup_1(struct Plugin* p) { p->name = NULL; }

static int plugin_init_2(void) { return 1; }
static void plugin_process_2(int x) { v_int -= x; }

/* Function that uses transparent union */
static void use_transparent_union(transparent_union_t arg) {
    v_int = (int)(long)arg.p;
}

/* Function with complex parameter types */
static void process_kitchen_sink(struct TypeKitchenSink* sink, 
                                complex_callback cb,
                                int32x4_t vec) {
    if (sink && sink->on_event) {
        sink->on_event();
    }
    if (cb) {
        cb(v_int, sink ? sink->name : "null", sink);
    }
    sink->vector_data = vec;
}

/* Function that manipulates multi-dimensional arrays */
static int sum_matrix(void) {
    int total = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            total += matrix_2d[i][j];
        }
    }
    return total;
}

/* Function using GCC vector types */
static int32x4_t add_vectors(int32x4_t a, int32x4_t b) {
    return a + b;
}

/* ========== Main Function ========== */
int main(void) {
    /* Initialize variables to ensure they're used */
    static int init_done = 0;
    if (init_done) return 0;
    
    /* 1. Initialize scalar types with operations */
    v_char++;
    v_int = v_short * 2;
    v_float = v_double * 2.0f;
    v_cfloat = v_cdouble * 1.5f;
    
    /* 2. Use string types */
    const char* msg = error_messages[v_int % 3];
    mutable_string[0] = msg[0];
    
    /* 3. Initialize and use structs */
    struct Point p1 = {.x = 10, .y = 20, .z = 30};
    struct Point p2;
    p2.x = p1.y;
    p2.y = p1.z;
    p2.z = p1.x;
    
    struct BitFieldStruct bf = {.flag1 = 1, .flag2 = 3, .flag3 = 7, .value = -100};
    
    struct WithAnonymousUnion au;
    au.type = 1;
    au.int_val = 42;
    au.tag = 'X';
    
    struct Outer outer;
    outer.inner.depth = 5;
    outer.inner.coordinates = p1;
    outer.flags = bf;
    outer.name[0] = 'T';
    outer.name[1] = 'e';
    outer.name[2] = 's';
    outer.name[3] = 't';
    outer.name[4] = '\0';
    
    /* 4. Initialize and use unions */
    union SimpleUnion su;
    su.as_int = 42;
    v_float = su.as_float;  /* Type punning */
    
    union ComplexUnion cu;
    cu.as_struct.type = 1;
    cu.as_struct.data[0] = 'A';
    cu.as_llong = 0xDEADBEEF;
    
    union WithAnonymousStruct was;
    was.x = 10;
    was.y = 20;
    v_llong = was.position;
    
    /* 5. Initialize arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix_2d[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].z = i * 3;
    }
    
    /* 6. Initialize pointers and pointer chains */
    int x = 42;
    int* px = &x;
    int** ppx = &px;
    triple_ptr = &ppx;
    
    struct Point* point_ptr = &p1;
    struct Point** pptr = &point_ptr;
    struct Point*** ppptr = &pptr;
    quad_point_ptr = &ppptr;
    
    ptr_to_array = &matrix_2d[0];
    
    /* 7. Initialize and use function pointers/callbacks */
    simple_callback cb = callback_impl;
    cb();
    
    complex_callback ccb = complex_callback_impl;
    int result = ccb(10, "test", &result);
    
    union FunctionHolder fh;
    fh.simple = cb;
    fh.complex = ccb;
    
    /* 8. Initialize plugin registry */
    plugin_registry[0].name = "Plugin1";
    plugin_registry[0].init = plugin_init_1;
    plugin_registry[0].process = plugin_process_1;
    plugin_registry[0].get_callback = plugin_get_callback_1;
    plugin_registry[0].cleanup = plugin_cleanup_1;
    
    plugin_registry[1].name = "Plugin2";
    plugin_registry[1].init = plugin_init_2;
    plugin_registry[1].process = plugin_process_2;
    plugin_registry[1].get_callback = NULL;
    plugin_registry[1].cleanup = NULL;
    
    /* Call plugin functions */
    for (int i = 0; i < 2; i++) {
        if (plugin_registry[i].init) {
            plugin_registry[i].init();
        }
        if (plugin_registry[i].process) {
            plugin_registry[i].process(i * 10);
        }
    }
    
    /* 9. Use GCC vector types */
    int32x4_t vec1 = {1, 2, 3, 4};
    int32x4_t vec2 = {5, 6, 7, 8};
    int32x4_t vec3 = add_vectors(vec1, vec2);
    
    float32x8_t fvec1, fvec2, fvec3;
    for (int i = 0; i < 8; i++) {
        ((float*)&fvec1)[i] = i * 1.5f;
        ((float*)&fvec2)[i] = i * 2.5f;
    }
    fvec3 = fvec1 + fvec2;
    
    /* 10. Use transparent union */
    transparent_union_t tu;
    tu.p = &p1;
    use_transparent_union(tu);
    
    /* 11. Create and use TypeKitchenSink */
    struct TypeKitchenSink* sink = (struct TypeKitchenSink*)malloc(
        sizeof(struct TypeKitchenSink) + 5 * sizeof(struct Point));
    if (sink) {
        sink->id = 1001;
        sink->weight = 3.14;
        sink->cf = 1.0 + 2.0i;
        sink->name = "TestSink";
        sink->location = p1;
        sink->option.int_option = 42;
        for (int i = 0; i < 10; i++) sink->history[i] = i * i;
        sink->next = NULL;
        sink->dynamic_data = (int*)malloc(50 * sizeof(int));
        if (sink->dynamic_data) {
            for (int i = 0; i < 50; i++) sink->dynamic_data[i] = i * 3;
        }
        sink->on_event = callback_impl;
        for (int i = 0; i < 5; i++) {
            sink->trajectory[i].x = i;
            sink->trajectory[i].y = i * 2;
            sink->trajectory[i].z = i * 3;
        }
        sink->samples = NULL;
        sink->vector_data = vec3;
        sink->extra_count = 5;
        for (int i = 0; i < 5; i++) {
            sink->extra_points[i].x = i * 10;
            sink->extra_points[i].y = i * 20;
            sink->extra_points[i].z = i * 30;
        }
        
        /* Process the kitchen sink */
        process_kitchen_sink(sink, ccb, vec3);
        
        free(sink->dynamic_data);
        free(sink);
    }
    
    /* 12. Perform operations with multi-dimensional arrays */
    int matrix_sum = sum_matrix();
    
    /* 13. Use volatile to prevent optimization */
    volatile int* volatile_ptr = &v_int;
    *volatile_ptr += matrix_sum;
    
    /* 14. Pointer arithmetic with different types */
    char* char_ptr = mutable_string;
    char_ptr += 3;
    
    struct Point* point_iter = point_array;
    point_iter += 10;
    
    /* 15. Compute deterministic return value */
    int hash = 0;
    hash += v_char;
    hash += v_int;
    hash += (int)v_float;
    hash += (int)v_double;
    hash += p1.x + p1.y + p1.z;
    hash += su.as_int;
    hash += matrix_sum % 1000;
    hash += result;
    for (int i = 0; i < 8; i++) {
        hash += ((float*)&fvec3)[i];
    }
    
    init_done = 1;
    return hash & 0xFF;  /* Return deterministic value based on all operations */
}

/* Additional global to ensure type visibility */
extern struct opaque* global_opaque_ptr;
struct TypeKitchenSink global_sink_instance;
