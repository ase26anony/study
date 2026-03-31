/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
/* This program defines a diverse set of type constructs to exercise all
   type categories in GCC's GGC type state serialization. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ==================== TYPE_UNDEFINED ==================== */
struct opaque;  /* Forward declaration - undefined type */
extern struct opaque *global_opaque_ptr;  /* External reference */

/* ==================== TYPE_SCALAR ==================== */
/* Use all fundamental scalar types */
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
volatile _Complex float v_cfloat = 1.0f + 2.0f * I;
volatile _Complex double v_cdouble = 3.0 + 4.0 * I;
volatile _Complex long double v_cldouble = 5.0L + 6.0L * I;

/* ==================== TYPE_STRING ==================== */
const char *global_string = "Global string literal";
const char *error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable character array";

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*simple_callback)(void);
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(int event_id, void *user_data);
typedef char *(*string_transformer)(const char *, int);

/* Struct with function pointer members */
struct Plugin {
    const char *name;
    int version;
    int (*init)(void *config);
    void (*process)(int data);
    void (*cleanup)(struct Plugin *self);
    event_handler on_event;
};

/* Another callback type */
typedef struct {
    int priority;
    binary_op operation;
} MathCallback;

/* ==================== TYPE_STRUCT ==================== */
/* Simple nested structure */
struct Inner {
    int a;
    char b;
    float c;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
    unsigned int : 16;  /* Unnamed bit-field */
};

/* Struct with anonymous union */
struct WithAnonymousUnion {
    int type;
    union {
        int int_val;
        float float_val;
        char *str_val;
    };
    double extra;
};

/* Struct with flexible array member */
struct FlexArray {
    size_t length;
    int data[];
};

