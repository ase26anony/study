/* test_rich_types.c - Comprehensive type coverage for GGC type state serialization */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Forward declaration - TYPE_UNDEFINED */
struct incomplete;                 /* Another forward declaration */
typedef struct opaque* opaque_ptr_t;

/* ========== TYPE_STRUCT / TYPE_USER_STRUCT ========== */
/* Complex nested structure with anonymous struct */
struct Container {
    struct {                       /* Anonymous struct */
        int a;
        char b;
        unsigned bitfield : 4;     /* Bit-field */
        unsigned : 4;              /* Unnamed bit-field */
    } inner;
    union {                        /* Nested union */
        long x;
        double y;
        void* ptr;
    } data;
    volatile int counter;          /* Volatile member */
    const char* name;              /* String pointer */
} __attribute__((packed, aligned(8)));

/* Another struct with flexible array member */
struct DynamicArray {
    size_t length;
    int elements[];                /* Flexible array member */
};

/* Struct with function pointer member */
struct Plugin {
    const char* name;
    int version;
    int (*init)(void* context);    /* Function pointer */
    void (*process)(int data);
    void (*cleanup)(struct Plugin* self);
} __attribute__((aligned(64)));

/* Recursive struct for linked list */
struct ListNode {
    void* data;
    struct ListNode* next;
    struct ListNode* prev;
};

/* ========== TYPE_UNION ========== */
/* Complex union with nested struct */
union Variant {
    int as_int;
    long as_long;
    double as_double;
    void* as_ptr;
    struct {                       /* Struct inside union */
        short len;
        char buf[32];              /* Fixed-size array */
    } as_string;
    struct {                       /* Another nested struct */
        int type;
        union {                    /* Union inside struct inside union */
            float f;
            int i;
        } value;
    } as_tagged;
};

/* Tagged union for type-safe variant */
typedef union {
    int i;
    float f;
    char* s;
    struct Container* c;
} AnyValue;

/* ========== TYPE_CALLBACK / Function Pointers ========== */
/* Typedef for complex function pointer */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*event_handler)(int event_id, void* user_data);
typedef union Variant* (*transformer_t)(union Variant*, int);

/* Struct with multiple callback types */
struct EventSystem {
    event_handler handlers[10];
    void* user_data[10];
    int (*register_handler)(int event, event_handler handler);
    void (*dispatch)(int event, void* data);
};

/* Function pointer returning function pointer */
typedef void (*(*factory_fn)(int))(void);

/* ========== TYPE_ARRAY / Multi-dimensional Arrays ========== */
/* Complex array declarations */
struct Node* adjacency_matrix[10][10];          /* 2D array of pointers */
int (*signal_processors[5])(float[][256], int**); /* Array of function pointers */
const char* error_messages[] = {"Error", "Warning", "Info", NULL}; /* String array */

/* Array of structs */
struct Plugin plugin_registry[3];

/* Array of unions */
union Variant variant_array[20];

/* Pointer to array */
int (*array_ptr)[10][20];

/* ========== TYPE_POINTER / Complex Pointer Chains ========== */
/* Multi-level pointers */
int*** triple_ptr;
struct Container**** container_ptr_ptr_ptr;
void (* volatile volatile_func_ptr)(void);  /* Volatile function pointer */

/* Pointer to flexible array member struct */
struct DynamicArray* (*create_dynamic_array)(size_t size);

/* ========== TYPE_LANG_STRUCT / GCC Extensions ========== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Struct with vector member */
struct VectorData {
    float32x8_t vec_data;
    int32x4_t int_vec;
    __attribute__((aligned(32))) double aligned_double;
};

/* Transparent union attribute */
typedef union __attribute__((transparent_union)) {
    int i;
    long l;
} TransparentUnion;

/* ========== TYPE_SCALAR ========== */
/* All fundamental scalar types */
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
_Complex float complex_float;
_Complex double complex_double;
_Complex long double complex_ldouble;
size_t size_var;
ptrdiff_t ptrdiff_var;
intptr_t intptr_var;
uintptr_t uintptr_var;

/* ========== TYPE_STRING ========== */
const char* string_literal = "Hello, GGC Type System!";
char string_array[] = "Test string array";
wchar_t wide_string[] = L"Wide string";
const char* const string_table[] = {"str1", "str2", "str3"};

/* ========== Function Definitions ========== */
/* Callback functions */
static int plugin_init(void* context) {
    (void)context;
    return 0;
}

static void plugin_process(int data) {
    volatile int x = data * 2;  /* Prevent optimization */
    (void)x;
}

static void plugin_cleanup(struct Plugin* self) {
    if (self) self->version = -1;
}

