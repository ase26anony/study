/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declaration creates undefined type */
struct opaque;
struct incomplete;

/* GCC-specific vector type extension */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Packed and aligned structs for language-specific handling */
struct __attribute__((packed, aligned(2))) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(64))) AlignedStruct {
    double data[8];
};

/* ========== TYPE_SCALAR ========== */
/* Use all fundamental scalar types */
volatile char char_var = 'A';
volatile signed char schar_var = -1;
volatile unsigned char uchar_var = 255;
volatile short short_var = -32768;
volatile unsigned short ushort_var = 65535;
volatile int int_var = -2147483647-1;
volatile unsigned int uint_var = 4294967295U;
volatile long long_var = -9223372036854775807L-1;
volatile unsigned long ulong_var = 18446744073709551615UL;
volatile long long llong_var = -9223372036854775807LL-1;
volatile unsigned long long ullong_var = 18446744073709551615ULL;
volatile float float_var = 3.14159f;
volatile double double_var = 2.718281828459045;
volatile _Bool bool_var = 1;

/* Complex types */
volatile _Complex float cfloat_var = 1.0f + 2.0fi;
volatile _Complex double cdouble_var = 3.0 + 4.0i;

/* ========== TYPE_STRING ========== */
const char* global_string = "Global string literal";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable string";

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*simple_callback)(void);
typedef int (*processor_func)(int, float*, const char*);
typedef void (*event_handler)(int, void*);
typedef union Variant* (*variant_factory)(int type);

/* Struct with function pointer members */
struct Plugin {
    const char* name;
    int (*init)(struct Plugin*);
    void (*process)(int, float*);
    void (*cleanup)(void);
    event_handler on_event;
};

/* Function that takes callback */
static int process_with_callback(int* data, int len, processor_func proc) {
    float temp = 0.0f;
    return proc(len, &temp, "processing");
}

/* Actual functions to point to */
static int plugin_init(struct Plugin* p) {
    return p ? 0 : -1;
}

static void plugin_process(int id, float* data) {
    if (data) *data = (float)id;
}

static void plugin_cleanup(void) {
    /* Do nothing */
}

static void handle_event(int code, void* data) {
    *(int*)data = code;
}

/* ========== TYPE_UNION ========== */
/* Complex union types */
union DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_ptr;
    struct {
        short len;
        char buf[32];
    } as_string;
    long long as_llong;
};

/* Anonymous union inside struct */
struct WithAnonymousUnion {
    int type;
    union {
        int i;
        float f;
        char* str;
    };
    double extra;
};

/* Union with bitfields */
union BitfieldUnion {
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 12;
        unsigned int d : 8;
    } bits;
    unsigned int full;
};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Nested structure definitions */
struct Inner {
    int id;
    char tag;
    float values[3];
};

struct Middle {
    struct Inner inner;
    long counter;
    struct Inner* inner_ptr;
};

struct Outer {
    struct Middle middle[2];
    char name[64];
    struct Outer* self_ptr;
};

/* Struct with flexible array member */
struct FlexArray {
    size_t length;
    int data[];
};

/* Struct containing union */
struct Container {
    int type_id;
    union {
        struct {
            int x, y;
        } point;
        struct {
            float radius;
            int segments;
        } circle;
        struct {
            int width, height;
        } rect;
    } shape;
    struct Container* next;
};

/* Bitfield struct */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int : 4;  /* padding */
    unsigned int value : 10;
    unsigned int mode : 4;
};

/* ========== TYPE_ARRAY ========== */
/* Multi-dimensional arrays */
int matrix_2d[5][5];
float tensor_3d[3][4][2];
struct Inner struct_array[10];

/* Array of pointers */
int* ptr_array[20];
struct Container* container_ptrs[5];

/* Pointer to array */
int (*ptr_to_array)[10];

/* Array of function pointers */
processor_func func_array[4];

/* ========== TYPE_POINTER ========== */
/* Complex pointer chains */
int**** quad_ptr;
struct Outer*** outer_ptr_chain;
void (*complex_func_ptr)(int (**)(float), char*[]);

/* Pointer to flexible array member struct */
struct FlexArray* flex_ptr;

/* ========== Global instances ========== */
/* Ensure types are actually used */
struct Plugin plugin_registry[3];
union DataUnion global_union;
struct Outer global_outer;
struct WithAnonymousUnion global_anon_union;
struct BitfieldStruct global_bitfield;
struct BitfieldUnion global_bitfield_union;

/* Vector type usage */
float32x8_t global_vector1, global_vector2;
int32x4_t int_vector;

