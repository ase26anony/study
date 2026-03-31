/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

/* ==================== TYPE_UNDEFINED ==================== */
struct opaque;  /* Forward declaration - undefined type */
extern struct opaque *global_opaque_ptr;

/* ==================== TYPE_STRUCT ==================== */
/* Simple struct */
struct Point {
    int x;
    int y;
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
    unsigned int padding : 24;
};

/* Struct with flexible array member */
struct FlexArray {
    size_t length;
    int data[];
};

/* Anonymous struct inside struct */
struct Container {
    struct {
        int a;
        char b;
    } inner;
    union {
        long x;
        double y;
    } data;
    volatile int counter;
};

/* ==================== TYPE_USER_STRUCT ==================== */
typedef struct Point Point_t;
typedef struct Rectangle Rect_t;

/* Complex typedef chain */
typedef Rect_t* RectPtr_t;
typedef RectPtr_t* RectDoublePtr_t;

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
    double as_double;
    _Complex float as_complex;
};

/* Tagged union */
struct TaggedVariant {
    enum { INT, FLOAT, STRING, COMPLEX } tag;
    union {
        int i;
        float f;
        char* s;
        _Complex double c;
    } value;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*simple_callback)(void);
typedef int (*processor_func)(int, char*);
typedef void (*event_handler)(int, void*);
typedef union Variant* (*variant_generator)(int);

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);
    void (*process)(int);
    event_handler on_event;
    variant_generator generate;
};

/* More complex callback signature */
typedef int (*(*callback_factory)(int))(int, int);

/* ==================== TYPE_ARRAY & TYPE_POINTER ==================== */
/* Multi-dimensional arrays */
int matrix_2d[10][10];
float matrix_3d[5][5][5];

/* Array of structs */
struct Point point_array[100];
struct Plugin plugins[5];

/* Array of pointers */
struct Rectangle* rect_ptrs[50];
int* int_ptr_array[20];

/* Pointer to array */
int (*ptr_to_array)[10];

/* Multi-level indirection */
int*** triple_ptr;

/* Complex pointer/array combination */
struct Node* adjacency_matrix[10][10];

/* Function with array parameter */
void process_matrix(int rows, int cols, float matrix[rows][cols]) {
    volatile float sum = 0.0f;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum += matrix[i][j];
        }
    }
}

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC vector types */
typedef float __attribute__((vector_size(32))) float32x8_t;
typedef int __attribute__((vector_size(16))) int32x4_t;

/* Packed struct */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Aligned struct */
struct __attribute__((aligned(64))) AlignedStruct {
    double data[8];
    long metadata;
};

/* Transparent union */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int* as_int_ptr;
    void* as_void_ptr;
} TransparentUnion_t;

/* ==================== TYPE_SCALAR ==================== */
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

/* Complex types */
_Complex float complex_float;
_Complex double complex_double;
_Complex long double complex_ldouble;

/* ==================== TYPE_STRING ==================== */
/* String literals and pointers */
const char* global_string = "Global string constant";
char mutable_string[] = "Mutable string";
const char* error_messages[] = {"Error", "Warning", "Info", "Debug"};
char* string_array[10];

/* ==================== Function Implementations ==================== */

/* Callback implementations */
int plugin1_init(void) {
    return 42;
}

void plugin1_process(int value) {
    volatile int result = value * 2;
}

void plugin1_event(int id, void* data) {
    volatile int processed = id + *(int*)data;
}

union Variant* plugin1_generate(int seed) {
    static union Variant v;
    v.as_int = seed * 2;
    return &v;
}

int complex_processor(int a, char* str) {
    return a + (int)strlen(str);
}

int (*create_callback(int base))(int, int) {
    static int multiplier = base;
    int callback(int x, int y) {
        return x * y + multiplier;
    }
    return callback;
}

