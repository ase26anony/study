/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
/* This program defines a diverse set of type constructs to exercise all
   type categories in GCC's GGC type state serialization. */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declaration for undefined type */
struct opaque;  /* TYPE_UNDEFINED candidate */

/* GCC-specific vector type extension */
typedef float __attribute__((vector_size(32))) float32x8_t;  /* TYPE_LANG_STRUCT */

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*event_handler)(int, void*);  /* TYPE_CALLBACK */
typedef int (*comparator_fn)(const void*, const void*);  /* TYPE_CALLBACK */
typedef float (*transform_fn)(float32x8_t);  /* TYPE_CALLBACK */

/* Struct with function pointer members */
struct Plugin {  /* Will be TYPE_STRUCT or TYPE_USER_STRUCT */
    const char* name;
    int (*init)(void);  /* TYPE_CALLBACK member */
    void (*process)(int);  /* TYPE_CALLBACK member */
    event_handler on_error;  /* TYPE_CALLBACK member */
};

/* ========== TYPE_UNION ========== */
/* Complex union with nested anonymous struct */
union Variant {  /* TYPE_UNION */
    int as_int;
    void* as_ptr;
    float as_float;
    double as_double;
    struct {  /* Anonymous struct */
        short len;
        char buf[];  /* Flexible array member */
    } as_string;
    struct {
        int tag;
        union {  /* Nested anonymous union */
            long id;
            double precision;
        };
    } as_tagged;
};

/* ========== TYPE_STRUCT ========== */
/* Nested struct with bitfields */
struct Inner {  /* TYPE_STRUCT */
    int a;
    char b;
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* Padding */
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {  /* TYPE_STRUCT with attribute */
    char header[4];
    int count;
    short checksum;
};

/* Aligned struct */
struct __attribute__((aligned(64))) CacheLine {  /* TYPE_STRUCT with attribute */
    float data[16];
    int timestamp;
};

/* Main container struct */
struct Container {  /* TYPE_STRUCT */
    struct Inner inner;  /* Nested struct */
    union {  /* Anonymous union */
        long x;
        double y;
        void* z;
    } data;
    struct CacheLine* cache;  /* Pointer to aligned struct */
    volatile int counter;  /* Volatile to prevent optimization */
};

/* ========== TYPE_ARRAY & TYPE_POINTER ========== */
/* Multi-dimensional array of pointers */
struct Node* adjacency_matrix[10][10];  /* TYPE_ARRAY of TYPE_POINTER */

/* Array of structs */
struct Container containers[5];  /* TYPE_ARRAY of TYPE_STRUCT */

/* Pointer to array */
int (*array_ptr)[20];  /* TYPE_POINTER to TYPE_ARRAY */

/* Triple pointer */
int ***triple_ptr;  /* TYPE_POINTER chain */

/* Function with array parameter */
int (*signal_processor)(float[][256], int**);  /* TYPE_CALLBACK with TYPE_ARRAY params */

/* ========== TYPE_SCALAR ========== */
/* Use all fundamental scalar types */
char char_var;
short short_var;
int int_var;
long long_var;
long long long_long_var;
float float_var;
double double_var;
_Bool bool_var;

/* Complex types */
_Complex float complex_float;
_Complex double complex_double;

/* ========== TYPE_STRING ========== */
/* String literals and arrays */
const char* error_messages[] = {"Error", "Warning", "Info", NULL};  /* TYPE_ARRAY of TYPE_STRING */
char static_string[] = "Static string data";

/* ========== Function Implementations ========== */
/* Callback function implementations */
void sample_event_handler(int event, void* data) {
    *(int*)data = event * 2;
}

int plugin_init(void) {
    return 42;
}

void plugin_process(int value) {
    /* Do nothing */
}

float vector_transform(float32x8_t vec) {
    /* Simple vector addition */
    float32x8_t one = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    vec = vec + one;
    return ((float*)&vec)[0];
}

int compare_ints(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

/* Function using the vector type */
void process_vector(void) {
    float32x8_t vec = {0.0f};
    transform_fn tf = vector_transform;
    float result = tf(vec);
    (void)result;  /* Use result to avoid unused warning */
}

/* Function to simulate union usage */
void simulate_variant(union Variant *v, event_handler cb) {
    static int event_data = 100;
    
    if (v->as_int > 0) {
        v->as_float = v->as_int * 1.5f;
        if (cb) cb(v->as_int, &event_data);
    } else {
        v->as_double = -v->as_int * 2.0;
    }
}

/* Traverse multi-dimensional array */
int traverse_matrix(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Simulate access */
            if (adjacency_matrix[i][j] != NULL) {
                sum++;
            }
        }
    }
    return sum;
}

/* Process pointer chain */
int process_triple_pointer(void) {
    int value = 42;
    int *p1 = &value;
    int **p2 = &p1;
    triple_ptr = &p2;
    
    return ***triple_ptr;
}

/* ========== Global Variables ========== */
/* Ensure type visibility */
extern struct Plugin plugin_registry[3];
struct Plugin plugin_registry[3] = {
    {"plugin1", plugin_init, plugin_process, sample_event_handler},
    {"plugin2", NULL, NULL, NULL},
    {"plugin3", plugin_init, NULL, sample_event_handler}
};

/* Array of unions */
union Variant variants[4];

/* ========== Main Function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize structs and unions */
    for (int i = 0; i < 5; i++) {
        containers[i].inner.a = i * 10;
        containers[i].inner.b = 'A' + i;
        containers[i].inner.flag1 = i % 2;
        containers[i].inner.flag2 = i % 8;
        containers[i].data.x = i * 100L;
        containers[i].counter = i;
        result += containers[i].inner.a;
    }
    
    /* 2. Use function pointers */
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    if (plugin_registry[0].on_error) {
        int callback_data = 0;
        plugin_registry[0].on_error(5, &callback_data);
        result += callback_data;
    }
    
    /* 3. Use GCC vector type */
    process_vector();
    
    /* 4. Process union types */
    for (int i = 0; i < 4; i++) {
        variants[i].as_int = i * 25 - 50;  /* Mix positive and negative */
        simulate_variant(&variants[i], sample_event_handler);
        result += (int)variants[i].as_float;
    }
    
    /* 5. Use multi-dimensional array */
    result += traverse_matrix();
    
    /* 6. Process pointer chain */
    result += process_triple_pointer();
    
    /* 7. Use all scalar types */
    char_var = 'Z';
    short_var = 1000;
    int_var = 123456;
    long_var = 987654321L;
    long_long_var = 123456789012345LL;
    float_var = 3.14159f;
    double_var = 2.718281828459045;
    bool_var = 1;
    complex_float = 1.0f + 2.0fi;
    complex_double = 3.0 + 4.0i;
    
    result += char_var + short_var + int_var + (int)float_var + bool_var;
    
    /* 8. Use string types */
    result += (int)strlen(error_messages[0]);
    result += (int)strlen(static_string);
    
    /* 9. Use comparator callback */
    int nums[] = {5, 2, 8, 1, 9};
    comparator_fn cmp = compare_ints;
    /* Simulate qsort-like usage */
    for (int i = 0; i < 4; i++) {
        if (cmp(&nums[i], &nums[i+1]) > 0) {
            result++;
        }
    }
    
    /* 10. Use array pointer */
    int local_array[20];
    array_ptr = &local_array;
    for (int i = 0; i < 20; i++) {
        (*array_ptr)[i] = i * 3;
        result += (*array_ptr)[i];
    }
    
    /* Return deterministic result */
    return result % 256;  /* Ensure result fits in return value */
}