/* ========== Helper functions ========== */
static int sum_array(int arr[], int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

static void init_plugin_registry(void) {
    for (int i = 0; i < 3; i++) {
        plugin_registry[i].name = "Test Plugin";
        plugin_registry[i].init = plugin_init;
        plugin_registry[i].process = plugin_process;
        plugin_registry[i].cleanup = plugin_cleanup;
        plugin_registry[i].on_event = handle_event;
    }
}

static void process_matrix(void) {
    int val = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix_2d[i][j] = val++;
        }
    }
}

static void use_vectors(void) {
    /* Initialize vectors */
    for (int i = 0; i < 8; i++) {
        global_vector1[i] = (float)i;
        global_vector2[i] = (float)(i * 2);
    }
    
    /* Vector addition */
    float32x8_t result;
    for (int i = 0; i < 8; i++) {
        result[i] = global_vector1[i] + global_vector2[i];
    }
    
    /* Use int vector */
    for (int i = 0; i < 4; i++) {
        int_vector[i] = i * 10;
    }
}

static int traverse_pointer_chain(void) {
    /* Create a simple pointer chain */
    int*** ptr3 = NULL;
    int** ptr2 = NULL;
    int* ptr1 = NULL;
    int value = 42;
    
    ptr1 = &value;
    ptr2 = &ptr1;
    ptr3 = &ptr2;
    
    return ***ptr3;
}

static void process_union(union DataUnion* u, int type) {
    switch (type) {
        case 0:
            u->as_int = 100;
            break;
        case 1:
            u->as_float = 3.14f;
            break;
        case 2:
            strcpy(u->as_string.buf, "union string");
            u->as_string.len = (short)strlen(u->as_string.buf);
            break;
        default:
            u->as_ptr = (void*)0xDEADBEEF;
            break;
    }
}

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize struct and union types */
    init_plugin_registry();
    
    /* Initialize global struct */
    global_outer.middle[0].inner.id = 1;
    global_outer.middle[0].inner.tag = 'A';
    global_outer.middle[0].inner.values[0] = 1.0f;
    global_outer.self_ptr = &global_outer;
    
    /* Initialize anonymous union struct */
    global_anon_union.type = 1;
    global_anon_union.i = 42;
    global_anon_union.extra = 2.71828;
    
    /* Initialize bitfield struct */
    global_bitfield.flag1 = 1;
    global_bitfield.flag2 = 2;
    global_bitfield.flag3 = 5;
    global_bitfield.value = 512;
    global_bitfield.mode = 9;
    
    /* Initialize bitfield union */
    global_bitfield_union.bits.a = 5;
    global_bitfield_union.bits.b = 127;
    global_bitfield_union.bits.c = 2047;
    global_bitfield_union.bits.d = 42;
    
    /* 2. Use arrays and matrices */
    process_matrix();
    result += sum_array(&matrix_2d[0][0], 25);
    
    /* Initialize struct array */
    for (int i = 0; i < 10; i++) {
        struct_array[i].id = i;
        struct_array[i].tag = 'A' + (char)i;
        for (int j = 0; j < 3; j++) {
            struct_array[i].values[j] = (float)(i * 10 + j);
        }
    }
    
    /* 3. Use function pointers */
    if (plugin_registry[0].init(&plugin_registry[0]) == 0) {
        float data = 0.0f;
        plugin_registry[0].process(99, &data);
        result += (int)data;
        
        int event_data = 0;
        plugin_registry[0].on_event(100, &event_data);
        result += event_data;
    }
    
    /* 4. Use vector types */
    use_vectors();
    result += (int)global_vector1[0] + (int)global_vector2[0];
    
    /* 5. Traverse pointer chains */
    result += traverse_pointer_chain();
    
    /* 6. Process union types */
    process_union(&global_union, 0);
    result += global_union.as_int;
    
    process_union(&global_union, 1);
    result += (int)global_union.as_float;
    
    process_union(&global_union, 2);
    result += global_union.as_string.len;
    
    /* 7. Use string types */
    result += (int)strlen(global_string);
    result += (int)strlen(error_messages[0]);
    result += (int)strlen(mutable_string);
    
    /* 8. Use all scalar types */
    result += char_var + schar_var + uchar_var;
    result += short_var + ushort_var;
    result += (int)(float_var + double_var);
    result += bool_var;
    
    /* 9. Use complex types */
    result += (int)(__real__ cfloat_var + __imag__ cfloat_var);
    result += (int)(__real__ cdouble_var + __imag__ cdouble_var);
    
    /* 10. Use packed and aligned structs */
    struct PackedStruct packed = { 'X', 1234, 'Y' };
    result += packed.a + packed.b + packed.c;
    
    struct AlignedStruct aligned;
    for (int i = 0; i < 8; i++) {
        aligned.data[i] = (double)i;
        result += (int)aligned.data[i];
    }
    
    /* Create deterministic return value */
    return result % 256;  /* Ensure small, non-negative return */
}
