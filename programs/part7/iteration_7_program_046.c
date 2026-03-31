/* test_rich_types.c - Comprehensive type coverage for GCC GGC type state serialization */

#include <stddef.h>
#include <string.h>

/* ==================== TYPE_UNDEFINED ==================== */
struct opaque;  /* Forward declaration - undefined type */
extern struct opaque *global_opaque_ptr;

/* ==================== TYPE_SCALAR ==================== */
/* All fundamental scalar types */
char global_char = 'A';
short global_short = 100;
int global_int = 1000;
long global_long = 10000L;
long long global_longlong = 100000LL;
float global_float = 3.14159f;
double global_double = 2.718281828459045;
_Bool global_bool = 1;
_Complex float global_complex_float = 3.0f + 4.0fi;
_Complex double global_complex_double = 1.0 + 2.0i;

/* ==================== TYPE_STRING ==================== */
const char* global_string = "Global string literal";
const char* error_messages[] = {"Error", "Warning", "Info", NULL};
char mutable_string[] = "Mutable string";

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*simple_callback)(void);
typedef int (*processor_func)(int, const char*);
typedef void* (*allocator_func)(size_t);
typedef void (*event_handler)(int, void*);

/* Struct with function pointer members */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    event_handler on_event;
    processor_func transform;
};

/* Union with function pointer */
union FuncUnion {
    simple_callback cb;
    void (*alternate)(int);
};

/* ==================== TYPE_STRUCT ==================== */
/* Simple nested struct */
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
    int value : 8;
    unsigned int : 4;  /* Padding */
};

/* Anonymous struct inside struct */
struct Container {
    struct {
        int x;
        int y;
    } position;
    struct Inner inner;
    union {
        long as_long;
        double as_double;
    } data;
    struct BitFieldStruct flags;
};

/* Struct with flexible array member */
struct DynamicArray {
    size_t length;
    int data[];
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Aligned struct */
struct __attribute__((aligned(64))) AlignedStruct {
    double values[8];
    int count;
};

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Union with struct member */
union Variant {
    int as_int;
    void* as_ptr;
    struct {
        short len;
        char buf[];
    } as_string;
    struct {
        double real;
        double imag;
    } as_complex;
};

/* Anonymous union inside struct */
struct WithAnonymousUnion {
    int type;
    union {
        int i;
        float f;
        char *s;
    };
};

/* ==================== TYPE_ARRAY ==================== */
/* Multi-dimensional arrays */
int matrix_2d[10][10];
struct Inner struct_array[5][3];
struct Plugin* plugin_ptrs[4];

/* Array of pointers */
int* int_ptr_array[20];
struct Container* container_ptr_array[8];

/* Pointer to array */
int (*ptr_to_array)[10];

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Vector type struct */
struct VectorData {
    float32x8_t vec1;
    float32x8_t vec2;
    int32x4_t mask;
};

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer chains */
int ***triple_ptr;
struct Container**** quad_container_ptr;

/* Function returning pointer to array of function pointers */
int (*(*complex_func_ptr)(void))[5];

/* ==================== Helper Functions ==================== */
static int plugin1_init(void) {
    return 42;
}

static void plugin1_process(int value) {
    global_int += value;
}

static void plugin1_event(int id, void* data) {
    *(int*)data = id * 2;
}

static int plugin1_transform(int x, const char* str) {
    return x + (int)strlen(str);
}

static int dummy_processor(int val, const char* name) {
    return val * 2;
}

static void simple_callback_impl(void) {
    global_char = 'Z';
}

/* Initialize plugin registry */
struct Plugin plugin_registry[3] = {
    {"Plugin1", plugin1_init, plugin1_process, plugin1_event, plugin1_transform},
    {"Plugin2", NULL, NULL, NULL, dummy_processor},
    {"Plugin3", NULL, NULL, NULL, NULL}
};

/* Global variables using complex types */
struct Container global_container = {
    .position = {10, 20},
    .inner = {100, 'X', 3.14f},
    .data = {.as_long = 999999L},
    .flags = {1, 3, 5, 42}
};

union Variant global_variant;
struct VectorData global_vector_data;
struct WithAnonymousUnion global_anon_union = {1, {.i = 42}};

/* Array of structs with function pointers */
struct Plugin* active_plugins[2];

/* ==================== Main Function ==================== */
int main(void) {
    volatile int result = 0;  /* Prevent optimization */
    
    /* 1. Use scalar types */
    result += global_char;
    result += global_short;
    result += global_int;
    result += (int)global_long;
    result += (int)global_longlong;
    result += (int)global_float;
    result += (int)global_double;
    result += global_bool;
    
    /* 2. Use string types */
    result += (int)strlen(global_string);
    result += (int)strlen(error_messages[0]);
    result += mutable_string[0];
    
    /* 3. Initialize and use arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix_2d[i][j] = i * j;
            result += matrix_2d[i][j];
        }
    }
    
    /* 4. Use struct types */
    struct Container local_container = {
        .position = {1, 2},
        .inner = {50, 'Y', 2.718f},
        .data = {.as_double = 1.414},
        .flags = {0, 7, 15, 127}
    };
    
    result += local_container.position.x;
    result += local_container.inner.a;
    result += (int)local_container.data.as_double;
    
    /* 5. Use union types */
    union SimpleUnion su;
    su.as_int = 314159;
    result += su.as_int;
    
    global_variant.as_int = 271828;
    result += global_variant.as_int;
    
    /* 6. Use function pointers and callbacks */
    active_plugins[0] = &plugin_registry[0];
    active_plugins[1] = &plugin_registry[1];
    
    if (active_plugins[0]->init) {
        result += active_plugins[0]->init();
    }
    
    if (active_plugins[0]->process) {
        active_plugins[0]->process(10);
    }
    
    int event_data = 0;
    if (active_plugins[0]->on_event) {
        active_plugins[0]->on_event(5, &event_data);
        result += event_data;
    }
    
    if (active_plugins[0]->transform) {
        result += active_plugins[0]->transform(100, "test");
    }
    
    /* 7. Use GCC vector types */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f};
    float32x8_t vec_sum = vec_a + vec_b;
    
