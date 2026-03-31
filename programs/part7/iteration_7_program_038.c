/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== TYPE_UNDEFINED / Forward Declarations ==================== */
struct opaque;                     /* Incomplete type - TYPE_UNDEFINED */
struct forward_declared;           /* Another undefined type */

/* ==================== TYPE_SCALAR - All fundamental types ==================== */
typedef char               t_char;
typedef short              t_short;
typedef int                t_int;
typedef long               t_long;
typedef long long          t_llong;
typedef float              t_float;
typedef double             t_double;
typedef long double        t_ldouble;
typedef _Bool              t_bool;
typedef _Complex float     t_cfloat;
typedef _Complex double    t_cdouble;

/* ==================== TYPE_STRING ==================== */
const char* global_string = "Global string literal";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};

/* ==================== TYPE_LANG_STRUCT - GCC extensions ==================== */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

struct __attribute__((packed)) packed_struct {
    char c;
    int i;
    short s;
};

struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
    int tag;
};

/* ==================== TYPE_CALLBACK - Function pointers ==================== */
typedef void (*simple_cb)(void);
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(int event_id, void* user_data);
typedef char* (*string_transformer)(const char*, int);

struct Plugin {
    const char* name;
    int (*init)(void* config);
    void (*process)(int data);
    void (*shutdown)(void);
    event_handler on_event;
};

struct CallbackContainer {
    binary_op op;
    string_transformer transform;
    void (*nested[3])(struct CallbackContainer*);
};

/* ==================== TYPE_UNION ==================== */
union DataVariant {
    int as_int;
    long as_long;
    float as_float;
    double as_double;
    void* as_ptr;
    char as_bytes[8];
    struct {
        short length;
        char data[];
    } as_string;
};

union NestedUnion {
    struct {
        int type;
        union {
            int i;
            float f;
        } value;
    } tagged;
    long long raw;
};

/* ==================== TYPE_STRUCT / TYPE_USER_STRUCT ==================== */
/* Anonymous struct/union */
struct Container {
    struct {
        int a;
        char b;
        unsigned bitfield : 3;
        unsigned : 5;  /* unnamed bitfield */
        unsigned another_bit : 4;
    } inner;
    union {
        long x;
        double y;
        void* z;
    } data;
    int array_in_struct[5];
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t capacity;
    size_t length;
    int items[];
};

/* Complex nested structure */
struct TreeNode {
    int value;
    struct TreeNode* left;
    struct TreeNode* right;
    struct TreeNode* parent;
    union {
        void* metadata;
        long hash;
    } aux;
};

/* Bitfield-intensive struct */
struct RegisterMap {
    unsigned int flag_a : 1;
    unsigned int flag_b : 1;
    unsigned int mode   : 3;
    unsigned int count  : 10;
    unsigned int : 0;  /* force alignment */
    unsigned int extended : 16;
    unsigned char padding[3];
};

/* ==================== TYPE_ARRAY ==================== */
/* Multi-dimensional arrays */
int matrix_2d[10][20];
float matrix_3d[5][5][5];

/* Array of structs */
struct Container container_array[7];

/* Array of pointers */
struct TreeNode* node_ptr_array[100];

/* Array of function pointers */
binary_op operation_array[4];

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer chains */
int**** quadruple_ptr;
struct Container*** container_ptr_ptr;
void (* volatile volatile_func_ptr)(void);

/* Pointer to array */
int (*ptr_to_array)[20];
float (*ptr_to_3d_array)[5][5];

/* ==================== Function Implementations ==================== */
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

void sample_event_handler(int event_id, void* user_data) {
    printf("Event %d handled\n", event_id);
}

char* uppercase_transform(const char* str, int len) {
    (void)len;
    static char buffer[256];
    strncpy(buffer, str, 255);
    for (int i = 0; buffer[i]; i++) {
        if (buffer[i] >= 'a' && buffer[i] <= 'z')
            buffer[i] = buffer[i] - 'a' + 'A';
    }
    return buffer;
}

int plugin_init(void* config) {
    (void)config;
    return 0;
}

void plugin_process(int data) {
    printf("Processing %d\n", data);
}

