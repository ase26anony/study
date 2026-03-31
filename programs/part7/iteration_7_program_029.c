/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ==================== TYPE_UNDEFINED / Forward declarations ==================== */
struct opaque;                     /* Incomplete type - TYPE_UNDEFINED */
struct forward_declared;           /* Another undefined type */

/* ==================== TYPE_SCALAR - All fundamental types ==================== */
typedef _Complex float complex_float;
typedef _Complex double complex_double;

/* ==================== TYPE_CALLBACK - Function pointers ==================== */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct opaque*, double, void*);
typedef void (*event_handler)(int, const char*, void*);

/* ==================== TYPE_STRUCT / TYPE_USER_STRUCT ==================== */

/* Simple nested struct */
struct Inner {
    int id;
    char tag;
    float data;
};

/* Struct with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int count : 10;
    signed int value : 16;
    unsigned int : 2;  /* Padding */
};

/* Anonymous struct inside a struct */
struct Container {
    struct {
        int x;
        int y;
    } position;  /* Anonymous struct member */
    
    struct {
        float temperature;
        float pressure;
    };  /* Truly anonymous member (C11) */
    
    char label[32];
};

/* Struct with flexible array member */
struct DynamicString {
    size_t length;
    char data[];  /* Flexible array member */
};

/* Complex user-defined struct with all features */
struct ComplexUserStruct {
    /* Basic scalars */
    int id;
    long counter;
    double value;
    _Bool active;
    
    /* Complex numbers */
    complex_float cf;
    complex_double cd;
    
    /* Nested structs */
    struct Inner inner;
    struct BitFieldStruct bits;
    
    /* Function pointer */
    simple_callback notify;
    
    /* Pointer to undefined type */
    struct opaque* opaque_ptr;
    
    /* Array member */
    int history[10];
    
    /* Volatile member */
    volatile int signal;
    
    /* Const member */
    const char* name;
    
    /* Alignment attribute */
} __attribute__((aligned(64)));

/* Packed struct */
struct PackedData {
    char type;
    int value;
    short index;
} __attribute__((packed));

/* ==================== TYPE_UNION ==================== */

/* Simple union */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Union with struct members */
union ComplexUnion {
    struct {
        short type;
        char data[6];
    } as_packet;
    
    struct {
        long key;
        double value;
    } as_entry;
    
    struct {
        int x, y, z;
    } as_vector;
};

/* Union with flexible array member */
union Variant {
    int as_int;
    double as_double;
    char* as_string;
    struct {
        size_t len;
        char buf[];
    } as_dynamic;
};

/* ==================== TYPE_LANG_STRUCT - GCC extensions ==================== */

/* GCC vector type */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Struct with vector types */
struct VectorizedData {
    float32x8_t vec1;
    float32x8_t vec2;
    int32x4_t mask;
    __attribute__((aligned(32))) float scalar;
};

/* Struct with transparent_union attribute */
union transparent_arg {
    int* ip;
    float* fp;
    void* vp;
} __attribute__((transparent_union));

/* ==================== TYPE_ARRAY / TYPE_POINTER ==================== */

/* Multi-dimensional arrays */
int matrix_2d[5][5];
float matrix_3d[3][3][3];

/* Array of structs */
struct Inner struct_array[20];

/* Array of pointers */
struct ComplexUserStruct* ptr_array[15];

/* Array of function pointers */
event_handler handlers[8];

/* Complex pointer types */
int**** quadruple_ptr;
struct Container** double_ptr_to_struct;
int (*func_ptr_array[5])(void);

/* Pointer to array */
int (*ptr_to_matrix)[5][5];

/* ==================== TYPE_STRING ==================== */

/* String literals */
const char* messages[] = {
    "Error: Invalid operation",
    "Warning: Deprecated feature",
    "Info: Processing complete",
    "Debug: Entering function"
};

const char* const fixed_string = "Constant string data";

/* ==================== Global variables ==================== */

/* Ensure all types are used globally */
struct ComplexUserStruct global_user_struct = {
    .id = 1001,
    .counter = 999999L,
    .value = 3.1415926535,
    .active = 1,
    .cf = 1.0 + 2.0*I,
    .cd = 3.0 - 4.0*I,
    .inner = {42, 'X', 2.718},
    .bits = {1, 7, 512, -16384},
    .notify = NULL,
    .opaque_ptr = NULL,
    .history = {0},
    .signal = 0,
    .name = "GlobalStruct"
};

union ComplexUnion global_union = {
    .as_packet = {1, "Data"}
};

struct VectorizedData global_vector_data;

/* Plugin registry with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    void (*cleanup)(void);
};

struct Plugin plugin_registry[3];

/* ==================== Function definitions ==================== */

