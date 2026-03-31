/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED target */
struct incomplete;                 /* Another undefined type */
typedef struct opaque* opaque_ptr; /* Pointer to undefined type */

/* ========== TYPE_SCALAR - All fundamental types ========== */
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
volatile float v_float = 3.1415926535f;
volatile double v_double = 2.718281828459045;
volatile long double v_ldouble = 1.6180339887498948482L;
volatile _Bool v_bool = 1;
volatile _Complex float v_cfloat = 1.0f + 2.0fi;
volatile _Complex double v_cdouble = 3.0 + 4.0i;
volatile _Complex long double v_cldouble = 5.0L + 6.0Li;

/* ========== TYPE_STRING ========== */
const char* global_string = "Global string literal";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable string";

/* ========== TYPE_CALLBACK - Function pointers ========== */
typedef void (*simple_callback)(void);                     /* Simple callback */
typedef int (*processor_cb)(int, const char*, void*);      /* Complex callback */
typedef void (*event_handler)(int, void*);                 /* Event handler */
typedef struct callback_chain* (*factory_cb)(int);         /* Factory callback */

/* Callback function definitions */
static void callback_impl(void) {
    v_int++;
}

static int processor_impl(int x, const char* s, void* data) {
    return x + (int)strlen(s) + *(int*)data;
}

static void event_handler_impl(int event, void* context) {
    *(int*)context = event * 2;
}

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */

/* Simple struct with basic types */
struct Point {
    int x;
    int y;
    int z;
} __attribute__((packed));

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 24;
    unsigned int : 4;  /* Unnamed bit-field */
} __attribute__((aligned(8)));

/* Nested anonymous struct */
struct Container {
    struct {
        int id;
        char tag;
    } __attribute__((packed));  /* Anonymous struct */
    
    union {
        long counter;
        double precision;
        void* pointer;
    } data;
    
    struct Point position;
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t length;
    size_t capacity;
    int data[];
};

/* Struct with function pointer members */
struct Plugin {
    const char* name;
    int version;
    int (*init)(struct Plugin*);
    void (*process)(int);
    void (*cleanup)(void);
    processor_cb custom_processor;
};

/* Complex nested struct */
struct TreeNode {
    int value;
    struct TreeNode* left;
    struct TreeNode* right;
    struct TreeNode* parent;
    void (*visit)(struct TreeNode*);
};

/* ========== TYPE_UNION ========== */

/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Union with struct members */
union Variant {
    int type_id;
    struct {
        short length;
        char buffer[64];
    } as_string;
    struct {
        double real;
        double imag;
    } as_complex;
    struct Point as_point;
    void (*as_func)(void);
};

/* Tagged union */
struct TaggedVariant {
    enum { INT, FLOAT, STRING, POINTER } tag;
    union {
        int i;
        float f;
        char* s;
        void* p;
    } value;
};

/* ========== TYPE_ARRAY ========== */

/* Multi-dimensional arrays */
int matrix_2d[10][10];
int matrix_3d[5][5][5];

/* Array of structs */
struct Point point_array[100];
struct Plugin plugin_registry[5];

/* Array of pointers */
struct TreeNode* node_ptr_array[50];
int* int_ptr_array[20];

/* Array of arrays */
char string_table[10][256];

/* ========== TYPE_POINTER ========== */

/* Multi-level pointers */
int*** triple_ptr;
struct Container**** quad_container_ptr;

/* Function pointer arrays */
event_handler event_handlers[10];
simple_callback callbacks[5];

/* Pointer to array */
int (*ptr_to_array)[10];

/* Pointer to function returning pointer */
struct Plugin* (*plugin_factory)(const char*);

/* ========== TYPE_LANG_STRUCT - GCC extensions ========== */

/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned types */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
};

/* Transparent union */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int i;
    long l;
} TransparentUnion;

/* ========== Function Definitions ========== */

/* Function using all type categories */
static int process_plugin(struct Plugin* plugin, int input) {
    if (!plugin || !plugin->process) return -1;
    
    plugin->process(input);
    
    if (plugin->custom_processor) {
        int local_data = 42;
        return plugin->custom_processor(input, plugin->name, &local_data);
    }
    
    return 0;
}

/* Function with complex parameters */
static void traverse_tree(struct TreeNode* root, 
                         void (*action)(struct TreeNode*)) {
    if (!root) return;
    
    if (action) action(root);
    
    traverse_tree(root->left, action);
    traverse_tree(root->right, action);
}