void plugin_shutdown(void) {
    printf("Shutdown\n");
}

/* ==================== Main Function ==================== */
int main(void) {
    volatile int coverage_helper = 0;
    
    /* 1. Initialize scalar types */
    t_char c = 'A';
    t_short s = 100;
    t_int i = -42;
    t_long l = 100000L;
    t_llong ll = 10000000000LL;
    t_float f = 3.14159f;
    t_double d = 2.718281828459045;
    t_ldouble ld = 1.618033988749895L;
    t_bool b = 1;
    t_cfloat cf = 1.0f + 2.0f * I;
    t_cdouble cd = 3.0 + 4.0 * I;
    
    coverage_helper += c + s + i;
    
    /* 2. Use string types */
    const char* local_string = "Local string";
    printf("%s\n", error_messages[0]);
    coverage_helper += strlen(global_string) + strlen(local_string);
    
    /* 3. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_c = vec_a; /* Simple operation */
    (void)vec_c;
    
    /* 4. Initialize packed and aligned structs */
    struct packed_struct ps = {'X', 12345, 6789};
    struct aligned_struct as = {{1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8}, 99};
    coverage_helper += ps.i + as.tag;
    
    /* 5. Use unions */
    union DataVariant dv;
    dv.as_int = 42;
    coverage_helper += dv.as_int;
    
    dv.as_float = 3.14f;
    coverage_helper += (int)dv.as_float;
    
    union NestedUnion nu;
    nu.tagged.type = 1;
    nu.tagged.value.i = 100;
    coverage_helper += nu.tagged.value.i;
    
    /* 6. Initialize complex structs */
    struct Container cont = {
        .inner = {10, 'Z', 3, 9},
        .data = {.y = 2.71828},
        .array_in_struct = {1,2,3,4,5}
    };
    coverage_helper += cont.inner.a + cont.array_in_struct[0];
    
    struct RegisterMap rm = {1, 0, 4, 512, 65535, {0,1,2}};
    coverage_helper += rm.count;
    
    /* 7. Setup function pointers and callbacks */
    struct Plugin my_plugin = {
        "TestPlugin",
        plugin_init,
        plugin_process,
        plugin_shutdown,
        sample_event_handler
    };
    
    struct CallbackContainer cbs = {
        add,
        uppercase_transform,
        {NULL, NULL, NULL}
    };
    
    /* Call through function pointers */
    int result = cbs.op(10, 20);
    coverage_helper += result;
    
    char* transformed = cbs.transform("hello", 5);
    coverage_helper += strlen(transformed);
    
    my_plugin.process(42);
    my_plugin.on_event(100, NULL);
    
    /* 8. Initialize arrays */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 20; y++) {
            matrix_2d[x][y] = x * y;
            coverage_helper += matrix_2d[x][y];
        }
    }
    
    for (int i = 0; i < 7; i++) {
        container_array[i].inner.a = i * 10;
        coverage_helper += container_array[i].inner.a;
    }
    
    operation_array[0] = add;
    operation_array[1] = multiply;
    coverage_helper += operation_array[0](5, 6);
    coverage_helper += operation_array[1](5, 6);
    
    /* 9. Complex pointer operations */
    int**** quad_ptr = malloc(sizeof(int***));
    *quad_ptr = malloc(sizeof(int**));
    **quad_ptr = malloc(sizeof(int*));
    ***quad_ptr = &i;
    coverage_helper += ****quad_ptr;
    
    /* Cleanup */
    free(***quad_ptr);
    free(**quad_ptr);
    free(*quad_ptr);
    free(quad_ptr);
    
    /* 10. Pointer to array */
    ptr_to_array = &matrix_2d[0];
    coverage_helper += (*ptr_to_array)[0];
    
    /* 11. Use volatile pointer */
    volatile_func_ptr = (void(*)())main;
    
    /* 12. Multi-dimensional array access */
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++) {
            for (int z = 0; z < 5; z++) {
                matrix_3d[x][y][z] = x * 0.1f + y * 0.2f + z * 0.3f;
                coverage_helper += (int)matrix_3d[x][y][z];
            }
        }
    }
    
    /* Return deterministic value based on all operations */
    return coverage_helper % 256;
}
