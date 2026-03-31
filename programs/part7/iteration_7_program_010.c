/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* ========== 1. User-Defined Structures and Unions ========== */

/* Forward declaration for undefined type */
struct opaque;  /* TYPE_UNDEFINED */

/* Complex nested structure */
struct Container {
    struct {
        int a;
        char b;
        unsigned bitfield1 : 3;
        unsigned bitfield2 : 5;
    } inner;
    union {
        long x;
        double y;
        void *ptr;
    } data;
    volatile int counter;
} __attribute__((packed));

/* Union with flexible array member */
union Variant {
    int as_int;
    void* as_ptr;
    struct {
        short len;
        char buf[];  /* Flexible array member */
    } as_string;
    long long as_llong;
};

/* Bitfield structure */
struct BitFieldStruct {
    unsigned a : 1;
    unsigned b : 2;
    unsigned c : 3;
    unsigned d : 26;
    signed e : 4;
};

/* Anonymous struct/union */
struct AnonymousExample {
    struct {
        int x, y;
    };  /* Anonymous struct */
    union {
        float f;
        double d;
    };  /* Anonymous union */
};

/* ========== 2. Function Pointers and Callbacks ========== */

/* Callback typedefs */
typedef void (*event_handler)(int, void*);  /* TYPE_CALLBACK */
typedef int (*comparator_fn)(const void*, const void*);
typedef double (*transform_fn)(double, double);

/* Complex function pointer signature */
typedef int (*(*factory_fn)(int))(float, char**);

/* Struct with function pointers */
struct Plugin {
    const char* name;
    int (*init)(void);           /* TYPE_CALLBACK */
    void (*process)(int);        /* TYPE_CALLBACK */
    void (*cleanup)(struct Plugin*);
    event_handler on_event;      /* TYPE_CALLBACK */
};

/* Array of function pointers */
static transform_fn math_ops[] = {
    (transform_fn)0,  /* Placeholder */
    (transform_fn)0
};

/* ========== 3. Arrays and Pointer Chains ========== */

/* Multi-dimensional array of structs */
struct Container matrix_3d[5][5][5];

/* Array of pointers to arrays */
int* ptr_array[10][10];

/* Complex pointer chain */
int ****quad_ptr;

/* Pointer to array */
int (*array_ptr)[20];

/* Function returning pointer to array */
int (*get_matrix(void))[10][10] {
    static int matrix[10][10];
    return matrix;
}

/* ========== 4. Language-Specific Types ========== */

/* GCC vector type */
typedef float __attribute__((vector_size(32))) float32x8_t;  /* TYPE_LANG_STRUCT */

/* Aligned structure */
struct AlignedData {
    char data[64];
} __attribute__((aligned(64)));

/* Transparent union */
typedef union __attribute__((transparent_union)) {
    int i;
    long l;
} TransparentUnion;

/* ========== 5. Scalar and String Types ========== */

/* All scalar types */
struct AllScalars {
    char c;
    signed char sc;
    unsigned char uc;
    short s;
    unsigned short us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;
    float f;
    double d;
    long double ld;
    _Bool b;
    float _Complex fc;     /* TYPE_SCALAR */
    double _Complex dc;    /* TYPE_SCALAR */
    void* vp;
};

/* String array */
const char* error_messages[] = {  /* TYPE_STRING */
    "Error",
    "Warning",
    "Info",
    NULL
};

/* Wide string */
const wchar_t* wide_str = L"Wide String";

/* ========== 6. Global Variables with Complex Types ========== */

/* Global instances */
struct Plugin plugin_registry[3];
union Variant global_variant;
struct Container global_container;
float32x8_t global_vector;

/* Volatile to prevent optimization */
volatile struct BitFieldStruct volatile_bf;

/* ========== Function Implementations ========== */

/* Callback implementations */
static int plugin_init_default(void) {
    return 0;
}

static void plugin_process_default(int x) {
    volatile int y = x * 2;
    (void)y;
}

static void event_handler_example(int event, void* data) {
    struct Container* c = (struct Container*)data;
    if (c) {
        c->counter++;
    }
}

/* Function using complex pointer chain */
static void process_pointer_chain(void) {
    int a = 1;
    int *p1 = &a;
    int **p2 = &p1;
    int ***p3 = &p2;
    quad_ptr = &p3;
    
    /* Access through chain */
    if (quad_ptr && *quad_ptr && **quad_ptr && ***quad_ptr && ****quad_ptr) {
        ****quad_ptr = 42;
    }
}