    /* Extract value from vector for result */
    float vec_result[8];
    memcpy(vec_result, &vec_sum, sizeof(vec_sum));
    result += (int)vec_result[0];
    
    global_vector_data.vec1 = vec_a;
    global_vector_data.vec2 = vec_b;
    
    /* 8. Complex pointer chains */
    int value = 42;
    int *ptr1 = &value;
    int **ptr2 = &ptr1;
    triple_ptr = &ptr2;
    
    result += ***triple_ptr;
    
    /* 9. Multi-dimensional pointer arrays */
    struct Container container1, container2;
    container_ptr_array[0] = &container1;
    container_ptr_array[1] = &container2;
    
    if (container_ptr_array[0]) {
        container_ptr_array[0]->inner.a = 999;
        result += container_ptr_array[0]->inner.a;
    }
    
    /* 10. Anonymous union access */
    switch (global_anon_union.type) {
        case 1:
            result += global_anon_union.i;
            break;
        case 2:
            result += (int)global_anon_union.f;
            break;
        default:
            if (global_anon_union.s) {
                result += global_anon_union.s[0];
            }
    }
    
    /* 11. Pointer to array */
    int local_array[10] = {0,1,2,3,4,5,6,7,8,9};
    ptr_to_array = &local_array;
    result += (*ptr_to_array)[5];
    
    /* 12. Use complex types with attributes */
    struct PackedStruct packed = {'A', 1234, 5678};
    result += packed.a + packed.b + packed.c;
    
    /* 13. Use all scalar types in operations */
    result += (int)(global_complex_float * global_complex_double);
    
    /* 14. Ensure all types are referenced */
    (void)global_opaque_ptr;  /* Reference undefined type */
    
    /* Return deterministic result based on all operations */
    return result % 256;  /* Ensure small, predictable return value */
}

/* Additional functions to increase type usage */
void process_container_array(struct Container* containers, int count) {
    for (int i = 0; i < count; i++) {
        containers[i].inner.a = i;
        containers[i].position.x = i * 2;
    }
}

union Variant create_variant(int type, int value) {
    union Variant v;
    if (type == 0) {
        v.as_int = value;
    } else {
        v.as_ptr = (void*)(long)value;
    }
    return v;
}

/* Function using all type categories */
void comprehensive_type_user(void) {
    /* Use all global variables */
    (void)global_container;
    (void)global_variant;
    (void)global_vector_data;
    (void)global_anon_union;
    
    /* Use arrays */
    (void)matrix_2d;
    (void)struct_array;
    (void)plugin_ptrs;
    
    /* Use pointers */
    (void)triple_ptr;
    (void)quad_container_ptr;
    (void)complex_func_ptr;
    
    /* Use callbacks */
    simple_callback cb = simple_callback_impl;
    if (cb) cb();
}
