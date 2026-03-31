/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

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
volatile int v_int = -2147483647 - 1;
volatile unsigned int v_uint = 4294967295U;
volatile long v_long = -9223372036854775807L - 1;
volatile unsigned long v_ulong = 18446744073709551615UL;
volatile long long v_llong = -9223372036854775807LL - 1;
volatile unsigned long long v_ullong = 18446744073709551615ULL;
volatile float v_float = 3.1415926535f;
volatile double v_double = 2.718281828459045;
volatile long double v_ldouble = 3.14159265358979323846L;
volatile _Bool v_bool = 1;
volatile _Complex float v_cfloat = 1.0f + 2.0fi;
volatile _Complex double v_cdouble = 3.0 + 4.0i;
volatile _Complex long double v_cldouble = 5.0L + 6.0Li;

/* ==================== TYPE_STRING ==================== */
const char *error_messages[] = {"Error", "Warning", "Info", NULL};
const char *multiline_string = "Line 1\n"
                               "Line 2\n"
                               "Line 3";
volatile const char *volatile_string = "Volatile string pointer";

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*simple_callback)(void);
typedef int (*data_processor)(const char *data, size_t len);
typedef double (*math_func)(double, double, void *context);

/* Complex callback signature */
typedef union Variant *(*variant_parser)(const char *input, 
                                         int (*validate)(const char*),
                                         void **error_ptr);

/* Struct with function pointer members */
struct Plugin {
    const char *name;
    int version;
    int (*init)(struct Plugin *self, void *config);
    void (*process)(struct Plugin *self, int data);
    void (*cleanup)(struct Plugin *self);
    variant_parser parse;
};

/* Another callback type for arrays */
typedef void (*event_handler[5])(int event_id, void *user_data);

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
    int id;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;  /* Unnamed bit-field */
    signed int value : 12;
    unsigned int : 0;  /* Force alignment */
    unsigned int last_flag : 2;
};

/* Struct with anonymous struct */
struct Container {
    struct {
        int a;
        char b;
        float c;
    } inner;
    long outer;
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t length;
    size_t capacity;
    int data[];
};

/* Packed struct */
struct __attribute__((packed)) PackedData {
    char type;
    int value;
    short count;
};

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {
    double data[8];
    int tag;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef struct */
typedef struct {
    int id;
    char name[32];
    struct Point location;
} UserType;

/* Typedef with attributes */
typedef struct __attribute__((packed)) {
    uint8_t header;
    uint32_t payload;
    uint16_t checksum;
} Packet;

/* Complex typedef chain */
typedef struct Node Node;
typedef struct Edge Edge;

struct Edge {
    Node *from;
    Node *to;
    double weight;
    Edge *next;
};

struct Node {
    int id;
    char *label;
    Edge *edges;
    Node **neighbors;  /* Array of pointers */
    size_t neighbor_count;
};

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* Tagged union */
union Variant {
    struct {
        int type;
        union {
            int int_val;
            double double_val;
            char *string_val;
            struct Point point_val;
        } data;
    } tagged;
    
    struct {
        unsigned char raw_data[16];
    } raw;
    
    struct {
        short len;
        char buf[];  /* Flexible array member in union */
    } as_string;
};

/* Union with anonymous union inside struct */
struct TypeValue {
    int type;
    union {
        int i;
        double d;
        const char *s;
    };  /* Anonymous union */
};

/* ==================== TYPE_ARRAY ==================== */
/* Multi-dimensional arrays */
int matrix_2d[10][10];
int matrix_3d[5][5][5];

/* Array of structs */
struct Point point_array[100];
struct Plugin plugin_registry[5];

/* Array of pointers */
char *string_array[] = {"first", "second", "third", NULL};
int *int_ptr_array[20];

/* Array of arrays */
typedef int Row[10];
Row row_array[5];

/* Array of function pointers */
math_func math_operations[] = {NULL, NULL, NULL};

/* ==================== TYPE_POINTER ==================== */
/* Multi-level pointers */
int ***triple_ptr;
void ****quad_ptr;

/* Pointer to array */
int (*ptr_to_array)[10];
char (*ptr_to_string_array)[20];

/* Pointer to function pointer */
simple_callback *callback_ptr_array;

/* Complex pointer type */
union Variant *(*variant_processor)(union Variant **input, 
                                    int count,
                                    void (*log)(const char*));

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;
typedef short __attribute__((vector_size(8))) int16x8_t;

/* Transparent union */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentPtr;

/* ==================== FUNCTION DEFINITIONS ==================== */

/* Callback implementations */
static int plugin_init(struct Plugin *self, void *config) {
    (void)self; (void)config;
    return 0;
}

static void plugin_process(struct Plugin *self, int data) {
    (void)self; (void)data;
    /* Do nothing */
}

static void plugin_cleanup(struct Plugin *self) {
    (void)self;
    /* Do nothing */
}

static union Variant *parse_variant(const char *input, 
                                   int (*validate)(const char*),
                                   void **error_ptr) {
    (void)input; (void)validate; (void)error_ptr;
    static union Variant result;
    return &result;
}

static double add_numbers(double a, double b, void *context) {
    (void)context;
    return a + b;
}

static double multiply_numbers(double a, double b, void *context) {
    (void)context;
    return a * b;
}

/* Function using vector types */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex pointer operations */
static void process_matrix(int (**matrix)[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            (*matrix)[i][j] = i * j;
        }
    }
}