/* Main container structure */
struct Container {
    struct Inner inner;
    struct {
        long id;
        char tag[16];
    } metadata;
    union {
        long as_long;
        double as_double;
        void *as_pointer;
    } value;
    struct BitFieldStruct flags;
    struct WithAnonymousUnion anon_union;
    struct Plugin *plugin;
    volatile int counter;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Using typedef to create user-defined struct types */
typedef struct Container Container;
typedef struct {
    int x, y, z;
} Point3D;

typedef struct Node Node;
struct Node {
    int value;
    Node *next;
    Node *prev;
};

/* Self-referential structure */
typedef struct TreeNode {
    int data;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

/* ==================== TYPE_UNION ==================== */
union Variant {
    int as_int;
    long as_long;
    float as_float;
    double as_double;
    void *as_ptr;
    char *as_string;
    struct {
        short length;
        char buffer[32];
    } as_fixed_string;
    struct {
        size_t len;
        char buf[];
    } as_flex_string;
};

union NumericUnion {
    int i;
    float f;
    unsigned char bytes[4];
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC-specific type extensions */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Struct with GCC attributes */
struct __attribute__((aligned(64))) CacheAligned {
    int data[16];
    char padding[64 - sizeof(int[16])];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    double d;
};

/* ==================== TYPE_ARRAY ==================== */
/* Multi-dimensional arrays */
int matrix_2d[5][5];
float matrix_3d[3][3][3];

/* Array of structs */
struct Inner struct_array[10];
Container container_array[4];

/* Array of pointers */
char *string_ptr_array[] = {"first", "second", "third", NULL};
int *int_ptr_array[8];

/* Array of function pointers */
binary_op op_array[] = {NULL, NULL, NULL};

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer chains */
int ****quadruple_ptr;
Node ***node_ptr_matrix[5];
void (*complex_callback_array[3])(int, float, char*);

/* Pointer to array */
int (*ptr_to_array)[10];
float (*ptr_to_2d_array)[5][5];

/* Pointer to function returning pointer */
int *(*func_returning_int_ptr)(void);
char **(*func_returning_string_array)(int);

/* ==================== Function Definitions ==================== */
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

int plugin_init(void *config) {
    (void)config;
    return 0;
}

void plugin_process(int data) {
    volatile int result = data * 2;
    (void)result;
}

void plugin_cleanup(struct Plugin *self) {
    if (self) {
        /* Cleanup logic */
    }
}

void sample_event_handler(int event_id, void *user_data) {
    (void)event_id;
    (void)user_data;
}

char *uppercase_transform(const char *str, int len) {
    static char buffer[256];
    for (int i = 0; i < len && str[i]; i++) {
        buffer[i] = (str[i] >= 'a' && str[i] <= 'z') ? str[i] - 32 : str[i];
    }
    buffer[len < 255 ? len : 255] = '\0';
    return buffer;
}

/* Function using vector types */
float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex parameter types */
void process_variant(union Variant *v, event_handler cb) {
    if (v->as_int > 0 && cb) {
        cb(v->as_int, v);
    }
}

/* ==================== Global Variables ==================== */
struct Plugin global_plugin = {
    .name = "TestPlugin",
    .version = 1,
    .init = plugin_init,
    .process = plugin_process,
    .cleanup = plugin_cleanup,
    .on_event = sample_event_handler
};

Container global_container;
union Variant global_variants[3];
MathCallback global_callbacks[2];

float32x8_t global_vec1, global_vec2;
struct CacheAligned global_aligned_struct;
struct PackedStruct global_packed_struct;

/* ==================== main() ==================== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize struct and union variables */
    global_container.inner.a = 42;
    global_container.inner.b = 'X';
    global_container.inner.c = 3.14f;
    global_container.metadata.id = 1001;
    strncpy(global_container.metadata.tag, "main_container", 15);
    global_container.value.as_double = 2.71828;
    global_container.flags.flag1 = 1;
    global_container.flags.flag2 = 5;
    global_container.flags.value = -42;
    global_container.anon_union.type = 1;
    global_container.anon_union.int_val = 999;
    global_container.plugin = &global_plugin;
    global_container.counter = 0;
    
    /* 2. Initialize union array */
    global_variants[0].as_int = 100;
    global_variants[1].as_float = 3.14159f;
    global_variants[2].as_string = "union string";
    
    /* 3. Initialize callback array */
    global_callbacks[0].priority = 1;
    global_callbacks[0].operation = add;
    global_callbacks[1].priority = 2;
    global_callbacks[1].operation = multiply;
    
    /* 4. Populate array of structs with function pointers */
    struct Plugin plugin_registry[3];
    for (int i = 0; i < 3; i++) {
        plugin_registry[i].name = "Plugin";
        plugin_registry[i].version = i + 1;
        plugin_registry[i].init = plugin_init;
        plugin_registry[i].process = plugin_process;
        plugin_registry[i].cleanup = plugin_cleanup;
        plugin_registry[i].on_event = sample_event_handler;
    }
    
    /* Call function through pointer */
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init(NULL);
    }
    
    /* 5. Use vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = vector_add(vec_a, vec_b);
    global_vec1 = vec_sum;
    
    /* 6. Traverse multi-dimensional array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix_2d[i][j] = i * 5 + j;
            result += matrix_2d[i][j];
        }
    }
    
    /* 7. Process union type with runtime condition */
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            process_variant(&global_variants[i], sample_event_handler);
        } else if (i == 1) {
            volatile float f = global_variants[i].as_float;
            result += (int)f;
        } else {
            volatile const char *s = global_variants[i].as_string;
            if (s) result += s[0];
        }
    }
    
    /* 8. Use pointer chains */
    int val = 42;
    int *p1 = &val;
    int **p2 = &p1;
    int ***p3 = &p2;
    quadruple_ptr = &p3;
    
    if (****quadruple_ptr == 42) {
        result += 1000;
    }
    
    /* 9. Use string transformer callback */
    string_transformer transformer = uppercase_transform;
    char *transformed = transformer("hello world", 11);
    if (transformed) {
        result += transformed[0];
    }
    
    /* 10. Initialize and use aligned/packed structs */
    for (int i = 0; i < 16; i++) {
        global_aligned_struct.data[i] = i * 2;
        result += global_aligned_struct.data[i];
    }
    
    global_packed_struct.a = 'Z';
    global_packed_struct.b = 123456;
    global_packed_struct.c = -789;
    global_packed_struct.d = 1.23456789;
    
    /* 11. Use all scalar types in computation */
    result += v_char + v_short + v_int + (int)v_float + (int)v_double;
    result += (int)creal(v_cfloat) + (int)cimag(v_cdouble);
    
    /* 12. Create and use flexible array member */
    struct FlexArray *flex = malloc(sizeof(struct FlexArray) + 10 * sizeof(int));
    if (flex) {
        flex->length = 10;
        for (size_t i = 0; i < flex->length; i++) {
            flex->data[i] = (int)i * 3;
            result += flex->data[i];
        }
        free(flex);
    }
    
    /* 13. Build a linked list */
    Node *head = NULL;
    for (int i = 0; i < 5; i++) {
        Node *new_node = malloc(sizeof(Node));
        if (new_node) {
            new_node->value = i * 10;
            new_node->next = head;
            new_node->prev = NULL;
            if (head) head->prev = new_node;
            head = new_node;
            result += new_node->value;
        }
    }
    
    /* Cleanup linked list */
    while (head) {
        Node *next = head->next;
        free(head);
        head = next;
    }
    
    /* 14. Use array of function pointers */
    op_array[0] = add;
    op_array[1] = multiply;
    
    if (op_array[0] && op_array[1]) {
        result += op_array[0](10, 20);
        result += op_array[1](10, 20);
    }
    
    /* Final deterministic result */
    printf("Result: %d\n", result);
    return result % 256;
}
