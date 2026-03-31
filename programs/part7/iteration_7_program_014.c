/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
/* This program defines a diverse set of type constructs to ensure all */
/* type categories in GCC's GGC type state serialization are exercised. */

#include <stdio.h>
#include <string.h>
#include <complex.h>

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations for incomplete/undefined types */
struct opaque;              /* TYPE_UNDEFINED candidate */
typedef struct incomplete incomplete_t;

/* ==================== TYPE_SCALAR ==================== */
/* Use all fundamental scalar types */
static char char_var = 'A';
static signed char schar_var = -1;
static unsigned char uchar_var = 255;
static short short_var = -32768;
static unsigned short ushort_var = 65535;
static int int_var = -2147483647-1;
static unsigned int uint_var = 4294967295U;
static long long_var = -9223372036854775807L-1;
static unsigned long ulong_var = 18446744073709551615UL;
static long long llong_var = -9223372036854775807LL-1;
static unsigned long long ullong_var = 18446744073709551615ULL;
static float float_var = 3.14159f;
static double double_var = 2.718281828459045;
static _Bool bool_var = 1;
static _Complex float cfloat_var = 1.0f + 2.0f * I;
static _Complex double cdouble_var = 3.0 + 4.0 * I;

/* ==================== TYPE_STRING ==================== */
/* String literals and pointers */
const char* global_string = "Global string literal";
static const char* string_array[] = {
    "Error",
    "Warning", 
    "Info",
    "Debug",
    "Trace"
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs and variables */
typedef void (*simple_callback)(void);
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(int event_id, void* user_data);
typedef char* (*string_transformer)(const char*, int);

/* Callback implementations */
static void null_callback(void) { /* Does nothing */ }
static int add_ints(int a, int b) { return a + b; }
static int multiply_ints(int a, int b) { return a * b; }
static void log_event(int event_id, void* user_data) {
    printf("Event %d logged\n", event_id);
}
static char* duplicate_string(const char* str, int times) {
    static char buffer[256];
    buffer[0] = '\0';
    for (int i = 0; i < times && strlen(buffer) < 255; i++) {
        strcat(buffer, str);
    }
    return buffer;
}

/* Global callback variables */
simple_callback global_cb = null_callback;
binary_op arithmetic_ops[3] = {add_ints, multiply_ints, NULL};

/* ==================== TYPE_STRUCT ==================== */
/* Complex nested structures with various members */
struct Inner {
    int a;
    char b;
    float c;
    void* ptr;
};

struct Container {
    struct {
        int counter;
        char tag;
    } header __attribute__((packed));
    
    union {
        long as_long;
        double as_double;
        void* as_pointer;
    } data;
    
    struct Inner inner;
    
    /* Bit-fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* Padding */
    unsigned int value : 8;
    
    /* Flexible array member */
    int flexible_array[];
};

/* Anonymous struct/union */
struct AnonymousExample {
    struct {
        int x;
        int y;
    };  /* Anonymous struct */
    
    union {
        int id;
        void* handle;
    };  /* Anonymous union */
    
    char name[32];
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct with function pointers */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void* context);
    void (*process)(int data);
    void (*cleanup)(void);
    event_handler on_error;
};

struct Node {
    int id;
    char* label;
    struct Node** children;  /* Array of pointers */
    int child_count;
    struct Node* next;
};

