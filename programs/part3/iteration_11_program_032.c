/* gengtype_test.c
 * 
 * This program is specifically designed to trigger the type classification
 * logic in gengtype.cc lines 182-213 by presenting a diverse set of type
 * declarations and constructs to GCC's internal type system.
 * 
 * Compilation recommendations:
 *   gcc -O1 -fdump-tree-gimple -frandom-seed=1 -c gengtype_test.c
 *   gcc -O2 -g -dA -c gengtype_test.c
 *   gcc -flto -O2 -fno-fat-lto-objects -c gengtype_test.c
 */

#include <stddef.h>

/* ==================== TYPE_UNDEFINED and incomplete types ==================== */

/* Forward declarations creating incomplete/undefined types */
extern struct undefined_struct;           /* TYPE_UNDEFINED */
extern union undefined_union;             /* TYPE_UNDEFINED */
extern int incomplete_array[];            /* TYPE_UNDEFINED array */

/* ==================== TYPE_SCALAR declarations ==================== */

static _Bool __attribute__((unused)) scalar_bool = 0;          /* TYPE_SCALAR */
static int __attribute__((unused)) scalar_int = 42;            /* TYPE_SCALAR */
static float __attribute__((unused)) scalar_float = 3.14f;     /* TYPE_SCALAR */
static volatile const long __attribute__((unused)) scalar_vc_long = 100L; /* TYPE_SCALAR with qualifiers */

/* ==================== TYPE_STRING declarations ==================== */

static const char* __attribute__((unused)) string_literal = "Hello"; /* TYPE_STRING */
static char* __attribute__((unused)) string_array[] = {"world", "test"}; /* TYPE_STRING array */

/* ==================== TYPE_STRUCT definitions ==================== */

/* Basic struct with __attribute__((annotate("gengtype"))) */
struct __attribute__((annotate("gengtype"))) basic_struct {
    int x;
    float y;
    char* name;  /* TYPE_STRING member */
};

/* Nested struct containing union */
struct __attribute__((annotate("gengtype"))) container_struct {
    struct basic_struct nested;
    int tag;
};

/* ==================== TYPE_USER_STRUCT (typedef struct) ==================== */

typedef struct __attribute__((annotate("gengtype"))) {
    double data;
    void* metadata;  /* TYPE_POINTER member */
} user_struct_t;  /* TYPE_USER_STRUCT */

/* ==================== TYPE_UNION definitions ==================== */

union __attribute__((annotate("gengtype"))) data_union {
    int as_int;
    float as_float;
    void* as_ptr;  /* TYPE_POINTER member */
    char as_str[32]; /* TYPE_ARRAY member */
};

/* Union containing struct pointer */
union __attribute__((unused)) complex_union {
    struct basic_struct* struct_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
    user_struct_t* user_struct_ptr;   /* TYPE_POINTER to TYPE_USER_STRUCT */
};

/* ==================== TYPE_POINTER declarations ==================== */

static int* __attribute__((unused)) pointer_to_int = &scalar_int;  /* TYPE_POINTER */
static volatile const int* const __attribute__((unused)) complex_pointer = (volatile const int* const)&scalar_int; /* Qualified TYPE_POINTER */
static struct basic_struct* __attribute__((unused)) struct_pointer = 0;  /* TYPE_POINTER to TYPE_STRUCT */
static union data_union* __attribute__((unused)) union_pointer = 0;      /* TYPE_POINTER to TYPE_UNION */

/* Pointer to incomplete type */
extern struct undefined_struct* __attribute__((unused)) undefined_ptr;  /* TYPE_POINTER to TYPE_UNDEFINED */

/* ==================== TYPE_ARRAY declarations ==================== */

/* Fixed-size array */
static int __attribute__((unused)) fixed_array[10] = {0};  /* TYPE_ARRAY */

/* Array of pointers */
static void* __attribute__((unused)) pointer_array[5] = {0};  /* TYPE_ARRAY of TYPE_POINTER */

/* Multi-dimensional array */
static float __attribute__((unused)) matrix[3][3];  /* TYPE_ARRAY of TYPE_ARRAY */

/* Array of structs */
static struct basic_struct __attribute__((unused)) struct_array[2];  /* TYPE_ARRAY of TYPE_STRUCT */

/* Variable-length array (in function scope) */
static void init_vla(void) __attribute__((unused));
static void init_vla(void) {
    int n = 5;
    int vla[n];  /* TYPE_ARRAY (VLA) */
    (void)vla;   /* Suppress unused warning */
}

/* ==================== TYPE_CALLBACK declarations ==================== */

/* Simple function pointer */
typedef int (*simple_callback_t)(int, float);  /* TYPE_CALLBACK */