/* Function using vector types */
static float32x8_t add_vectors(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function processing unions */
static void process_variant(union Variant* var, int mode) {
    switch (mode) {
        case 0:
            var->as_string.length = 10;
            strncpy(var->as_string.buffer, "test", 64);
            break;
        case 1:
            var->as_complex.real = 1.0;
            var->as_complex.imag = 2.0;
            break;
        case 2:
            var->as_point.x = 100;
            var->as_point.y = 200;
            var->as_point.z = 300;
            break;
        default:
            var->type_id = -1;
    }
}

/* ========== Global Variables ========== */

/* Ensure all types are used globally */
struct Container global_container = {
    {1, 'A'},
    {.counter = 1000},
    {10, 20, 30}
};

union Variant global_variant;
struct TaggedVariant global_tagged = {INT, {.i = 42}};

float32x8_t global_vector1, global_vector2;
int32x4_t global_int_vector;

struct CacheLine global_cacheline;

/* Array of various pointers */
void* void_ptr_array[] = {
    &v_int,
    &global_string,
    &global_container,
    &global_variant,
    matrix_2d,
    point_array,
    (void*)callback_impl
};

/* ========== Main Function ========== */

int main(void) {
    int result = 0;
    
    /* 1. Initialize and use struct types */
    struct Point p1 = {1, 2, 3};
    struct Point p2 = {4, 5, 6};
    
    point_array[0] = p1;
    point_array[1] = p2;
    
    for (int i = 0; i < 2; i++) {
        result += point_array[i].x + point_array[i].y + point_array[i].z;
    }
    
    /* 2. Initialize bit-field struct */
    struct BitFieldStruct bfs = {1, 7, 15, -8388608};
    result += bfs.flag1 + bfs.flag2 + bfs.flag3 + bfs.value;
    
    /* 3. Use union types */
    union SimpleUnion su;
    su.as_int = 0xDEADBEEF;
    result += su.as_int;
    
    process_variant(&global_variant, 0);
    result += global_variant.as_string.length;
    
    /* 4. Initialize and call through function pointers */
    struct Plugin my_plugin = {
        .name = "TestPlugin",
        .version = 1,
        .init = NULL,
        .process = NULL,
        .cleanup = NULL,
        .custom_processor = processor_impl
    };
    
    int local_state = 100;
    if (my_plugin.custom_processor) {
        result += my_plugin.custom_processor(5, "test", &local_state);
    }
    
    /* 5. Use arrays and pointers */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix_2d[i][j] = i * j;
            result += matrix_2d[i][j];
        }
    }
    
    /* 6. Pointer chains */
    int x = 42;
    int* px = &x;
    int** ppx = &px;
    int*** pppx = &ppx;
    
    result += ***pppx;
    
    /* 7. Use vector types (GCC extension) */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = add_vectors(vec1, vec2);
    
    /* Access vector elements through union for result */
    union {
        float32x8_t vec;
        float array[8];
    } vec_union;
    vec_union.vec = vec_sum;
    
    for (int i = 0; i < 8; i++) {
        result += (int)vec_union.array[i];
    }
    
    /* 8. Use string types */
    const char* local_str = "Local string";
    result += (int)strlen(local_str);
    result += (int)strlen(global_string);
    
    /* 9. Process all scalar types */
    result += v_char + v_short + v_int + (int)v_float;
    result += (int)v_bool;
    
    /* 10. Use opaque pointer type */
    opaque_ptr unknown_ptr = NULL;
    (void)unknown_ptr;  /* Suppress unused warning */
    
    /* 11. Initialize and use callback array */
    callbacks[0] = callback_impl;
    if (callbacks[0]) {
        callbacks[0]();
    }
    
    /* 12. Use multi-dimensional pointer array */
    struct Node {
        int value;
        struct Node* next;
    };
    
    struct Node* node_grid[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            node_grid[i][j] = NULL;
        }
    }
    
    /* 13. Use transparent union */
    TransparentUnion tu;
    tu.i = 12345;
    result += tu.i;
    
    /* 14. Ensure all global arrays are touched */
    for (int i = 0; i < 10; i++) {
        strcpy(string_table[i], "row");
        char num[10];
        sprintf(num, "%d", i);
        strcat(string_table[i], num);
        result += (int)strlen(string_table[i]);
    }
    
    /* 15. Use all elements of void pointer array */
    for (size_t i = 0; i < sizeof(void_ptr_array)/sizeof(void_ptr_array[0]); i++) {
        if (void_ptr_array[i]) {
            result += 1;
        }
    }
    
    /* Final deterministic result */
    printf("Result: %d\n", result);
    
    /* Return value based on all operations */
    return result % 256;
}