/* ==================== TYPE_UNION ==================== */
union Variant {
    int as_int;
    long as_long;
    float as_float;
    double as_double;
    void* as_pointer;
    struct {
        short length;
        char data[64];
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

union BitFieldUnion {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits;
    unsigned int full;
};

/* ==================== TYPE_ARRAY ==================== */
/* Complex array types */
int multi_dim_array[3][4][5];
struct Container* array_of_structs[10];
union Variant variant_array[20];
int* array_of_pointers[15];
int (*array_of_callbacks[5])(void);

/* Pointer to array */
int (*ptr_to_array)[10];

/* Array of pointers to functions */
string_transformer transformers[] = {
    duplicate_string,
    NULL
};

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer chains */
int**** quadruple_ptr;
struct Node*** node_matrix[5][5];
void (*complex_callback_array[2][3])(int, float, double);

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC-specific type extensions */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

struct __attribute__((aligned(64))) CacheAligned {
    int data[16];
    float values[8];
};

struct __attribute__((packed)) TightPacked {
    char a;
    int b;
    short c;
    double d;
};

/* ==================== Global Variables ==================== */
/* Ensure all types are used and visible */
struct Plugin plugin_registry[3];
struct Container global_container;
union Variant global_variant;
struct AnonymousExample global_anonymous;
float32x8_t global_vector;
struct CacheAligned global_aligned;
struct TightPacked global_packed;

/* ==================== Function Definitions ==================== */
/* Functions that use all the complex types */

static void initialize_plugin_registry(void) {
    /* Plugin 1 */
    plugin_registry[0].name = "Logger";
    plugin_registry[0].version = 1;
    plugin_registry[0].init = NULL;
    plugin_registry[0].process = NULL;
    plugin_registry[0].cleanup = NULL;
    plugin_registry[0].on_error = log_event;
    
    /* Plugin 2 */
    plugin_registry[1].name = "Processor";
    plugin_registry[1].version = 2;
    plugin_registry[1].init = NULL;
    plugin_registry[1].process = NULL;
    plugin_registry[1].cleanup = null_callback;
    plugin_registry[1].on_error = NULL;
    
    /* Plugin 3 */
    plugin_registry[2].name = "Transformer";
    plugin_registry[2].version = 3;
    plugin_registry[2].init = NULL;
    plugin_registry[2].process = NULL;
    plugin_registry[2].cleanup = NULL;
    plugin_registry[2].on_error = NULL;
}

static void process_variant(union Variant* v, int type) {
    volatile union Variant* volatile_v = v;  /* Prevent optimization */
    
    switch (type) {
        case 0:
            volatile_v->as_int = 42;
            break;
        case 1:
            volatile_v->as_double = 3.14159;
            break;
        case 2:
            strncpy(volatile_v->as_string.data, "Hello", 63);
            volatile_v->as_string.length = 5;
            break;
        default:
            volatile_v->as_pointer = &global_container;
    }
}

static int traverse_pointer_chain(void) {
    int value = 42;
    int* p1 = &value;
    int** p2 = &p1;
    int*** p3 = &p2;
    int**** p4 = &p3;
    
    quadruple_ptr = p4;
    
    /* Chain dereference */
    return ****quadruple_ptr;
}

static void use_gcc_vector_types(void) {
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t result;
    
    /* Simulate addition - actual vector operations depend on target */
    volatile float32x8_t* v1 = &vec1;
    volatile float32x8_t* v2 = &vec2;
    volatile float32x8_t* r = &result;
    
    /* Prevent unused variable warnings */
    (void)v1;
    (void)v2;
    (void)r;
    
    global_vector = vec1;
}

static void populate_multi_dim_array(void) {
    volatile int counter = 0;
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                multi_dim_array[i][j][k] = counter++;
            }
        }
    }
    
    /* Pointer to array */
    ptr_to_array = &multi_dim_array[0][0];
}

static int call_through_function_pointer(void) {
    int result = 0;
    
    /* Call through function pointer in struct */
    if (arithmetic_ops[0]) {
        result = arithmetic_ops[0](10, 20);
    }
    
    /* Call through array of function pointers */
    if (transformers[0]) {
        char* transformed = transformers[0]("test", 2);
        result += (int)strlen(transformed);
    }
    
    return result;
}

static void use_anonymous_structs(void) {
    global_anonymous.x = 100;
    global_anonymous.y = 200;
    global_anonymous.id = 42;
    strcpy(global_anonymous.name, "AnonymousExample");
}

static void initialize_global_container(void) {
    global_container.header.counter = 1;
    global_container.header.tag = 'C';
    global_container.data.as_double = 2.71828;
    global_container.inner.a = 10;
    global_container.inner.b = 'X';
    global_container.inner.c = 1.234f;
    global_container.inner.ptr = &global_variant;
    global_container.flag1 = 1;
    global_container.flag2 = 3;
    global_container.value = 127;
    
    /* Note: flexible_array not initialized as size unknown */
}

/* ==================== Main Function ==================== */
int main(void) {
    volatile int checksum = 0;  /* Prevent optimization */
    
    /* 1. Initialize and use structs and unions */
    initialize_global_container();
    use_anonymous_structs();
    
    /* 2. Process union with different type accesses */
    process_variant(&global_variant, 0);
    process_variant(&global_variant, 1);
    process_variant(&global_variant, 2);
    
    /* 3. Populate plugin registry and use function pointers */
    initialize_plugin_registry();
    checksum += call_through_function_pointer();
    
    /* 4. Use GCC vector types */
    use_gcc_vector_types();
    
    /* 5. Traverse pointer chains */
    checksum += traverse_pointer_chain();
    
    /* 6. Populate multi-dimensional arrays */
    populate_multi_dim_array();
    
    /* 7. Use all scalar types in computation */
    checksum += char_var + schar_var + uchar_var;
    checksum += short_var + ushort_var;
    checksum += int_var % 1000;
    checksum += uint_var % 1000;
    checksum += (int)float_var;
    checksum += (int)double_var;
    checksum += bool_var;
    
    /* 8. Use string types */
    checksum += (int)strlen(global_string);
    for (int i = 0; i < 5; i++) {
        checksum += (int)strlen(string_array[i]);
    }
    
    /* 9. Use aligned and packed structs */
    global_aligned.data[0] = 42;
    global_packed.a = 'Z';
    global_packed.b = 1000;
    global_packed.c = 50;
    global_packed.d = 3.14;
    
    checksum += global_aligned.data[0];
    checksum += global_packed.b;
    
    /* 10. Use complex and volatile to ensure all types are live */
    volatile _Complex float cv = cfloat_var;
    volatile _Complex double cd = cdouble_var;
    checksum += (int)creal(cv) + (int)cimag(cv);
    
    /* Return deterministic checksum */
    printf("Final checksum: %d\n", checksum);
    return checksum % 256;
}
