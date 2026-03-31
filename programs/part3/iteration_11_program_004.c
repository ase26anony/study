/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;           /* TYPE_UNDEFINED */
extern int undefined_array[];             /* TYPE_UNDEFINED array */
struct forward_declared;                  /* Forward declaration */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused)) char* string_literal = "Hello, gengtype!";
__attribute__((unused)) const char* const const_string = "Constant string";
__attribute__((unused)) volatile char* volatile volatile_string_ptr;

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
};

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    int id;
    char tag;
} user_struct_t;

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might appear through extensions */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
};

/* ========== Complex nested type constructs ========== */
typedef int (*callback_func_t)(int, void*);  /* TYPE_CALLBACK */

struct complex_nested {
    /* TYPE_ARRAY of TYPE_CALLBACK */
    callback_func_t handlers[5];
    
    /* TYPE_POINTER to TYPE_UNION */
    union annotated_union* union_ptr;
    
    /* Nested TYPE_STRUCT */
    struct inner_struct {
        int counter;
        /* TYPE_ARRAY with volatile elements */
        volatile short sensor_readings[8];
    } inner;
    
    /* TYPE_UNION within struct */
    union {
        int option_a;
        float option_b;
        /* TYPE_POINTER to incomplete type */
        struct forward_declared* fwd_ptr;
    } choice;
};

/* ========== Array types (TYPE_ARRAY) ========== */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) int variable_array[];
__attribute__((unused)) const int const_array[5] = {1, 2, 3, 4, 5};
__attribute__((unused)) volatile float volatile_array[3];

/* Multi-dimensional arrays */
__attribute__((unused)) int matrix[3][4];
__attribute__((unused)) char* string_array[] = {"one", "two", "three"};

/* ========== Pointer types (TYPE_POINTER) ========== */
__attribute__((unused)) int* int_ptr;
__attribute__((unused)) float* float_ptr;
__attribute__((unused)) char** string_ptr_ptr;
__attribute__((unused)) volatile const int* const volatile_const_ptr;
__attribute__((unused)) struct annotated_struct* struct_ptr;
__attribute__((unused)) union annotated_union* union_ptr;
__attribute__((unused)) void (*func_ptr)(void);
__attribute__((unused)) int (*array_ptr)[10];  /* Pointer to array */

/* Complex pointer declaration */
__attribute__((unused)) const volatile int* const restrict volatile complex_ptr;

/* ========== Function pointers (TYPE_CALLBACK) with annotation ========== */
__attribute__((annotate("gengtype"))) 
int (*annotated_callback)(char*, int);

/* Callback with complex signature */
typedef void (*complex_callback_t)(
    struct complex_nested*,
    union annotated_union*,
    callback_func_t
);

/* ========== Type comparison expressions ========== */
/* These may trigger type classification during compilation */
static int type_comparisons __attribute__((unused)) = 
    __builtin_types_compatible_p(int, float) +
    __builtin_types_compatible_p(int*, float*) +
    __builtin_types_compatible_p(struct annotated_struct*, union annotated_union*) +
    __builtin_types_compatible_p(int[10], int*) +
    __builtin_types_compatible_p(callback_func_t, void*) +
    __builtin_types_compatible_p(user_struct_t, struct annotated_struct);

/* ========== Function using the types ========== */
static void use_types(void) {
    /* Force usage of various types to prevent dead code elimination */
    struct complex_nested nested __attribute__((unused));
    complex_callback_t cb __attribute__((unused));
    
    /* Use sizeof with various types (valid with incomplete types for pointers) */
    size_t sizes[] __attribute__((unused)) = {
        sizeof(struct undefined_struct*),
        sizeof(int*),
        sizeof(callback_func_t),
        sizeof(user_struct_t),
        sizeof(fixed_array),
        sizeof(matrix[0])
    };
    
    /* Initialize function pointer */
    __attribute__((unused)) int (*local_callback)(int, void*) = NULL;
    
    /* Use volatile to prevent optimization */
    volatile int vol __attribute__((unused)) = int_scalar;
}

/* ========== Main function ========== */
int main(void) {
    /* Declare and initialize variables of our complex types */
    struct annotated_struct my_struct = {.x = 1, .y = 2.0f, .name = "test"};
    union annotated_union my_union = {.as_int = 42};
    user_struct_t my_user_struct = {.id = 100, .tag = 'A'};
    
    /* Array of function pointers */
    callback_func_t callbacks[3] = {NULL, NULL, NULL};
    
    /* Pointer to volatile const */
    volatile const int* vc_ptr = &int_scalar;
    
    /* Use the types in trivial operations */
    int_scalar += my_struct.x;
    my_union.as_float = float_scalar;
    
    /* Call the type usage function */
    use_types();
    
    /* Use a function pointer (even if NULL) */
    __attribute__((unused)) complex_callback_t test_cb = NULL;
    
    /* Return based on initialized values (not important for coverage) */
    return (my_struct.x > 0) ? 0 : 1;
}

/* Additional incomplete type definitions */
struct forward_declared {
    int data;
    struct undefined_struct* undefined_ptr;  /* Pointer to undefined type */
};

/* Global variable with complex type */
__attribute__((unused)) 
volatile struct complex_nested* global_complex_ptr = NULL;
