/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Incomplete/undefined type */
struct forward_declared;           /* Another forward declaration */

/* ========== TYPE_SCALAR - All fundamental scalar types ========== */
volatile char char_var = 'A';
volatile signed char schar_var = -1;
volatile unsigned char uchar_var = 255;
volatile short short_var = -32768;
volatile unsigned short ushort_var = 65535;
volatile int int_var = -2147483647 - 1;
volatile unsigned int uint_var = 4294967295U;
volatile long long_var = -9223372036854775807L - 1;
volatile unsigned long ulong_var = 18446744073709551615UL;
volatile long long llong_var = -9223372036854775807LL - 1;
volatile unsigned long long ullong_var = 18446744073709551615ULL;
volatile float float_var = 3.1415926535f;
volatile double double_var = 2.718281828459045;
volatile _Bool bool_var = 1;
volatile _Complex float complex_float = 1.0f + 2.0fi;
volatile _Complex double complex_double = 3.0 + 4.0i;

/* ========== TYPE_STRING ========== */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
volatile const char* static_string = "Hello, GCC GGC!";
char mutable_string[] = "Mutable string data";

/* ========== TYPE_CALLBACK - Function pointers and typedefs ========== */
typedef void (*simple_callback)(void);
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(int event_id, void* user_data);
typedef char* (*string_transform)(const char*, int);

/* Callback function definitions */
static void noop_callback(void) { /* Does nothing */ }
static int add_ints(int a, int b) { return a + b; }
static int multiply_ints(int a, int b) { return a * b; }
static void log_event(int event_id, void* user_data) {
    *(int*)user_data = event_id;
}
static char* duplicate_string(const char* str, int n) {
    static char buf[256];
    strncpy(buf, str, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    return buf;
}

/* Struct with function pointer members */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void* config);
    void (*process)(int data);
    void (*cleanup)(void);
    event_handler on_error;
};

/* Union with function pointer */
union FunctionHolder {
    simple_callback cb_simple;
    binary_op cb_binary;
    void* (*cb_generic)(void*);
};

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Simple nested struct */
struct InnerData {
    int counter;
    char tag;
    float precision;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
    unsigned int : 16;  /* Padding */
};

/* Struct with anonymous union */
struct WithAnonymousUnion {
    int type;
    union {
        int int_val;
        float float_val;
        void* ptr_val;
    };
    char description[32];
};

/* Complex nested struct */
struct Container {
    struct {
        int id;
        char name[16];
        struct InnerData* data_ptr;
    } metadata;
    
    union {
        long as_long;
        double as_double;
        struct {
            short length;
            unsigned char bytes[8];
        } as_bytes;
    } payload;
    
    struct BitFieldStruct flags;
    int array[4];
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t capacity;
    size_t length;
    int elements[];
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    char d;
};

/* Aligned struct */
struct __attribute__((aligned(64))) AlignedStruct {
    double data[8];
    int index;
};

/* ========== TYPE_UNION ========== */
union Variant {
    int as_int;
    float as_float;
    double as_double;
    void* as_ptr;
    struct {
        short type;
        char data[10];
    } as_struct;
    long long as_llong;
};

/* Tagged union */
struct TaggedVariant {
    enum { INT, FLOAT, STRING, ARRAY } tag;
    union {
        int int_val;
        float float_val;
        char* string_val;
        int array_val[4];
    } value;
};

/* ========== TYPE_ARRAY - Complex arrays ========== */
/* Multi-dimensional array of structs */
struct Container container_matrix[3][3];

/* Array of pointers to unions */
union Variant* variant_array[10];

/* Array of function pointers */
binary_op operation_array[] = {add_ints, multiply_ints, NULL};

/* Pointer to array */
int (*array_ptr)[4];

/* Complex nested array */
int nested_array[2][3][4][5];

/* ========== TYPE_POINTER - Complex pointer chains ========== */
int*** triple_ptr;
struct Container**** container_ptr_chain;
void (*function_ptr_array[5])(void);

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Transparent union attribute */
union __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    long* long_ptr;
    void* void_ptr;
};

/* ========== Global variables using complex types ========== */
struct Plugin plugin_registry[3];
union Variant global_variant;
float32x8_t global_vector;
struct Container global_container;
struct TaggedVariant global_tagged;

