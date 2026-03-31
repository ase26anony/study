/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ==================== TYPE_UNDEFINED / Forward Declarations ==================== */
struct opaque;                     /* Forward declaration - TYPE_UNDEFINED */
struct incomplete;                 /* Another forward declaration */

/* ==================== TYPE_SCALAR - All fundamental types ==================== */
volatile char global_char = 'A';
volatile short global_short = 100;
volatile int global_int = 1000;
volatile long global_long = 10000L;
volatile long long global_llong = 100000LL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;
volatile _Bool global_bool = 1;
volatile _Complex float global_cfloat = 1.0f + 2.0f * I;
volatile _Complex double global_cdouble = 3.0 + 4.0 * I;

/* ==================== TYPE_STRING ==================== */
const char* global_string = "Global string literal";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};

/* ==================== TYPE_CALLBACK - Function pointers ==================== */
typedef void (*event_handler)(int, void*);  /* Callback typedef */
typedef int (*comparator_fn)(const void*, const void*);  /* Another callback type */

/* Callback function implementations */
void sample_handler(int event_id, void* data) {
    printf("Event %d handled with data %p\n", event_id, data);
}

int int_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

/* ==================== TYPE_STRUCT ==================== */
/* Simple struct */
struct Point {
    int x;
    int y;
    char label[20];
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

/* Struct with anonymous union (C11) */
struct WithAnonymousUnion {
    int type;
    union {
        int int_value;
        double double_value;
        char* string_value;
    };
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Complex nested struct */
struct Container {
    struct {
        int a;
        char b;
        volatile long counter;
    } inner;
    
    union {
        long x;
        double y;
        void* ptr;
    } data;
    
    struct Point points[5];
    event_handler callback;
};

/* Struct with flexible array member */
struct FlexArray {
    size_t length;
    int data[];
};

/* ==================== TYPE_UNION ==================== */
union Variant {
    int as_int;
    long as_long;
    double as_double;
    void* as_ptr;
    struct {
        short len;
        char buf[];  /* Flexible array in union member */
    } as_string;
    
    struct {
        int type;
        union {
            int i;
            float f;
        } value;
    } nested;
};

/* Tagged union */
union TaggedUnion {
    struct {
        int tag;
        union {
            int i;
            float f;
            char* s;
        } data;
    } tagged;
    long raw;
};

/* ==================== TYPE_LANG_STRUCT - GCC extensions ==================== */
/* GCC vector type */
typedef float __attribute__((vector_size(32))) float32x8_t;

/* Packed struct */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    double d;
};

/* Aligned struct */
struct __attribute__((aligned(64))) AlignedStruct {
    int data[16];
    float matrix[4][4];
};

/* Transparent union */
union __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    void* void_ptr;
};

/* ==================== TYPE_ARRAY & TYPE_POINTER ==================== */
/* Multi-dimensional array of structs */
struct Point point_matrix[10][10];

/* Array of pointers to different struct types */
void* polymorphic_array[20];

/* Complex pointer chain */
int**** quad_ptr;

/* Function pointer with array parameters */
int (*signal_processor)(float[][256], int**, size_t);

/* ==================== Struct with function pointers ==================== */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void);
    void (*process)(int);
    void (*cleanup)(struct Plugin*);
    comparator_fn compare;
};

/* Plugin implementations */
int plugin1_init(void) {
    printf("Plugin 1 initialized\n");
    return 0;
}

void plugin1_process(int data) {
    printf("Plugin 1 processing: %d\n", data);
}

void plugin1_cleanup(struct Plugin* p) {
    printf("Cleaning up plugin: %s\n", p->name);
}

/* ==================== Complex type definitions ==================== */
/* Typedef chain */
typedef struct Node* NodePtr;
typedef NodePtr* NodePtrPtr;
typedef NodePtrPtr (*NodeFactory)(int);

/* Struct with pointer to itself */
struct Node {
    int id;
    char* name;
    struct Node* next;
    struct Node* children[5];
    NodePtrPtr indirect;
};

/* ==================== Function declarations ==================== */
void process_container(struct Container* c);
void handle_variant(union Variant* v, int type);
float32x8_t process_vector(float32x8_t a, float32x8_t b);
void traverse_pointers(int**** ptr);
void register_plugins(struct Plugin* plugins, int count);
int compute_hash(void);

/* ==================== Function implementations ==================== */
void process_container(struct Container* c) {
    c->inner.a = global_int;
    c->inner.b = (char)global_char;
    c->inner.counter++;
    
    if (c->callback) {
        c->callback(42, c);
    }
    
    for (int i = 0; i < 5; i++) {
        c->points[i].x = i * 10;
        c->points[i].y = i * 20;
        snprintf(c->points[i].label, 20, "Point%d", i);
    }
}

void handle_variant(union Variant* v, int type) {
    switch (type) {
        case 0:
            v->as_int = global_int;
            break;
        case 1:
            v->as_double = global_double;
            break;
        case 2:
            v->as_ptr = &global_int;
            break;
        case 3:
            v->nested.type = 1;
            v->nested.value.i = 100;
            break;
    }
}

