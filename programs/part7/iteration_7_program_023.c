/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ==================== TYPE_UNDEFINED ==================== */
struct opaque;  /* Forward declaration - undefined type */
extern struct opaque *global_opaque_ptr;

/* ==================== TYPE_SCALAR ==================== */
/* All fundamental scalar types */
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
volatile float v_float = 3.14159f;
volatile double v_double = 2.718281828459045;
volatile _Bool v_bool = 1;
volatile _Complex float v_cfloat = 1.0f + 2.0fi;
volatile _Complex double v_cdouble = 3.0 + 4.0i;

/* ==================== TYPE_STRING ==================== */
const char *global_string = "Hello, GCC type system!";
const char *error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable string data";

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(int, const char*, void*);
typedef void (*event_handler)(int, void*);
typedef double (*math_func)(double, double);

/* Callback function implementations */
static void callback_impl(void) {
    v_int++;
}

static int complex_callback_impl(int code, const char *msg, void *data) {
    return code + (int)((intptr_t)data);
}

static double add_func(double a, double b) { return a + b; }
static double mul_func(double a, double b) { return a * b; }

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    void (*cleanup)(void);
    complex_callback user_cb;
};

static int plugin1_init(void) { return 0; }
static void plugin1_process(int x) { v_int += x; }
static void plugin1_cleanup(void) { v_char = 'Z'; }

static int plugin2_init(void) { return 1; }
static void plugin2_process(int x) { v_int -= x; }
static void plugin2_cleanup(void) { v_char = 'Y'; }

/* Array of function pointers */
math_func math_operations[] = {add_func, mul_func, NULL};

/* ==================== TYPE_STRUCT ==================== */
/* Simple struct */
struct Point {
    int x;
    int y;
    int z;
};

/* Nested struct */
struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int area;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
    signed int value : 16;
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t length;
    int data[];
};

/* Anonymous struct inside struct */
struct Container {
    struct {
        int a;
        char b;
        float c;
    } inner;
    union {
        long x;
        double y;
        void *z;
    } data;
    struct Container *next;
};

/* Packed struct */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    double d;
};

/* Aligned struct */
struct __attribute__((aligned(64))) AlignedStruct {
    int values[16];
    char tag;
};

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* Union with struct member */
union Variant {
    int as_int;
    void* as_ptr;
    struct {
        short len;
        char buf[];
    } as_string;
    double as_double;
    struct Point as_point;
};

/* Tagged union */
struct TaggedVariant {
    enum { INT, FLOAT, STRING, POINT } type;
    union {
        int i;
        float f;
        char *s;
        struct Point p;
    } value;
};

/* ==================== TYPE_ARRAY ==================== */
/* Multi-dimensional arrays */
int matrix_2d[5][5];
int matrix_3d[3][3][3];

/* Array of structs */
struct Point point_array[10];

/* Array of pointers */
int *pointer_array[20];

/* Array of arrays */
int nested_array[4][8];

/* Array of function pointers */
simple_callback callback_array[5];

/* ==================== TYPE_POINTER ==================== */
/* Multi-level pointers */
int ***triple_ptr;

/* Pointer to array */
int (*ptr_to_array)[10];

/* Pointer to function */
int (*func_ptr)(int, char**);

/* Pointer to struct */
struct Rectangle *rect_ptr;

/* Pointer to union */
union Variant *variant_ptr;

/* Complex pointer type */
struct Container **(*complex_ptr_func)(int, struct Point[][5]);

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* GCC-specific attributes */
struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
    long value;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef struct */
typedef struct {
    int id;
    char name[32];
    float score;
} User;

typedef union {
    int x;
    long y;
} Number;

/* Typedef for complex type */
typedef struct Container* ContainerPtr;
typedef ContainerPtr (*ContainerFactory)(int);

/* ==================== Global Variables ==================== */
struct Plugin plugin_registry[3] = {
    {"Plugin1", plugin1_init, plugin1_process, plugin1_cleanup, complex_callback_impl},
    {"Plugin2", plugin2_init, plugin2_process, plugin2_cleanup, NULL},
    {"Plugin3", NULL, NULL, NULL, NULL}
};