/* Vector operations */
float32x8_t add_vectors(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* Union processing */
void process_union(union Variant* v, int type) {
    switch (type % 4) {
        case 0:
            v->as_int = 100;
            break;
        case 1:
            v->as_double = 3.14159;
            break;
        case 2:
            v->as_ptr = &global_string;
            break;
        case 3:
            v->as_complex = 1.0f + 2.0f * I;
            break;
    }
}

/* Multi-level pointer traversal */
int traverse_pointers(int*** ptr, int depth) {
    if (depth == 0) return **(*ptr);
    return traverse_pointers((int***)*ptr, depth - 1);
}

/* ==================== Global Variables ==================== */
struct Plugin plugin_registry[3] = {
    {"Plugin1", plugin1_init, plugin1_process, plugin1_event, plugin1_generate},
    {"Plugin2", NULL, NULL, NULL, NULL},
    {"Plugin3", NULL, NULL, NULL, NULL}
};

struct Container global_container = {
    .inner = {10, 'A'},
    .data = {.x = 1000L},
    .counter = 0
};

union SimpleUnion global_unions[5];
struct TaggedVariant global_variants[3];

/* ==================== Main Function ==================== */
int main(void) {
    /* 1. Initialize struct and union variables */
    struct Point p1 = {10, 20};
    struct Rectangle rect = {{0, 0}, {100, 100}, 1};
    struct BitFieldStruct bfs = {1, 3, 5, 1000};
    
    union Variant v1;
    process_union(&v1, 0);
    
    struct TaggedVariant tv = {INT, {.i = 42}};
    global_variants[0] = tv;
    
    /* 2. Use arrays of structs with function pointers */
    for (int i = 0; i < 3; i++) {
        if (plugin_registry[i].init) {
            int result = plugin_registry[i].init();
            volatile int unused = result; /* Prevent optimization */
        }
    }
    
    /* Call function through pointer */
    plugin_registry[0].process(42);
    
    /* 3. Use GCC vector types */
    float32x8_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_sum = add_vectors(vec1, vec2);
    volatile float first_element = ((float*)&vec_sum)[0];
    
    /* 4. Traverse multi-dimensional arrays and pointer chains */
    int value = 999;
    int* ptr1 = &value;
    int** ptr2 = &ptr1;
    triple_ptr = &ptr2;
    
    int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    ptr_to_array = matrix;
    
    /* Initialize adjacency matrix */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Simulate pointer assignment */
            adjacency_matrix[i][j] = NULL;
        }
    }
    
    /* 5. Process union types with runtime conditions */
    for (int i = 0; i < 5; i++) {
        process_union(&v1, i);
        global_unions[i].as_int = i * 10;
    }
    
    /* 6. Use all scalar types */
    char_var = 'Z';
    schar_var = -128;
    uchar_var = 255;
    short_var = -32768;
    ushort_var = 65535;
    int_var = -2147483648;
    uint_var = 4294967295U;
    long_var = -2147483648L;
    ulong_var = 4294967295UL;
    llong_var = -9223372036854775807LL;
    ullong_var = 18446744073709551615ULL;
    float_var = 3.14159f;
    double_var = 2.718281828459045;
    ldouble_var = 1.618033988749895L;
    bool_var = 1;
    
    complex_float = 1.0f + 2.0f * I;
    complex_double = 3.0 + 4.0 * I;
    
    /* 7. String operations */
    mutable_string[0] = 'M';
    string_array[0] = mutable_string;
    string_array[1] = "Literal string";
    
    const char** msg_ptr = error_messages;
    volatile const char* first_msg = *msg_ptr;
    
    /* 8. Complex callback factory */
    callback_factory factory = create_callback;
    int (*produced_callback)(int, int) = factory(10);
    volatile int callback_result = produced_callback(3, 4);
    
    /* 9. Use packed and aligned structs */
    struct PackedStruct packed = {'X', 1000, 50};
    struct AlignedStruct aligned = {{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}, 999};
    
    /* 10. Flexible array member simulation */
    struct FlexArray* flex = malloc(sizeof(struct FlexArray) + 10 * sizeof(int));
    flex->length = 10;
    for (size_t i = 0; i < flex->length; i++) {
        flex->data[i] = (int)(i * i);
    }
    
    /* 11. Transparent union */
    int transparent_data = 42;
    TransparentUnion_t tu;
    tu.as_int_ptr = &transparent_data;
    void* void_ptr = tu.as_void_ptr;
    volatile int transparent_value = *(int*)void_ptr;
    
    /* Compute deterministic return value using all manipulated data */
    int hash = 0;
    hash += p1.x + p1.y;
    hash += rect.id;
    hash += bfs.flag1 + bfs.flag2 + bfs.flag3;
    hash += v1.as_int;
    hash += (int)char_var;
    hash += int_var % 1000;
    hash += (int)float_var;
    hash += (int)first_element;
    hash += callback_result;
    hash += flex->data[3];
    hash += transparent_value;
    
    /* Cleanup */
    free(flex);
    
    /* Return deterministic value based on all operations */
    return hash % 256;
}