float32x8_t process_vector(float32x8_t a, float32x8_t b) {
    float32x8_t result;
    for (int i = 0; i < 8; i++) {
        result[i] = a[i] + b[i];
    }
    return result;
}

void traverse_pointers(int**** ptr) {
    if (ptr && *ptr && **ptr && ***ptr) {
        ****ptr = global_int;
    }
}

void register_plugins(struct Plugin* plugins, int count) {
    for (int i = 0; i < count; i++) {
        if (plugins[i].init) {
            plugins[i].init();
        }
    }
}

int compute_hash(void) {
    int hash = 0;
    
    /* Use all global scalars */
    hash += global_char;
    hash += global_short;
    hash += global_int;
    hash ^= (int)global_long;
    hash ^= (int)(global_llong >> 32);
    hash ^= (int)global_llong;
    hash += (int)global_float;
    hash += (int)global_double;
    hash += global_bool;
    
    /* Use string */
    if (global_string) {
        hash += strlen(global_string);
    }
    
    return hash & 0x7FFFFFFF;
}

/* ==================== Main function ==================== */
int main(void) {
    volatile int result = 0;
    
    /* 1. Initialize and use struct types */
    struct Container container = {
        .inner = { .a = 1, .b = 'X', .counter = 0 },
        .data = { .x = 1000L },
        .callback = sample_handler
    };
    process_container(&container);
    
    /* 2. Use union types */
    union Variant variant;
    for (int i = 0; i < 4; i++) {
        handle_variant(&variant, i);
        if (i == 0) result += variant.as_int;
        if (i == 1) result += (int)variant.as_double;
    }
    
    /* 3. Setup and use function pointers in structs */
    struct Plugin plugins[3] = {
        {
            .name = "Plugin1",
            .version = 1,
            .init = plugin1_init,
            .process = plugin1_process,
            .cleanup = plugin1_cleanup,
            .compare = int_comparator
        },
        {
            .name = "Plugin2",
            .version = 2,
            .init = NULL,
            .process = plugin1_process,  /* Reuse */
            .cleanup = plugin1_cleanup,
            .compare = int_comparator
        }
    };
    
    register_plugins(plugins, 2);
    if (plugins[0].process) {
        plugins[0].process(123);
    }
    
    /* 4. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_result = process_vector(vec_a, vec_b);
    result += (int)vec_result[0];
    
    /* 5. Complex pointer and array operations */
    /* Initialize point matrix */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            point_matrix[i][j].x = i + j;
            point_matrix[i][j].y = i * j;
            snprintf(point_matrix[i][j].label, 20, "(%d,%d)", i, j);
            result += point_matrix[i][j].x;
        }
    }
    
    /* Setup polymorphic array */
    polymorphic_array[0] = &container;
    polymorphic_array[1] = &variant;
    polymorphic_array[2] = plugins;
    polymorphic_array[3] = &point_matrix[0][0];
    
    /* 6. Use all scalar types in computations */
    result += global_char + global_short + global_int;
    result += (int)(global_cfloat + global_cdouble);
    
    /* 7. String operations */
    for (int i = 0; error_messages[i] != NULL; i++) {
        result += (int)error_messages[i][0];  /* Use first char */
    }
    
    /* 8. Use packed and aligned structs */
    struct PackedStruct packed = { 'Z', 42, 99, 3.14 };
    result += packed.a + packed.b + (int)packed.d;
    
    struct AlignedStruct aligned;
    for (int i = 0; i < 16; i++) {
        aligned.data[i] = i * i;
        result += aligned.data[i];
    }
    
    /* 9. Create and use a linked structure */
    struct Node node1 = { .id = 1, .name = "Node1" };
    struct Node node2 = { .id = 2, .name = "Node2" };
    struct Node node3 = { .id = 3, .name = "Node3" };
    
    node1.next = &node2;
    node2.next = &node3;
    node3.next = NULL;
    
    node1.children[0] = &node2;
    node1.children[1] = &node3;
    
    /* Traverse the structure */
    struct Node* current = &node1;
    while (current) {
        result += current->id;
        if (current->name) {
            result += (int)current->name[0];
        }
        current = current->next;
    }
    
    /* 10. Final hash computation */
    int final_hash = compute_hash();
    result += final_hash;
    
    /* Ensure all types are referenced to prevent optimization */
    volatile struct opaque* opaque_ptr = NULL;
    volatile struct incomplete* inc_ptr = NULL;
    (void)opaque_ptr;
    (void)inc_ptr;
    
    printf("Final result: %d\n", result);
    return result % 256;  /* Return deterministic value */
}

/* Additional functions to ensure type usage */
void __attribute__((noinline)) ensure_type_usage(void) {
    /* Force usage of all types through a function that won't be inlined */
    struct WithAnonymousUnion anon_union = { .type = 1, .int_value = 42 };
    union TaggedUnion tagged = { .tagged = { .tag = 0, .data = { .i = 100 } } };
    union TransparentUnion transparent = { .int_ptr = &global_int };
    
    /* Use them */
    volatile int temp = anon_union.int_value + tagged.tagged.data.i + *transparent.int_ptr;
    (void)temp;
    
    /* Reference the forward declarations */
    extern struct opaque* get_opaque(void);
    extern struct incomplete* get_incomplete(void);
}
