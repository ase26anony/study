/* gengtype_test.c - Test program to trigger coverage in gengtype.cc */
#include <stddef.h>

/* ==================== TYPE_UNDEFINED & TYPE_LANG_STRUCT ==================== */
/* Forward declarations create incomplete/undefined types */
extern struct undefined_struct;           /* TYPE_UNDEFINED */
extern union undefined_union;             /* TYPE_UNDEFINED */
extern int undefined_array[];             /* TYPE_UNDEFINED array */
struct forward_declared_struct;           /* TYPE_UNDEFINED */
union forward_declared_union;             /* TYPE_UNDEFINED */

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types */
_Bool __attribute__((unused)) scalar_bool = 0;
int __attribute__((unused)) scalar_int = 42;
float __attribute__((unused)) scalar_float = 3.14f;
volatile const long __attribute__((unused)) scalar_volatile_const = 100L;

/* ==================== TYPE_STRING ==================== */
/* String literals and pointers */
char* __attribute__((unused)) string_literal = "Hello, gengtype!";
const char* __attribute__((unused)) const_string = "Constant string";
volatile char* __attribute__((unused)) volatile_string_ptr;

/* ==================== TYPE_STRUCT & TYPE_UNION ==================== */
/* Named struct and union with attributes */
struct __attribute__((annotate("gengtype"))) my_struct {
    int x;
    float y;
    char* str;
};

union __attribute__((annotate("gengtype"))) my_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Complex nested struct */
struct __attribute__((annotate("gengtype"))) container_struct {
    struct my_struct nested_struct;
    union my_union nested_union;
    volatile const int flags;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef struct creates TYPE_USER_STRUCT */
typedef struct __attribute__((annotate("gengtype"))) {
    double data[4];
    struct container_struct* link;
} user_struct_t;

/* Another typedef with qualifiers */
typedef volatile const struct my_struct vc_my_struct_t;

/* ==================== TYPE_POINTER ==================== */
/* Pointers to various types */
int* __attribute__((unused)) int_ptr = &scalar_int;
float* __attribute__((unused)) float_ptr = &scalar_float;
char** __attribute__((unused)) string_ptr_ptr = &string_literal;
struct my_struct* __attribute__((unused)) struct_ptr = 0;
union my_union* __attribute__((unused)) union_ptr = 0;
user_struct_t* __attribute__((unused)) user_struct_ptr = 0;

/* Complex pointer with qualifiers */
volatile const int* const __attribute__((unused)) complex_ptr = &scalar_int;
void* volatile __attribute__((unused)) volatile_void_ptr;

/* Pointer to incomplete type */
struct forward_declared_struct* __attribute__((unused)) fwd_ptr = 0;

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size arrays */
int __attribute__((unused)) fixed_array[10] = {0};
float __attribute__((unused)) multi_dim[3][4][5];

/* Array of pointers */
struct my_struct* __attribute__((unused)) struct_ptr_array[5];

/* Array with qualifiers */
const volatile int __attribute__((unused)) cv_array[7];

/* Variable-length array (in function scope) */

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct my_struct*, union my_union*, user_struct_t*);

/* Function pointer with attributes */
int (* __attribute__((annotate("gengtype"))) annotated_callback)(const char*, ...);

/* Array of function pointers */
simple_callback_t __attribute__((unused)) callback_array[3];

/* Struct containing function pointer */
struct callback_container {
    complex_callback_t handler;
    simple_callback_t validators[2];
};

/* ==================== NESTED TYPE CONSTRUCTS ==================== */
/* Ultimate nested type combining everything */
struct __attribute__((annotate("gengtype"))) mega_nested {
    /* Scalar */
    _Bool flag;
    
    /* String */
    char* name;
    
    /* Struct */
    struct container_struct container;
    
    /* Union */
    union my_union data_union;
    
    /* Pointer */
    user_struct_t* user_data;
    
    /* Array */
    int matrix[3][3];
    
    /* Array of function pointers */
    simple_callback_t callbacks[5];
    
    /* Pointer to array */
    float (*dynamic_matrix)[10];
    
    /* Function pointer */
    complex_callback_t processor;
    
    /* Pointer to incomplete type */
    struct forward_declared_struct* future;
};

/* Union with nested complex types */
union __attribute__((annotate("gengtype"))) complex_union {
    struct mega_nested as_nested;
    struct callback_container as_callbacks;
    void* as_opaque;
};