/* Function using vector types */
static float32x8_t vector_add(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Function with complex signature */
static union Variant* transform_variant(union Variant* v, int op) {
    if (v && op == 1) {
        v->as_int *= 2;
    }
    return v;
}

/* Comparator for qsort */
static int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

/* Function that uses all type categories */
static void process_complex_types(struct Container* containers, 
                                  union Variant* variants,
                                  struct Plugin* plugins,
                                  int count) {
    /* Use struct types */
    for (int i = 0; i < count; i++) {
        containers[i].inner.a = i;
        containers[i].data.x = i * 100L;
        containers[i].counter++;
    }
    
    /* Use union types */
    for (int i = 0; i < count; i++) {
        if (i % 2 == 0) {
            variants[i].as_int = i;
        } else {
            variants[i].as_double = i * 1.5;
        }
    }
    
    /* Use function pointers */
    for (int i = 0; i < count && i < 3; i++) {
        if (plugins[i].init) {
            plugins[i].init(&containers[i]);
        }
        if (plugins[i].process) {
            plugins[i].process(i);
        }
    }
    
    /* Use array of pointers */
    int* ptr_array[5];
    int values[5] = {5, 2, 8, 1, 9};
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &values[i];
    }
    
    /* Sort using function pointer */
    qsort(values, 5, sizeof(int), compare_ints);
    
    /* Use multi-dimensional array */
    static int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            sum += matrix[i][j];
        }
    }
    
    /* Use vector type */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    float32x8_t vec_result = vector_add(vec_a, vec_b);
    (void)vec_result;  /* Use result to prevent dead code elimination */
    
    /* Use string types */
    const char* msg = string_literal;
    char first_char = msg[0];
    (void)first_char;
}

/* Main function that exercises all types */
int main(void) {
    /* 1. Declare and initialize struct/union variables */
    struct Container containers[5];
    union Variant variants[5];
    struct VectorData vector_data;
    
    /* Initialize struct members */
    for (int i = 0; i < 5; i++) {
        containers[i].inner.a = i * 10;
        containers[i].inner.b = 'A' + i;
        containers[i].data.x = i * 1000L;
        containers[i].name = error_messages[i % 3];
        containers[i].counter = 0;
        
        variants[i].as_int = i * 100;
    }
    
    /* 2. Initialize plugin registry with function pointers */
    plugin_registry[0].name = "PluginA";
    plugin_registry[0].version = 1;
    plugin_registry[0].init = plugin_init;
    plugin_registry[0].process = plugin_process;
    plugin_registry[0].cleanup = plugin_cleanup;
    
    plugin_registry[1].name = "PluginB";
    plugin_registry[1].version = 2;
    plugin_registry[1].init = NULL;  /* Some may be NULL */
    plugin_registry[1].process = plugin_process;
    plugin_registry[1].cleanup = plugin_cleanup;
    
    /* 3. Use vector types */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    float32x8_t vec_sum = vector_add(vec1, vec2);
    vector_data.vec_data = vec_sum;
    
    /* 4. Complex pointer chain */
    int value = 42;
    int* ptr1 = &value;
    int** ptr2 = &ptr1;
    int*** ptr3 = &ptr2;
    triple_ptr = ptr3;
    
    /* 5. Process union with runtime condition */
    for (int i = 0; i < 5; i++) {
        if (i % 3 == 0) {
            variants[i].as_int = i * 1000;
        } else if (i % 3 == 1) {
            variants[i].as_double = i * 3.14159;
        } else {
            strncpy(variants[i].as_string.buf, "test", 31);
            variants[i].as_string.len = 4;
        }
    }
    
    /* 6. Multi-dimensional array traversal */
    int md_array[4][4];
    int* md_ptr = &md_array[0][0];
    for (int i = 0; i < 16; i++) {
        md_ptr[i] = i;
    }
    
    /* 7. Call function that uses all types */
    process_complex_types(containers, variants, plugin_registry, 5);
    
    /* 8. Compute deterministic return value using all manipulated data */
    int hash = 0;
    
    /* Hash from struct data */
    for (int i = 0; i < 5; i++) {
        hash += containers[i].inner.a;
        hash += containers[i].inner.b;
        hash ^= containers[i].data.x;
    }
    
    /* Hash from union data */
    for (int i = 0; i < 5; i++) {
        if (i % 3 == 0) {
            hash += variants[i].as_int;
        } else if (i % 3 == 1) {
            hash += (int)variants[i].as_double;
        } else {
            hash += variants[i].as_string.len;
        }
    }
    
    /* Hash from vector data (use first element) */
    hash += (int)vec_sum[0];
    
    /* Hash from pointer chain */
    hash += ***triple_ptr;
    
    /* Hash from multi-dimensional array */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            hash += md_array[i][j];
        }
    }
    
    /* Use string literals */
    hash += (int)strlen(string_literal);
    
    /* Ensure all scalar types are used */
    char_var = hash & 0xFF;
    int_var = hash;
    float_var = hash * 0.5f;
    double_var = hash * 0.25;
    bool_var = hash > 0;
    complex_float = hash + hash * I;
    
    /* Return deterministic value based on all type manipulations */
    return (hash & 0x7FFFFFFF);  /* Ensure non-negative return */
}

/* Additional global variables to ensure type visibility */
volatile struct Container global_container;
volatile union Variant global_variant;
struct EventSystem global_event_system;
float32x8_t global_vector __attribute__((used));
const char* const global_strings[] __attribute__((used)) = {
    "Global string 1",
    "Global string 2",
    "Global string 3"
};

/* Force inclusion of forward declared types */
struct opaque {
    int dummy;
    struct incomplete* next;
};

struct incomplete {
    struct opaque* back_ref;
    float data;
};
