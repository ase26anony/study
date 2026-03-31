/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ==================== TYPE_UNDEFINED ==================== */
struct opaque;  /* Forward declaration - undefined type */
extern struct opaque *global_opaque;

/* ==================== TYPE_STRUCT & TYPE_USER_STRUCT ==================== */

/* Complex nested structure with bit-fields */
struct Inner {
    int a;
    char b;
    unsigned int flag1:1;
    unsigned int flag2:3;
    unsigned int :4;  /* Padding */
};

/* Structure with anonymous union */
struct Container {
    struct Inner inner;
    union {
        long x;
        double y;
        struct {
            short count;
            char tag;
        } meta;
    } data;
    volatile int status;
};

/* Packed structure with GCC attributes */
struct __attribute__((packed, aligned(8))) PackedData {
    char id;
    int value;
    short checksum;
};

/* Structure with flexible array member */
struct DynamicBuffer {
    size_t length;
    char data[];
};

/* ==================== TYPE_UNION ==================== */

/* Complex union with nested struct */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    struct {
        short len;
        char buf[32];
    } as_string;
    double as_double_array[2];
};

/* Tagged union */
struct TaggedUnion {
    enum { INT, FLOAT, STRING, PTR } tag;
    union {
        int i;
        float f;
        char *str;
        void *ptr;
    } value;
};

/* ==================== TYPE_CALLBACK ==================== */

/* Function pointer typedefs */
typedef void (*event_handler)(int, void*);
typedef int (*processor_func)(float**, int);
typedef union Variant* (*variant_factory)(int);

/* Structure containing function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    event_handler on_event;
    variant_factory create_variant;
};

/* Callback function implementations */
static void sample_handler(int event, void* data) {
    struct Container *c = (struct Container*)data;
    if (c) c->status = event;
}

static int plugin_init(void) {
    return 42;
}

static void plugin_process(int x) {
    volatile int y = x * 2;
    (void)y;
}

static union Variant* make_variant(int type) {
    static union Variant v;
    v.as_int = type;
    return &v;
}

/* Array of plugins */
struct Plugin plugin_registry[3] = {
    {"alpha", plugin_init, plugin_process, sample_handler, make_variant},
    {"beta", plugin_init, plugin_process, NULL, NULL},
    {"gamma", NULL, NULL, sample_handler, make_variant}
};

/* ==================== TYPE_ARRAY & TYPE_POINTER ==================== */

/* Complex pointer and array types */
struct Node {
    int value;
    struct Node **children;  /* Pointer to pointer */
    struct Node *next;
};

/* Multi-dimensional array of pointers */
struct Node* adjacency_matrix[5][5];

/* Pointer to array */
int (*array_ptr)[10];

/* Function pointer with array parameter */
int (*signal_processor)(float[][256], int**);

/* Triple pointer chain */
int ***triple_ptr_chain;

/* ==================== TYPE_LANG_STRUCT ==================== */

/* GCC vector extensions */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned types */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
};

/* Transparent union */
union __attribute__((transparent_union)) TransparentUnion {
    int i;
    long l;
};

/* ==================== TYPE_SCALAR ==================== */

/* All scalar types */
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

/* Complex numbers */
float _Complex complex_float;
double _Complex complex_double;

/* ==================== TYPE_STRING ==================== */

/* String literals and arrays */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char static_string[] = "Static string data";
const char *dynamic_string = "Dynamic string";

/* ==================== GLOBAL VARIABLES ==================== */

/* Ensure all types are used globally */
struct Container global_container = {
    .inner = {1, 'A', 1, 3},
    .data = {.meta = {42, 'X'}},
    .status = 0
};

union Variant global_variant;
struct PackedData global_packed = {'Z', 999, 1234};
float32x8_t global_vector;
struct CacheLine global_cacheline;

/* ==================== FUNCTIONS ==================== */

/* Function using complex pointer types */
static void process_matrix(void) {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            adjacency_matrix[i][j] = NULL;
        }
    }
    
    /* Create a simple chain */
    struct Node *node = malloc(sizeof(struct Node));
    if (node) {
        node->value = 100;
        node->children = malloc(3 * sizeof(struct Node*));
        if (node->children) {
            for (int k = 0; k < 3; k++) node->children[k] = NULL;
        }
        adjacency_matrix[0][1] = node;
    }
}