/* ==================== TYPE COMPARISONS ==================== */
/* Use __builtin_types_compatible_p to force type analysis */
static void type_comparisons(void) __attribute__((unused));
static void type_comparisons(void) {
    /* These comparisons trigger internal type classification */
    int scalar_vs_pointer = __builtin_types_compatible_p(int, int*);
    int struct_vs_union = __builtin_types_compatible_p(struct my_struct, union my_union);
    int ptr_vs_array = __builtin_types_compatible_p(int*, int[]);
    int callback_vs_ptr = __builtin_types_compatible_p(simple_callback_t, void*);
    int const_vs_nonconst = __builtin_types_compatible_p(const int, int);
    int volatile_vs_nonvolatile = __builtin_types_compatible_p(volatile int, int);
    
    /* Compare incomplete vs complete types */
    int incomplete_vs_complete = __builtin_types_compatible_p(
        struct forward_declared_struct*, 
        struct my_struct*
    );
    
    /* Compare typedef vs original */
    int typedef_vs_original = __builtin_types_compatible_p(
        user_struct_t,
        struct { double d[4]; struct container_struct*; }
    );
    
    /* Silence unused warnings */
    (void)scalar_vs_pointer;
    (void)struct_vs_union;
    (void)ptr_vs_array;
    (void)callback_vs_ptr;
    (void)const_vs_nonconst;
    (void)volatile_vs_nonvolatile;
    (void)incomplete_vs_complete;
    (void)typedef_vs_original;
}

/* ==================== FUNCTION DEFINITIONS ==================== */
/* Callback function implementations */
static int sample_callback(int a, float b) __attribute__((unused));
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static void complex_handler(struct my_struct* s, union my_union* u, user_struct_t* us) 
    __attribute__((unused));
static void complex_handler(struct my_struct* s, union my_union* u, user_struct_t* us) {
    if (s) s->x = 0;
    if (u) u->as_int = 0;
    if (us) us->data[0] = 0.0;
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Variable declarations with various types */
    volatile int __attribute__((unused)) local_scalar = 255;
    char __attribute__((unused)) local_string[] = "Local string";
    
    /* Struct and union instances */
    struct my_struct __attribute__((unused)) local_struct = {1, 2.0f, "test"};
    union my_union __attribute__((unused)) local_union;
    local_union.as_int = 42;
    
    /* User struct instance */
    user_struct_t __attribute__((unused)) local_user_struct = {
        .data = {1.0, 2.0, 3.0, 4.0},
        .link = 0
    };
    
    /* Nested struct instance */
    struct mega_nested __attribute__((unused)) nested = {
        .flag = 1,
        .name = "Nested",
        .container = {{0, 0.0f, 0}, {0}, 0},
        .data_union = {.as_int = 100},
        .user_data = &local_user_struct,
        .matrix = {{1,2,3},{4,5,6},{7,8,9}},
        .callbacks = {sample_callback, 0, 0, 0, 0},
        .dynamic_matrix = 0,
        .processor = complex_handler,
        .future = 0
    };
    
    /* Function pointer assignment */
    simple_callback_t __attribute__((unused)) local_callback = sample_callback;
    annotated_callback = (int (*)(const char*, ...))0;
    
    /* Array usage */
    fixed_array[0] = local_scalar;
    
    /* Pointer operations */
    int_ptr = &local_scalar;
    float_ptr = &local_struct.y;
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    size_t __attribute__((unused)) sz1 = sizeof(struct forward_declared_struct*);
    size_t __attribute__((unused)) sz2 = sizeof(undefined_array);
    
    /* Call function pointer */
    if (local_callback) {
        int __attribute__((unused)) result = local_callback(10, 20.5f);
    }
    
    /* Trigger type comparisons */
    type_comparisons();
    
    /* Ensure no dead code elimination */
    asm volatile("" : : "r"(&nested), "r"(&local_struct), "r"(&local_union));
    
    return 0;
}

/* ==================== EXTERNAL DEFINITIONS ==================== */
/* Define previously extern-declared types */
struct undefined_struct {
    int placeholder;
};

union undefined_union {
    long placeholder;
};

/* Define forward-declared types */
struct forward_declared_struct {
    struct mega_nested* connection;
    complex_callback_t handler;
};

union forward_declared_union {
    struct forward_declared_struct* s;
    user_struct_t* u;
};
