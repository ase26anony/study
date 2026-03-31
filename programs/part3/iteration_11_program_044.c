/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_extern;  /* TYPE_UNDEFINED */
extern int incomplete_array[];   /* TYPE_UNDEFINED for array size */

/* Forward declarations creating incomplete types */
struct forward_declared_struct;  /* TYPE_UNDEFINED */
union forward_declared_union;    /* TYPE_UNDEFINED */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused)) char* string_literal = "Hello, gengtype!";
__attribute__((unused)) const char* const const_string = "Constant string";
__attribute__((unused)) volatile char* volatile volatile_string_ptr;

/* ========== TYPE_STRUCT definitions ========== */
/* Simple struct with __attribute__((annotate("gengtype"))) */
struct __attribute__((annotate("gengtype"))) simple_struct {
    int x;
    float y;
    char z;
};

/* Complex nested struct with various type members */
struct __attribute__((annotate("gengtype"))) complex_struct {
    /* TYPE_SCALAR members */
    _Bool flag;
    int counter;
    
    /* TYPE_POINTER members */
    void* generic_ptr;
    const int* const_ptr_to_int;
    
    /* TYPE_ARRAY members */
    int fixed_array[10];
    char string_array[32];
    
    /* TYPE_UNION member */
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } data_union;
    
    /* TYPE_CALLBACK member */
    int (*callback)(int, float);
    
    /* Pointer to forward declared struct (TYPE_UNDEFINED in pointer) */
    struct forward_declared_struct* fwd_ptr;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct __attribute__((annotate("gengtype"))) {
    int id;
    char name[64];
    struct complex_struct* nested;
} user_struct_t;

/* Another typedef struct with volatile qualifiers */
typedef volatile const struct {
    volatile int sensor_value;
    const char* const label;
} sensor_data_t;

/* ========== TYPE_UNION definitions ========== */
union __attribute__((annotate("gengtype"))) data_union {
    int int_val;
    float float_val;
    double double_val;
    void* ptr_val;
    char str_val[16];
};

/* Union containing struct */
union nested_union {
    struct {
        int x, y;
    } point;
    struct {
        float r, g, b, a;
    } color;
    unsigned long bits;
};

/* ========== TYPE_POINTER declarations ========== */
__attribute__((unused)) int* int_ptr;
__attribute__((unused)) float* float_ptr;
__attribute__((unused)) volatile const int* const volatile_const_ptr;
__attribute__((unused)) struct simple_struct* struct_ptr;
__attribute__((unused)) union data_union* union_ptr;
__attribute__((unused)) user_struct_t* user_struct_ptr;
__attribute__((unused)) void (*function_ptr)(void);

/* Triple pointer for complexity */
__attribute__((unused)) int*** triple_ptr;

/* ========== TYPE_ARRAY declarations ========== */
/* Fixed-size arrays */
__attribute__((unused)) int fixed_int_array[100];
__attribute__((unused)) struct simple_struct struct_array[5];
__attribute__((unused)) union data_union union_array[8];

/* Multi-dimensional arrays */
__attribute__((unused)) int matrix[3][3];
__attribute__((unused)) char string_table[4][32];

/* Array of pointers */
__attribute__((unused)) int* pointer_array[20];
__attribute__((unused)) void (*callback_array[10])(void);

/* Array with volatile qualifier */
__attribute__((unused)) volatile int volatile_array[50];

/* ========== TYPE_CALLBACK declarations ========== */
/* Simple function pointer */
__attribute__((unused)) int (*simple_callback)(int, char*);

/* Complex callback signature with __attribute__ */
typedef int (*complex_callback_t)(
    struct complex_struct*, 
    union data_union, 
    const char*, 
    ...
) __attribute__((annotate("gengtype")));

/* Callback returning pointer to struct */
typedef struct simple_struct* (*struct_factory_t)(int, float);

/* Callback taking array parameter */
typedef void (*array_processor_t)(int[], size_t);

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* Using GCC extension: vector types (may trigger TYPE_LANG_STRUCT) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== Combined complex type ========== */
/* Struct containing array of function pointers (combining STRUCT, ARRAY, CALLBACK) */
struct master_container {
    /* Array of callbacks */
    complex_callback_t callbacks[5];
    
    /* Pointer to array of structs */
    user_struct_t* (*get_records)(void);
    
    /* Union with pointer to different types */
    union {
        int* ints;
        float* floats;
        struct complex_struct* structs;
    } data_source;
    
