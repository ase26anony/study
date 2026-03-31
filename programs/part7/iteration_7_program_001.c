/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
/* This program defines a diverse set of type constructs to exercise all */
/* type categories in GCC's GGC type state serialization. */

#include <stddef.h>
#include <string.h>

/* ==================== TYPE_UNDEFINED / TYPE_LANG_STRUCT ==================== */

/* Forward declaration for undefined/incomplete type */
struct opaque;                    /* TYPE_UNDEFINED candidate */

/* GCC-specific vector type extension */
typedef float __attribute__((vector_size(32))) float32x8_t; /* TYPE_LANG_STRUCT */

/* ==================== TYPE_STRUCT / TYPE_USER_STRUCT ==================== */

/* Complex nested structure with bit-fields and anonymous union */
struct __attribute__((packed)) PacketHeader {
    unsigned short version : 4;
    unsigned short flags   : 12;
    union {
        struct {
            unsigned int seq_num;
            unsigned short checksum;
        } tcp;
        struct {
            unsigned char ttl;
            unsigned char protocol;
        } ip;
    } proto;
    char payload[0]; /* Flexible array member */
};

/* User-defined structure with function pointer member */
struct Plugin {
    const char* name;
    int priority;
    int (*init)(void* context);          /* TYPE_CALLBACK */
    void (*process)(int data, void* out); /* TYPE_CALLBACK */
    struct Plugin* next;                 /* TYPE_POINTER */
};

/* Structure containing array of pointers */
struct Graph {
    int vertex_count;
    struct Node** adjacency_list;        /* TYPE_POINTER -> TYPE_ARRAY */
};

/* ==================== TYPE_UNION ==================== */

/* Complex union with nested struct and flexible array member */
union Variant {
    long as_int;
    double as_float;
    void* as_ptr;                        /* TYPE_POINTER */
    struct {
        unsigned short length;
        unsigned short capacity;
        char data[];                     /* Flexible array */
    } as_string;
    float32x8_t as_vector;               /* TYPE_LANG_STRUCT */
};

/* ==================== TYPE_CALLBACK ==================== */

/* Typedef for complex function pointer */
typedef int (*binary_op)(int, int);      /* TYPE_CALLBACK */

/* Function pointer with array parameter */
typedef void (*signal_handler)(float[][256], int**);

/* ==================== TYPE_ARRAY / TYPE_POINTER ==================== */

/* Multi-dimensional array of struct pointers */
struct Node* routing_table[8][8];        /* TYPE_ARRAY of TYPE_POINTER */

/* Triple pointer chain */
int*** deep_pointer_chain;               /* TYPE_POINTER chain */

/* Array of unions */
union Variant variant_array[16];

/* ==================== TYPE_SCALAR ==================== */

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
_Complex float cfloat_var;
_Complex double cdouble_var;

/* ==================== TYPE_STRING ==================== */

/* String literals and arrays */
const char* error_messages[] = {         /* TYPE_ARRAY of TYPE_POINTER to TYPE_STRING */
    "Fatal error",
    "Warning",
    "Information",
    NULL
};

char static_string[] = "Static buffer";  /* TYPE_ARRAY of TYPE_SCALAR */

/* ==================== FUNCTION DEFINITIONS ==================== */

/* Callback function implementations */
int add_callback(int a, int b) {
    return a + b;
}

int multiply_callback(int a, int b) {
    return a * b;
}

int plugin_init(void* context) {
    *(int*)context = 42;
    return 0;
}

void plugin_process(int data, void* out) {
    *(int*)out = data * 2;
}

/* Function using vector type */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex parameter types */
void process_signal(float spectrum[][256], int** coefficients, 
                    signal_handler handler) {
    if (handler)
        handler(spectrum, coefficients);
}