/* Function using vector types */
static float vector_operation(void) {
    float32x8_t a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t b = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    float32x8_t c = a + b;
    
    /* Force use of result */
    float sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += ((float*)&c)[i];
    }
    return sum;
}

/* Function processing union type */
static int process_variant(union Variant *v, int type) {
    switch (type) {
        case 0:
            return v->as_int * 2;
        case 1:
            return (int)(v->as_float * 10);
        case 2:
            return v->as_string.len;
        default:
            return -1;
    }
}

/* Function with complex callback usage */
static void trigger_callbacks(void) {
    for (int i = 0; i < 3; i++) {
        if (plugin_registry[i].init) {
            int result = plugin_registry[i].init();
            (void)result;
        }
        
        if (plugin_registry[i].on_event) {
            plugin_registry[i].on_event(i, &global_container);
        }
        
        if (plugin_registry[i].create_variant) {
            union Variant *v = plugin_registry[i].create_variant(i);
            process_variant(v, i);
        }
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    int hash = 0;
    
    /* 1. Initialize and use struct/union types */
    global_variant.as_int = 42;
    hash += global_variant.as_int;
    
    global_variant.as_string.len = 10;
    strcpy(global_variant.as_string.buf, "Test");
    hash += global_variant.as_string.len;
    
    /* 2. Use vector types */
    float vector_result = vector_operation();
    hash += (int)vector_result;
    
    /* 3. Process matrix with pointer chains */
    process_matrix();
    if (adjacency_matrix[0][1]) {
        hash += adjacency_matrix[0][1]->value;
    }
    
    /* 4. Trigger callbacks */
    trigger_callbacks();
    hash += global_container.status;
    
    /* 5. Use all scalar types */
    char_var = 'A';
    schar_var = -1;
    uchar_var = 255;
    short_var = -32768;
    ushort_var = 65535;
    int_var = -2147483647;
    uint_var = 4294967295U;
    long_var = -2147483647L;
    ulong_var = 4294967295UL;
    llong_var = -9223372036854775807LL;
    ullong_var = 18446744073709551615ULL;
    float_var = 3.14159f;
    double_var = 2.718281828459045;
    ldouble_var = 1.618033988749895L;
    bool_var = 1;
    
    complex_float = 1.0f + 2.0f * I;
    complex_double = 3.0 + 4.0 * I;
    
    hash += char_var + schar_var + uchar_var + short_var + int_var;
    hash += (int)float_var + (int)double_var + bool_var;
    
    /* 6. Use string types */
    for (int i = 0; error_messages[i]; i++) {
        hash += error_messages[i][0];
    }
    hash += static_string[0];
    hash += dynamic_string[0];
    
    /* 7. Use packed and aligned types */
    global_packed.value = hash;
    hash += global_packed.checksum;
    
    /* 8. Create and use triple pointer */
    int ***triple = malloc(sizeof(int**));
    if (triple) {
        *triple = malloc(sizeof(int*));
        if (*triple) {
            **triple = malloc(sizeof(int));
            if (**triple) {
                ***triple = 999;
                hash += ***triple;
                free(**triple);
            }
            free(*triple);
        }
        free(triple);
    }
    
    /* 9. Process variant union */
    union Variant local_var;
    local_var.as_float = 3.14f;
    hash += process_variant(&local_var, 1);
    
    /* 10. Use opaque pointer */
    struct opaque *opaque_ptr = NULL;
    (void)opaque_ptr;
    
    /* Cleanup */
    if (adjacency_matrix[0][1]) {
        if (adjacency_matrix[0][1]->children) {
            free(adjacency_matrix[0][1]->children);
        }
        free(adjacency_matrix[0][1]);
    }
    
    printf("Final hash: %d\n", hash);
    return hash & 0xFF;  /* Return deterministic value */
}

/* Additional function to ensure type usage in different contexts */
static __attribute__((noinline)) void use_types_in_different_scope(void) {
    /* Force different type contexts */
    volatile struct Container local_container;
    volatile union Variant local_variant;
    volatile float32x8_t local_vector;
    
    local_container.status = 1;
    local_variant.as_int = 2;
    local_vector = (float32x8_t){0};
    
    /* Use function pointers indirectly */
    void (*func_ptr)(void) = (void (*)(void))use_types_in_different_scope;
    (void)func_ptr;
}

/* Ensure the function is called */
__attribute__((constructor)) static void init_types(void) {
    use_types_in_different_scope();
}