    /* Nested struct with volatile member */
    struct {
        volatile int status;
        const char* name;
    } metadata;
};

/* ========== Function using __builtin_types_compatible_p ========== */
static void perform_type_comparisons(void) {
    /* Compare scalar types */
    int is_int_compatible = __builtin_types_compatible_p(int, float);  /* false */
    int is_bool_int = __builtin_types_compatible_p(_Bool, int);        /* false */
    
    /* Compare pointer types */
    int is_ptr_compatible = __builtin_types_compatible_p(int*, float*); /* false */
    int is_struct_ptr_compatible = __builtin_types_compatible_p(
        struct simple_struct*, 
        struct complex_struct*
    ); /* false */
    
    /* Compare struct vs union */
    int is_struct_union = __builtin_types_compatible_p(
        struct simple_struct,
        union data_union
    ); /* false */
    
    /* Compare array types */
    int is_array_compatible = __builtin_types_compatible_p(
        int[10],
        int[20]
    ); /* false */
    
    /* Compare function pointer types */
    int is_callback_compatible = __builtin_types_compatible_p(
        int(*)(int),
        void(*)(void)
    ); /* false */
    
    /* Compare typedef with underlying type */
    int is_typedef_compatible = __builtin_types_compatible_p(
        user_struct_t,
        struct { int id; char name[64]; struct complex_struct* nested; }
    ); /* implementation-defined */
    
    /* Suppress unused variable warnings */
    (void)is_int_compatible;
    (void)is_bool_int;
    (void)is_ptr_compatible;
    (void)is_struct_ptr_compatible;
    (void)is_struct_union;
    (void)is_array_compatible;
    (void)is_callback_compatible;
    (void)is_typedef_compatible;
}

/* ========== Function using sizeof with incomplete types ========== */
static size_t get_incomplete_sizes(void) {
    /* sizeof with pointer to incomplete type is valid */
    size_t ptr_size = sizeof(struct forward_declared_struct*);
    size_t extern_ptr_size = sizeof(struct undefined_extern*);
    
    /* sizeof incomplete array (GCC extension) */
    size_t incomplete_array_size = sizeof(incomplete_array);
    
    return ptr_size + extern_ptr_size + incomplete_array_size;
}

/* ========== Callback function implementations ========== */
static int example_callback(struct complex_struct* cs, union data_union du, const char* str, ...) {
    (void)cs; (void)du; (void)str;
    return 0;
}

static struct simple_struct* create_struct(int x, float y) {
    static struct simple_struct instance;
    instance.x = x;
    instance.y = y;
    instance.z = 'A';
    return &instance;
}

/* ========== Main function ========== */
int main(void) {
    /* Declare variables of our complex types */
    struct complex_struct cs_instance = {
        .flag = 1,
        .counter = 100,
        .generic_ptr = NULL,
        .const_ptr_to_int = &int_scalar,
        .fixed_array = {0},
        .string_array = "Test",
        .data_union = {.as_int = 42},
        .callback = NULL,
        .fwd_ptr = NULL
    };
    
    user_struct_t user_instance = {
        .id = 1,
        .name = "Test User",
        .nested = &cs_instance
    };
    
    union data_union du_instance;
    du_instance.int_val = 0xDEADBEEF;
    
    struct master_container container = {
        .callbacks = {example_callback, NULL, NULL, NULL, NULL},
        .get_records = NULL,
        .data_source = {.ints = fixed_int_array},
        .metadata = {.status = 0, .name = "Container"}
    };
    
    /* Initialize vector types */
    v4si vector_int = {1, 2, 3, 4};
    v4sf vector_float = {1.0f, 2.0f, 3.0f, 4.0f};
    (void)vector_int;
    (void)vector_float;
    
    /* Use function pointers */
    complex_callback_t cb = example_callback;
    struct_factory_t factory = create_struct;
    
    /* Call type comparison function */
    perform_type_comparisons();
    
    /* Use sizeof with various types */
    size_t total_size = 
        sizeof(cs_instance) +
        sizeof(user_instance) +
        sizeof(du_instance) +
        sizeof(container) +
        sizeof(cb) +
        sizeof(factory);
    
    /* Get sizes of incomplete types */
    total_size += get_incomplete_sizes();
    
    /* Use volatile variables to prevent optimization */
    volatile size_t result = total_size;
    
    /* Call a function via pointer (trivial example) */
    if (factory) {
        struct simple_struct* s = factory(10, 20.5f);
        result += s->x;
    }
    
    /* Return the result to prevent dead code elimination */
    return (int)(result % 256);
}