/* Function using all type categories */
static int process_variant(union Variant *v) {
    if (!v) return -1;
    
    switch (v->tagged.type) {
        case 0:
            return v->tagged.data.int_val;
        case 1:
            return (int)v->tagged.data.double_val;
        case 2:
            return v->tagged.data.string_val ? 0 : -1;
        case 3:
            return v->tagged.data.point_val.x + v->tagged.data.point_val.y;
        default:
            return -2;
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    /* Initialize scalar types with operations */
    int scalar_sum = (int)v_char + v_int + (int)v_float;
    
    /* Use string types */
    const char *first_msg = error_messages[0];
    volatile_string = first_msg;
    
    /* Initialize and use structs */
    struct Point p1 = {.x = 10, .y = 20, .z = 30};
    struct Rectangle rect = {
        .top_left = {.x = 0, .y = 0, .z = 0},
        .bottom_right = {.x = 100, .y = 100, .z = 0},
        .id = 1
    };
    
    struct BitFieldStruct bfs = {
        .flag1 = 1,
        .flag2 = 3,
        .flag3 = 7,
        .value = -2048,
        .last_flag = 2
    };
    
    /* Initialize user-defined struct */
    UserType user = {
        .id = 42,
        .name = "Test User",
        .location = p1
    };
    
    /* Initialize union */
    union Variant var;
    var.tagged.type = 0;
    var.tagged.data.int_val = 12345;
    
    union SimpleUnion su;
    su.as_int = 42;
    su.as_float = 3.14f;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix_2d[i][j] = i + j;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].z = i * 3;
    }
    
    /* Initialize plugin registry with callbacks */
    plugin_registry[0] = (struct Plugin){
        .name = "Test Plugin",
        .version = 1,
        .init = plugin_init,
        .process = plugin_process,
        .cleanup = plugin_cleanup,
        .parse = parse_variant
    };
    
    /* Initialize function pointer array */
    math_operations[0] = add_numbers;
    math_operations[1] = multiply_numbers;
    
    /* Use function pointers */
    double result1 = math_operations[0](2.5, 3.5, NULL);
    double result2 = math_operations[1](2.5, 3.5, NULL);
    
    /* Use vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_c = vector_add(vec_a, vec_b);
    
    /* Complex pointer operations */
    int **dynamic_array = (int**)malloc(10 * sizeof(int*));
    for (int i = 0; i < 10; i++) {
        dynamic_array[i] = (int*)malloc(10 * sizeof(int));
        for (int j = 0; j < 10; j++) {
            dynamic_array[i][j] = i * j;
        }
    }
    
    /* Pointer to array */
    ptr_to_array = &matrix_2d;
    (*ptr_to_array)[5][5] = 99;
    
    /* Multi-level pointer */
    int val = 42;
    int *pval = &val;
    int **ppval = &pval;
    triple_ptr = &ppval;
    ***triple_ptr = 100;
    
    /* Process matrix through pointer */
    int (*matrix_ptr)[10][10] = &matrix_2d;
    process_matrix(&matrix_ptr);
    
    /* Process variant */
    int variant_result = process_variant(&var);
    
    /* Complex type chain */
    Node *node1 = (Node*)malloc(sizeof(Node));
    Node *node2 = (Node*)malloc(sizeof(Node));
    
    node1->id = 1;
    node1->label = "Node 1";
    node1->edges = NULL;
    node1->neighbors = (Node**)malloc(2 * sizeof(Node*));
    node1->neighbors[0] = node2;
    node1->neighbors[1] = NULL;
    node1->neighbor_count = 1;
    
    node2->id = 2;
    node2->label = "Node 2";
    node2->edges = NULL;
    node2->neighbors = (Node**)malloc(2 * sizeof(Node*));
    node2->neighbors[0] = node1;
    node2->neighbors[1] = NULL;
    node2->neighbor_count = 1;
    
    /* Create an edge */
    Edge *edge = (Edge*)malloc(sizeof(Edge));
    edge->from = node1;
    edge->to = node2;
    edge->weight = 1.5;
    edge->next = NULL;
    
    node1->edges = edge;
    
    /* Calculate final result using all types */
    int final_result = scalar_sum 
                     + p1.x + rect.id 
                     + (int)result1 + (int)result2
                     + variant_result
                     + node1->id + node2->id
                     + (int)vec_c[0]
                     + matrix_2d[0][0]
                     + ***triple_ptr;
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(dynamic_array[i]);
    }
    free(dynamic_array);
    
    free(node1->neighbors);
    free(node2->neighbors);
    free(edge);
    free(node1);
    free(node2);
    
    /* Return deterministic value based on all operations */
    return final_result % 256;
}