/* ========== Function definitions using complex types ========== */
static void initialize_plugin_registry(void) {
    for (int i = 0; i < 3; i++) {
        plugin_registry[i].name = "Test Plugin";
        plugin_registry[i].version = i + 1;
        plugin_registry[i].init = NULL;
        plugin_registry[i].process = NULL;
        plugin_registry[i].cleanup = noop_callback;
        plugin_registry[i].on_error = log_event;
    }
}

static void process_container_matrix(void) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            container_matrix[i][j].metadata.id = i * 10 + j;
            container_matrix[i][j].metadata.data_ptr = NULL;
            container_matrix[i][j].payload.as_long = i + j;
            container_matrix[i][j].flags.flag1 = (i + j) % 2;
            container_matrix[i][j].flags.flag2 = (i + j) % 8;
            
            for (int k = 0; k < 4; k++) {
                container_matrix[i][j].array[k] = i * 100 + j * 10 + k;
            }
        }
    }
}

static void demonstrate_vector_ops(void) {
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f};
    global_vector = vec_a + vec_b;
}

static int traverse_pointer_chain(void) {
    int value = 42;
    int* ptr1 = &value;
    int** ptr2 = &ptr1;
    triple_ptr = &ptr2;
    
    return ***triple_ptr;
}

static void process_variant(union Variant* v, int type) {
    switch (type % 4) {
        case 0:
            v->as_int = 100;
            break;
        case 1:
            v->as_float = 3.14f;
            break;
        case 2:
            v->as_double = 2.71828;
            break;
        case 3:
            v->as_ptr = &global_container;
            break;
    }
}

static void use_function_pointers(void) {
    /* Call through function pointer array */
    for (int i = 0; i < 2; i++) {
        if (operation_array[i]) {
            int result = operation_array[i](10, 20);
            (void)result; /* Use result to prevent dead code elimination */
        }
    }
    
    /* Call through struct member */
    int event_data = 0;
    if (plugin_registry[0].on_error) {
        plugin_registry[0].on_error(42, &event_data);
    }
}

/* ========== Main function ========== */
int main(void) {
    int hash = 0;
    
    /* 1. Initialize complex structs and unions */
    initialize_plugin_registry();
    
    /* 2. Process multi-dimensional array of structs */
    process_container_matrix();
    
    /* 3. Use GCC vector types */
    demonstrate_vector_ops();
    
    /* 4. Traverse pointer chains */
    hash += traverse_pointer_chain();
    
    /* 5. Process union types with runtime conditions */
    for (int i = 0; i < 5; i++) {
        process_variant(&global_variant, i);
        hash += (int)global_variant.as_int;
    }
    
    /* 6. Use function pointers and callbacks */
    use_function_pointers();
    
    /* 7. Use all scalar types in computation */
    hash += char_var + schar_var + uchar_var;
    hash += short_var + ushort_var;
    hash += (int)(float_var * 100) + (int)(double_var * 100);
    hash += bool_var ? 1 : 0;
    
    /* 8. Process string types */
    for (int i = 0; error_messages[i] != NULL; i++) {
        hash += (int)error_messages[i][0];
    }
    hash += (int)static_string[0];
    hash += (int)mutable_string[0];
    
    /* 9. Use arrays and pointers */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                nested_array[i][j][k][0] = i + j + k;
                hash += nested_array[i][j][k][0];
            }
        }
    }
    
    /* 10. Use opaque/forward declared types (through pointers) */
    struct opaque* opaque_ptr = NULL;
    struct forward_declared* forward_ptr = NULL;
    (void)opaque_ptr;
    (void)forward_ptr;
    
    /* 11. Use packed and aligned structs */
    struct PackedStruct packed = { 'a', 42, 100, 'z' };
    struct AlignedStruct aligned;
    for (int i = 0; i < 8; i++) {
        aligned.data[i] = i * 1.5;
    }
    aligned.index = hash;
    
    hash += packed.a + packed.b + packed.c + packed.d;
    hash += (int)aligned.data[0] + aligned.index;
    
    /* 12. Use transparent union */
    union TransparentUnion tu;
    int x = 99;
    tu.int_ptr = &x;
    hash += *tu.int_ptr;
    
    /* Return deterministic value based on all operations */
    return hash % 256;
}