/* Callback functions */
void sample_callback(int x) {
    volatile static int counter = 0;
    counter += x;
}

int complex_callback_impl(struct opaque* op, double d, void* ctx) {
    (void)op; (void)d; (void)ctx;
    return 42;
}

void event_handler_impl(int type, const char* msg, void* data) {
    (void)type; (void)msg; (void)data;
    /* Do nothing for test */
}

/* Plugin functions */
int plugin1_init(void) { return 0; }
void plugin1_process(int x) { global_user_struct.signal = x; }
void plugin1_cleanup(void) { }

int plugin2_init(void) { return 1; }
void plugin2_process(int x) { global_union.as_entry.key = x; }
void plugin2_cleanup(void) { }

/* Function using transparent union */
void process_transparent(union transparent_arg arg) {
    int* ip = arg.ip;
    if (ip) *ip = 100;
}

/* Function with complex parameter types */
void process_complex(struct ComplexUserStruct* s,
                     union ComplexUnion* u,
                     float32x8_t vec,
                     event_handler cb) {
    if (s && s->notify) {
        s->notify(s->id);
    }
    
    if (u->as_packet.type == 1) {
        /* Process packet */
    }
    
    /* Use vector type */
    float32x8_t result = vec + vec;
    (void)result;
    
    if (cb) {
        cb(1, "Processing", s);
    }
}

/* Function returning pointer to array */
int (*get_matrix_ptr(void))[5][5] {
    static int local_matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            local_matrix[i][j] = i * 5 + j;
        }
    }
    return &local_matrix;
}

/* ==================== main() - Exercise all types ==================== */

int main(void) {
    /* Initialize plugin registry */
    plugin_registry[0] = (struct Plugin){
        "Plugin1", plugin1_init, plugin1_process, plugin1_cleanup
    };
    plugin_registry[1] = (struct Plugin){
        "Plugin2", plugin2_init, plugin2_process, plugin2_cleanup
    };
    plugin_registry[2] = (struct Plugin){0};  /* Null plugin */
    
    /* Initialize arrays */
    for (int i = 0; i < 20; i++) {
        struct_array[i].id = i;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].data = i * 1.5f;
    }
    
    for (int i = 0; i < 15; i++) {
        ptr_array[i] = &global_user_struct;
    }
    
    /* Initialize handlers */
    handlers[0] = event_handler_impl;
    handlers[1] = NULL;
    
    /* Initialize matrix */
    ptr_to_matrix = get_matrix_ptr();
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix_2d[i][j] = (*ptr_to_matrix)[i][j];
        }
    }
    
    /* Initialize 3D matrix */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                matrix_3d[i][j][k] = i * 9 + j * 3 + k;
            }
        }
    }
    
    /* Set up function pointers */
    global_user_struct.notify = sample_callback;
    
    /* Call plugin functions */
    for (int i = 0; i < 2; i++) {
        if (plugin_registry[i].init) {
            plugin_registry[i].init();
        }
        if (plugin_registry[i].process) {
            plugin_registry[i].process(i * 10);
        }
    }
    
    /* Use vector types */
    float32x8_t vec_a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    float32x8_t vec_b = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    global_vector_data.vec1 = vec_a;
    global_vector_data.vec2 = vec_b;
    global_vector_data.vec1 = vec_a + vec_b;
    
    /* Complex union usage */
    union Variant var;
    int choice = matrix_2d[0][0] % 3;
    
    switch (choice) {
        case 0:
            var.as_int = 42;
            break;
        case 1:
            var.as_double = 3.14159;
            break;
        case 2:
            var.as_string = (char*)messages[0];
            break;
    }
    
    /* Pointer chain */
    int x = 10;
    int *p1 = &x;
    int **p2 = &p1;
    int ***p3 = &p2;
    quadruple_ptr = &p3;
    
    /* Process complex types */
    process_complex(&global_user_struct, &global_union, vec_a, handlers[0]);
    
    /* Use transparent union */
    int value = 0;
    union transparent_arg ta;
    ta.ip = &value;
    process_transparent(ta);
    
    /* Compute deterministic result from all manipulations */
    int result = 0;
    result += global_user_struct.id;
    result += global_user_struct.inner.id;
    result += global_union.as_packet.type;
    result += matrix_2d[2][2];
    result += (int)(*quadruple_ptr) ? 1 : 0;
    result += value;
    
    /* Ensure string types are referenced */
    const char* test_string = fixed_string;
    result += (int)test_string[0];
    
    /* Call callback if set */
    if (global_user_struct.notify) {
        global_user_struct.notify(result % 100);
    }
    
    /* Cleanup plugins */
    for (int i = 0; i < 2; i++) {
        if (plugin_registry[i].cleanup) {
            plugin_registry[i].cleanup();
        }
    }
    
    return result % 256;
}
