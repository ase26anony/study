/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== TYPE_UNDEFINED / Forward Declarations ==================== */
struct opaque;                    /* Incomplete/undefined type */
extern struct opaque *global_opaque_ptr;  /* TYPE_UNDEFINED via pointer */

/* ==================== TYPE_SCALAR - All fundamental types ==================== */
typedef _Complex float complex_float;
typedef _Complex double complex_double;

/* ==================== TYPE_STRING ==================== */
const char *error_messages[] = {"Error", "Warning", "Info", NULL};
const wchar_t *wide_str = L"WideString";

/* ==================== TYPE_CALLBACK - Function pointers ==================== */
typedef void (*event_handler)(int, void*);
typedef int (*comparator_fn)(const void*, const void*);
typedef double (*transform_fn)(double);

struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    event_handler on_error;
};

/* Callback implementations */
static int plugin_init_default(void) { return 0; }
static void plugin_process_default(int x) { printf("Processing %d\n", x); }
static void error_callback(int err, void *data) { 
    printf("Error %d: %s\n", err, (char*)data); 
}

/* ==================== TYPE_STRUCT / TYPE_USER_STRUCT ==================== */
/* Nested anonymous struct */
struct Container {
    struct {
        int a;
        char b;
        unsigned:4;  /* bitfield */
        signed field:8;
    } inner;
    union {
        long x;
        double y;
    } data;
    volatile int counter;
};

/* Struct with flexible array member */
struct DynamicBuffer {
    size_t length;
    char data[];
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedData {
    char id;
    int value;
    short tag;
};

/* ==================== TYPE_UNION ==================== */
union Variant {
    int as_int;
    void* as_ptr;
    float as_float;
    struct {
        short len;
        char buf[];  /* Flexible array in union member */
    } as_string;
};

/* Tagged union */
struct TaggedVariant {
    enum { INT, FLOAT, STRING } tag;
    union {
        int i;
        float f;
        char *s;
    } value;
};

/* ==================== TYPE_ARRAY - Complex arrays ==================== */
/* Multi-dimensional array of structs */
struct Point3D {
    float x, y, z;
};

struct Point3D point_grid[5][5];  /* 2D array of structs */

/* Array of pointers to function pointers */
event_handler handler_array[10];

/* Pointer to array */
int (*array_ptr)[20];

/* ==================== TYPE_POINTER - Complex pointer chains ==================== */
struct Node {
    int value;
    struct Node ***triple_ptr;  /* Triple indirection */
    struct Node *children[4];
};

/* Chain of pointers */
int ****quad_ptr;

/* ==================== TYPE_LANG_STRUCT - GCC extensions ==================== */
/* Vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {
    char data[64];
};

/* Transparent union */
union __attribute__((transparent_union)) Number {
    int i;
    float f;
    double d;
};

/* ==================== Global instances ==================== */
struct Plugin plugin_registry[3] = {
    {"plugin1", plugin_init_default, plugin_process_default, error_callback},
    {"plugin2", NULL, NULL, NULL},
    {"plugin3", plugin_init_default, plugin_process_default, NULL}
};

union Variant global_variant;
struct Container global_container;
float32x8_t global_vector;

/* ==================== Functions using complex types ==================== */
static double transform_square(double x) { return x * x; }

static int compare_ints(const void *a, const void *b) {
    return *(int*)a - *(int*)b;
}

void process_container(struct Container *c, event_handler cb) {
    c->inner.a++;
    c->counter *= 2;
    if (cb) cb(c->inner.a, &c->inner.b);
}

void simulate(union Variant *v, int type) {
    static char msg[] = "Simulation";
    switch(type) {
        case 0: v->as_int = 42; break;
        case 1: v->as_float = 3.14f; break;
        case 2: 
            v->as_string.len = strlen(msg);
            /* Note: can't assign flexible array in static init */
            break;
    }
}

void traverse_pointers(struct Node *n) {
    if (n && n->triple_ptr && *n->triple_ptr && **n->triple_ptr) {
        (***n->triple_ptr).value = 100;
    }
}

/* ==================== main ==================== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize structs and unions */
    struct Container local_container = {
        .inner = {.a = 10, .b = 'X', .field = 127},
        .data = {.x = 1000L},
        .counter = 1
    };
    
    union Variant local_variant;
    simulate(&local_variant, 0);
    simulate(&global_variant, 1);
    
    /* 2. Use arrays of structs with function pointers */
    for (int i = 0; i < 3; i++) {
        if (plugin_registry[i].init) {
            result += plugin_registry[i].init();
        }
        if (plugin_registry[i].process) {
            plugin_registry[i].process(i);
            result += i;
        }
    }
    
    /* 3. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    float32x8_t vec_c = vec_a + vec_b;  /* Vector addition */
    global_vector = vec_c;
    
    /* Access vector elements to prevent dead code elimination */
    volatile float *vp = (float*)&vec_c;
    result += (int)vp[0];
    
    /* 4. Multi-dimensional array operations */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            point_grid[i][j] = (struct Point3D){i*1.0f, j*1.0f, (i+j)*1.0f};
            result += (int)point_grid[i][j].x;
        }
    }
    
    /* 5. Pointer chains and complex structures */
    struct Node *node = malloc(sizeof(struct Node));
    struct Node **ptr1 = malloc(sizeof(struct Node*));
    struct Node *ptr2 = malloc(sizeof(struct Node));
    
    if (node && ptr1 && ptr2) {
        node->value = 50;
        node->triple_ptr = malloc(sizeof(struct Node**));
        *node->triple_ptr = ptr1;
        *ptr1 = ptr2;
        ptr2->value = 99;
        
        traverse_pointers(node);
        result += ptr2->value;
        
        free(node->triple_ptr);
        free(ptr1);
        free(ptr2);
        free(node);
    }
    
    /* 6. Process union with runtime condition */
    for (int i = 0; i < 3; i++) {
        simulate(&local_variant, i % 3);
        result += local_variant.as_int;
    }
    
    /* 7. Use string types */
    const char **msg_ptr = error_messages;
    while (*msg_ptr) {
        result += (int)**msg_ptr;
        msg_ptr++;
    }
    
    /* 8. Callback through typedef */
    comparator_fn cmp = compare_ints;
    transform_fn trans = transform_square;
    int nums[] = {5, 2, 8, 1, 9};
    qsort(nums, 5, sizeof(int), cmp);
    result += (int)trans(nums[4]);
    
    /* 9. Process container with callback */
    process_container(&local_container, error_callback);
    result += local_container.inner.a;
    
    /* 10. Use opaque pointer type */
    struct opaque *opaque_ptr = NULL;
    global_opaque_ptr = opaque_ptr;
    
    /* Deterministic return value */
    printf("Result: %d\n", result);
    return result % 256;
}