/* Function demonstrating union usage */
void process_variant(union Variant* v, int type) {
    volatile int prevent_opt = 0; /* Prevent optimization */
    
    switch (type) {
        case 0:
            v->as_int = 100;
            prevent_opt = v->as_int;
            break;
        case 1:
            v->as_float = 3.14159;
            prevent_opt = (int)v->as_float;
            break;
        case 2:
            v->as_ptr = &prevent_opt;
            prevent_opt = *(int*)v->as_ptr;
            break;
        case 3:
            /* Vector operation */
            v->as_vector = vector_add(v->as_vector, v->as_vector);
            prevent_opt = (int)v->as_vector[0];
            break;
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    /* 1. Initialize structs and unions */
    struct PacketHeader pkt = {
        .version = 1,
        .flags = 0xABC,
        .proto = { .tcp = { .seq_num = 1000, .checksum = 0xDEAD } }
    };
    
    union Variant vars[4];
    for (int i = 0; i < 4; i++) {
        vars[i].as_int = i * 100;
    }
    
    /* 2. Array of structs with function pointers */
    struct Plugin plugins[] = {
        { "adder", 1, plugin_init, plugin_process, NULL },
        { "multiplier", 2, plugin_init, NULL, NULL }
    };
    
    int context = 0;
    int result = 0;
    
    /* Call through function pointer */
    if (plugins[0].init) {
        plugins[0].init(&context);
    }
    if (plugins[0].process) {
        plugins[0].process(context, &result);
    }
    
    /* 3. Use GCC vector type */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_c = vector_add(vec_a, vec_b);
    volatile float vec_sum = vec_c[0] + vec_c[7]; /* Prevent optimization */
    
    /* 4. Multi-dimensional array and pointer chain */
    int*** triple_ptr = &deep_pointer_chain;
    int value = 123;
    int* single_ptr = &value;
    int** double_ptr = &single_ptr;
    deep_pointer_chain = &double_ptr;
    
    /* Access through triple pointer */
    if (deep_pointer_chain && *deep_pointer_chain && **deep_pointer_chain) {
        result += ***deep_pointer_chain;
    }
    
    /* 5. Process union types */
    for (int i = 0; i < 4; i++) {
        process_variant(&vars[i], i % 4);
    }
    
    /* 6. Use binary operation callbacks */
    binary_op operations[] = { add_callback, multiply_callback };
    int op_result = 0;
    for (int i = 0; i < 2; i++) {
        op_result += operations[i](10, 20);
    }
    
    /* 7. String operations */
    const char** msg_ptr = error_messages;
    int msg_len = 0;
    while (*msg_ptr) {
        msg_len += strlen(*msg_ptr);
        msg_ptr++;
    }
    
    /* 8. Complex array operations */
    float spectrum[4][256];
    int* coeffs[8];
    int coeff_data[8][16];
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 256; j++) {
            spectrum[i][j] = (float)(i * j);
        }
    }
    
    for (int i = 0; i < 8; i++) {
        coeffs[i] = coeff_data[i];
        for (int j = 0; j < 16; j++) {
            coeff_data[i][j] = i * 100 + j;
        }
    }
    
    /* Compute deterministic return value using all type interactions */
    int final_result = 
        result + 
        op_result + 
        msg_len + 
        (int)vec_sum + 
        pkt.proto.tcp.seq_num + 
        (int)vars[0].as_int +
        (int)spectrum[2][128] +
        coeff_data[3][5];
    
    /* Ensure all variables are marked as used */
    (void)char_var; (void)scha_var; (void)uchar_var;
    (void)short_var; (void)ushort_var; (void)int_var;
    (void)uint_var; (void)long_var; (void)ulong_var;
    (void)llong_var; (void)ullong_var; (void)float_var;
    (void)double_var; (void)ldouble_var; (void)bool_var;
    (void)cfloat_var; (void)cdouble_var;
    (void)static_string;
    
    return final_result % 255; /* Return deterministic value */
}

/* Additional global declarations for type visibility */
struct Plugin plugin_registry[3];
union Variant global_variant;
float32x8_t global_vector;
