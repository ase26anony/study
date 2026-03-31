/* Complex type declarations to trigger consume_balanced parsing */
#include "complex_types.h"
#include <stdio.h>

/* Function pointer with deeply nested signature */
int (*(*global_callback)(int (*)(float, double[2]), 
                         struct Outer*))[5];

/* Multi-dimensional array with nested initializer */
int tensor[2][3][4] = {
    {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    },
    {
        {13, 14, 15, 16},
        {17, 18, 19, 20},
        {21, 22, 23, 24}
    }
};

/* Structure with anonymous nested structs and bit-fields */
struct Outer outer_instance = {
    .c = 42,
    .func_ptr_arr = {NULL, NULL}
};

/* Typedef with parentheses grouping */
typedef int (*array_of_funcs[5])(char, ...);
array_of_funcs func_array;

/* GCC attributes with nested parentheses */
int __attribute__((aligned(32))) 
    __attribute__((format(printf, 2, 3)))
    my_printf(void *stream, const char *fmt, ...);

/* Conditional macro generating delimiter-heavy code */
#ifdef MAKE_COMPLEX
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*complex_var##n)[n])(char (*)[n][2*n], \
                                struct { \
                                    int a : n; \
                                    int b : 32-n; \
                                } *)

MAKE_COMPLEX_TYPE(4) var4;
MAKE_COMPLEX_TYPE(8) var8;
#endif

/* Compound literal in expression context */
void process_matrix(int (*mat)[3][4]) {
    /* Do nothing, just for parsing */
    (void)mat;
}

/* sizeof with complex type */
size_t get_complex_size(void) {
    return sizeof(int (*(*)[10])(int (*)(float)));
}

/* Cast expression with nested parentheses */
void* complex_cast = (void*)(int (*)(int (*(*))(double)))0xDEADBEEF;

/* Nested switch with initializers (triggers braces) */
int nested_switch_example(int x) {
    switch (x) {
        case 1: {
            int arr[3] = {1, {2, 3}, 4};
            return arr[0];
        }
        case 2: {
            struct { int a; int b; } s = {.a = 1, .b = 2};
            return s.a + s.b;
        }
        default:
            return -1;
    }
}

/* Main function - minimal but uses complex types */
int main(void) {
    /* Use sizeof on complex array type */
    size_t sz = sizeof(int [2][3][4][5]);
    printf("Size: %zu\n", sz);
    
    /* Pass compound literal */
    process_matrix(&(int [2][3][4]){{{0}}});
    
    /* Use nested switch */
    int result = nested_switch_example(1);
    printf("Result: %d\n", result);
    
    /* Access global complex type */
    global_callback = NULL;
    
    /* sizeof with function pointer type */
    printf("Func ptr size: %zu\n", 
           sizeof(int (*(*)(void))[10]));
    
    return 0;
}