union Variant global_variant;
struct TaggedVariant global_tagged;
float32x8_t global_vector;

/* ==================== Functions ==================== */
/* Function using complex types */
static void process_container(struct Container *cont, event_handler handler) {
    if (cont && handler) {
        handler(cont->inner.a, &cont->data);
    }
}

/* Function with array parameter */
static int sum_matrix(int mat[][5], int rows) {
    int total = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 5; j++) {
            total += mat[i][j];
        }
    }
    return total;
}

/* Function using vector types */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function processing union */
static void process_variant(union Variant *v, int type) {
    switch (type) {
        case 0:
            v->as_int = 42;
            break;
        case 1:
            v->as_double = 3.14159;
            break;
        case 2:
            /* Can't actually use flexible array here */
            break;
    }
}

/* Function with callback */
static void execute_callbacks(void) {
    for (int i = 0; i < 5 && callback_array[i]; i++) {
        callback_array[i]();
    }
}

/* ==================== Main Function ==================== */
int main(int argc, char **argv) {
    /* Initialize variables to prevent optimization */
    int result = 0;
    
    /* 1. Use scalar types */
    result += v_char + v_int + (int)v_float;
    
    /* 2. Use string types */
    result += (int)strlen(global_string);
    result += (int)strlen(error_messages[0]);
    
    /* 3. Initialize and use structs */
    struct Point p1 = {10, 20, 30};
    struct Rectangle rect = {{0, 0, 0}, {100, 100, 0}, 10000};
    struct Container cont = {{1, 'A', 2.5f}, {.x = 100}, NULL};
    
    result += p1.x + rect.area + cont.inner.a;
    
    /* 4. Use unions */
    union Variant var;
    var.as_int = 123;
    result += var.as_int;
    
    global_tagged.type = INT;
    global_tagged.value.i = 456;
    result += global_tagged.value.i;
    
    /* 5. Use arrays */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix_2d[i][j] = i * j;
            result += matrix_2d[i][j];
        }
    }
    
    /* 6. Use pointers */
    int x = 42;
    int *px = &x;
    int **ppx = &px;
    int ***pppx = &ppx;
    
    result += ***pppx;
    
    /* 7. Use function pointers and callbacks */
    callback_array[0] = callback_impl;
    callback_array[1] = NULL;
    
    execute_callbacks();
    
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    if (plugin_registry[0].process) {
        plugin_registry[0].process(5);
    }
    
    /* 8. Use vector types (GCC extension) */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = vector_add(vec1, vec2);
    
    /* Extract value from vector to use in result */
    float vec_elements[8];
    memcpy(vec_elements, &vec_sum, sizeof(vec_sum));
    result += (int)vec_elements[0];
    
    /* 9. Use typedef types */
    User user = {1, "Test User", 95.5f};
    result += user.id + (int)user.score;
    
    /* 10. Complex pointer chain */
    int val = 999;
    int *p1 = &val;
    int **p2 = &p1;
    int ***p3 = &p2;
    result += ***p3;
    
    /* 11. Pointer to array */
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    ptr_to_array = &arr;
    result += (*ptr_to_array)[5];
    
    /* 12. Multi-dimensional array of pointers */
    struct Node* adjacency_matrix[10][10];
    /* Initialize to NULL */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            adjacency_matrix[i][j] = NULL;
        }
    }
    
    /* 13. Use math function pointers */
    if (math_operations[0]) {
        double math_result = math_operations[0](2.5, 3.5);
        result += (int)math_result;
    }
    
    /* 14. Process container with callback */
    process_container(&cont, NULL);
    
    /* 15. Use bit-field struct */
    struct BitFieldStruct bfs = {1, 7, 15, 0, -100};
    result += bfs.flag1 + bfs.flag2 + bfs.flag3 + bfs.value;
    
    /* Return deterministic result */
    return result % 256;
}