/* Complex callback with __attribute__((annotate("gengtype"))) */
typedef void (*__attribute__((annotate("gengtype"))) complex_callback_t)(
    struct basic_struct*, 
    user_struct_t*, 
    int(*)(void)
);  /* TYPE_CALLBACK with nested TYPE_CALLBACK parameter */

/* Struct containing function pointer */
struct callback_container {
    simple_callback_t cb;  /* TYPE_CALLBACK member */
    int state;
};

/* ==================== TYPE_LANG_STRUCT simulation ==================== */

/* Use GCC extension to potentially create language-specific struct type */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
    void* context;
};  /* May be classified as TYPE_LANG_STRUCT */

/* ==================== Complex nested type constructs ==================== */

/* Struct containing array of function pointers */
struct __attribute__((annotate("gengtype"))) processor {
    complex_callback_t callbacks[4];  /* TYPE_ARRAY of TYPE_CALLBACK */
    union data_union storage;         /* TYPE_UNION member */
};

/* Typedef for complex nested type */
typedef struct {
    struct processor* proc;           /* TYPE_POINTER to TYPE_STRUCT */
    user_struct_t (*factory)(int);    /* TYPE_CALLBACK returning TYPE_USER_STRUCT */
    volatile const int* limits[2];    /* TYPE_ARRAY of TYPE_POINTER with qualifiers */
} system_config_t;  /* TYPE_USER_STRUCT */

/* Union with pointer to struct containing callback */
union master_union {
    system_config_t* config;          /* TYPE_POINTER to TYPE_USER_STRUCT */
    struct callback_container* cb_container; /* TYPE_POINTER to TYPE_STRUCT */
    void (*startup)(void);            /* TYPE_CALLBACK */
};

/* ==================== __builtin_types_compatible_p comparisons ==================== */

/* These expressions force GCC to compare and classify types internally */
static void type_comparisons(void) __attribute__((unused));
static void type_comparisons(void) {
    /* Compare scalar vs pointer */
    int scalar_vs_ptr = __builtin_types_compatible_p(int, int*);
    
    /* Compare struct vs union */
    int struct_vs_union = __builtin_types_compatible_p(struct basic_struct, union data_union);
    
    /* Compare pointer types with different qualifiers */
    int qual_comparison = __builtin_types_compatible_p(const int*, volatile int*);
    
    /* Compare array types */
    int array_comparison = __builtin_types_compatible_p(int[10], int[]);
    
    /* Compare callback types */
    int callback_comparison = __builtin_types_compatible_p(
        simple_callback_t, 
        int(*)(int, float)
    );
    
    /* Compare user struct with its underlying struct */
    int user_struct_comparison = __builtin_types_compatible_p(
        user_struct_t,
        struct { double d; void* p; }
    );
    
    /* Suppress unused variable warnings */
    (void)scalar_vs_ptr;
    (void)struct_vs_union;
    (void)qual_comparison;
    (void)array_comparison;
    (void)callback_comparison;
    (void)user_struct_comparison;
}

/* ==================== Variable declarations using all types ==================== */

/* Declare variables of all complex types */
static struct processor __attribute__((unused)) global_processor;
static system_config_t __attribute__((unused)) global_config;
static union master_union __attribute__((unused)) global_union;

/* Function pointer variables */
static simple_callback_t __attribute__((unused)) current_callback = 0;
static complex_callback_t __attribute__((unused)) complex_handler = 0;

/* ==================== main function ==================== */

int main(void) {
    /* Ensure all types are processed by using them in some way */
    
    /* Use scalar types */
    scalar_int = scalar_float + 1;
    
    /* Use string */
    const char* local_str = string_literal;
    (void)local_str;
    
    /* Initialize struct */
    struct basic_struct local_struct = {1, 2.0f, "test"};
    
    /* Use union */
    union data_union local_union;
    local_union.as_int = 42;
    
    /* Use pointer */
    int* local_ptr = &scalar_int;
    *local_ptr = 100;
    
    /* Use array */
    fixed_array[0] = scalar_int;
    
    /* Use function pointer */
    if (current_callback) {
        /* Would call if initialized */
    }
    
    /* Use nested types */
    global_processor.storage = local_union;
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    size_t ptr_size = sizeof(struct undefined_struct*);  /* Pointer to incomplete type */
    (void)ptr_size;
    
    /* Trigger type comparisons */
    type_comparisons();
    
    /* Call VLA initialization */
    init_vla();
    
    /* Return success */
    return 0;
}

/* ==================== Additional external declarations ==================== */

/* External incomplete array definition */
int incomplete_array[5];  /* Now defined, was TYPE_UNDEFINED */

/* Function using the complex callback type */
static void __attribute__((unused)) register_callback(complex_callback_t cb) {
    complex_handler = cb;
}

/* Empty function to potentially use as callback */
static int __attribute__((unused)) sample_callback(int a, float b) {
    return a + (int)b;
}
