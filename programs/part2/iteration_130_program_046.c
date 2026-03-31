/* Complex type declarations with deeply nested delimiters */
/* This should trigger consume_balanced calls in gengtype-parse.cc */

#include "complex_types.h"
#include <stdio.h>

/* Preprocessor macro to generate delimiter-heavy code */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])
#define NESTED_ARRAY(n) int arr##n[n][n][n]

/* Function pointer with complex signature - triggers '(' and ')' */
int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array with initializer - triggers '{' and '}' */
int matrix[3][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12}
};

/* Structure with nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
};

/* Type definition with parentheses for grouping */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Complex function pointer type */
typedef void (*(*signal_handler_t)(int, void (*(*)(int))(int)))(int);

/* Nested array type in sizeof context */
size_t get_size(void) {
    return sizeof(int[10][20]);
}

/* Compound literal with nested braces */
struct Point3D {
    float x, y, z;
};

/* Function with GCC attributes (nested parentheses) */
void debug_print(const char *format, ...) 
    __attribute__((format(printf, 1, 2)))
    __attribute__((aligned(32)));

/* Complex declaration with all delimiter types */
struct Container {
    /* Parentheses in function pointer */
    int (*compare)(const void *, const void *);
    
    /* Brackets in array */
    int data[100];
    
    /* Braces in nested struct */
    struct {
        int flags;
        union {
            struct {
                short x, y;
            };
            long position;
        };
    } metadata;
};

/* Macro-generated types */
MAKE_COMPLEX_TYPE(5);
MAKE_COMPLEX_TYPE(10);

/* Array of function pointers returning pointers to arrays */
int (*(*func_array[3])(int))[4];

/* Initialize with compound literal */
struct Container global_container = {
    .compare = NULL,
    .data = {[0] = 1, [99] = 100},
    .metadata = {
        .flags = 0,
        .position = 0xDEADBEEF
    }
};

/* Function using complex cast expression */
void *complex_cast(void) {
    return (void *(*[2])(int, ...)){NULL, NULL};
}

/* Nested switch with initializers (triggers braces) */
int process_value(int val) {
    switch (val) {
        case 1: {
            int nested[3] = {1, 2, 3};
            return nested[0];
        }
        case 2: {
            struct { int a, b; } s = {.a = 1, .b = 2};
            return s.a + s.b;
        }
        default: {
            int (*func)(void) = NULL;
            return 0;
        }
    }
}

/* Main function - minimal but uses complex types */
int main(void) {
    /* Declare variables of complex types */
    array_of_5_funcs funcs = {NULL};
    signal_handler_t handler = NULL;
    
    /* Use sizeof on complex array type */
    size_t s1 = sizeof(int[10][20]);
    size_t s2 = sizeof(int (*[5])(char, ...));
    
    /* Compound literal */
    struct Point3D points[] = {
        {.x = 1.0, .y = 2.0, .z = 3.0},
        {.x = 4.0, .y = 5.0, .z = 6.0},
        {.x = 7.0, .y = 8.0, .z = 9.0}
    };
    
    /* Nested array access */
    int value = matrix[1][2];
    
    /* Process with switch */
    int result = process_value(1);
    
    printf("Test program compiled successfully.\n");
    printf("Matrix[1][2] = %d\n", value);
    printf("Size of int[10][20] = %zu\n", s1);
    printf("Result = %d\n", result);
    
    return 0;
}

/* Function definitions */
void debug_print(const char *format, ...) {
    /* Implementation not needed for parsing test */
}

/* Additional complex type in function scope */
void test_local_types(void) {
    /* Local struct with bitfields */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } local_bitfield = {.a = 1, .b = 2, .c = 3};
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*local_funcs[2])(int))[3];
    
    /* Nested initializer */
    int nested_init[2][3][4] = {
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
}