/* Function using vector type */
static void use_vector_type(void) {
    float32x8_t v1 = {0, 1, 2, 3, 4, 5, 6, 7};
    float32x8_t v2 = {7, 6, 5, 4, 3, 2, 1, 0};
    float32x8_t result = v1 + v2;  /* Vector addition */
    global_vector = result;
}

/* Process union variant */
static int process_variant(union Variant *v, int type) {
    switch (type) {
        case 0:
            return v->as_int * 2;
        case 1:
            return (int)(intptr_t)v->as_ptr;
        case 2:
            return v->as_string.len;
        default:
            return (int)v->as_llong;
    }
}

/* Traverse multi-dimensional array */
static int traverse_matrix(void) {
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                matrix_3d[i][j][k].counter = i + j + k;
                sum += matrix_3d[i][j][k].counter;
            }
        }
    }
    return sum;
}

/* Complex function pointer usage */
static int (*(*setup_factory(void))(int))(float, char**) {
    static int (*result(int x))(float, char**) {
        static int inner_func(float f, char** s) {
            return (int)f + (s ? (int)(intptr_t)*s : 0);
        }
        return inner_func;
    }
    return result;
}

/* ========== Main Function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize structs and unions */
    struct Container local_container = {
        .inner = { .a = 42, .b = 'X', .bitfield1 = 3, .bitfield2 = 7 },
        .data = { .x = 123456789L },
        .counter = 0
    };
    global_container = local_container;
    
    union Variant var;
    var.as_int = 100;
    global_variant = var;
    
    /* 2. Setup plugin registry with function pointers */
    plugin_registry[0] = (struct Plugin){
        .name = "TestPlugin",
        .init = plugin_init_default,
        .process = plugin_process_default,
        .cleanup = 0,
        .on_event = event_handler_example
    };
    
    /* Call through function pointer */
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    /* 3. Use vector type */
    use_vector_type();
    
    /* 4. Process pointer chains */
    process_pointer_chain();
    
    /* 5. Process union with different types */
    result += process_variant(&var, 0);
    
    var.as_ptr = &local_container;
    result += process_variant(&var, 1);
    
    /* 6. Traverse multi-dimensional array */
    result += traverse_matrix();
    
    /* 7. Use all scalar types */
    struct AllScalars scalars = {
        .c = 'A',
        .sc = -1,
        .uc = 255,
        .s = -32768,
        .us = 65535,
        .i = -2147483647-1,
        .ui = 4294967295U,
        .l = -1L,
        .ul = 1UL,
        .ll = -9223372036854775807LL-1,
        .ull = 18446744073709551615ULL,
        .f = 3.14159f,
        .d = 2.718281828459045,
        .ld = 1.4142135623730950488L,
        .b = 1,
        .fc = 1.0f + 2.0f * I,
        .dc = 3.0 + 4.0 * I,
        .vp = &result
    };
    
    /* Use scalars in computation */
    result += scalars.i + (int)scalars.c + (int)scalars.b;
    
    /* 8. Use string types */
    const char** msg_ptr = error_messages;
    while (*msg_ptr) {
        result += (int)**msg_ptr;  /* Sum first chars */
        msg_ptr++;
    }
    
    /* 9. Complex function pointer setup */
    factory_fn fn_factory = setup_factory();
    if (fn_factory) {
        int (*(*tmp)(int))(float, char**) = fn_factory;
        if (tmp) {
            int (*final_fn)(float, char**) = tmp(42);
            if (final_fn) {
                char* args[] = {"test", NULL};
                result += final_fn(3.14f, args);
            }
        }
    }
    
    /* 10. Use bitfield structure */
    volatile_bf.a = 1;
    volatile_bf.b = 2;
    volatile_bf.c = 3;
    volatile_bf.d = 0x3FFFFFF;
    volatile_bf.e = -4;
    result += volatile_bf.d & 0xFF;
    
    /* 11. Use opaque pointer */
    struct opaque* opaque_ptr = 0;
    (void)opaque_ptr;  /* TYPE_UNDEFINED usage */
    
    /* Return deterministic result based on all operations */
    return result & 0xFF;  /* Return lower 8 bits */
}

/* Additional global to ensure type visibility */
struct Container* global_container_ptr = &global_container;
union Variant* global_variant_ptr = &global_variant;
struct Plugin* global_plugin_ptr = plugin_registry;
