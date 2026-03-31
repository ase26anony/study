/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_vector_shuffle(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * - 4 source vectors (a, b, c, d)
     * - 6 immediate control values for shuffle indices
     * Total: 10 operands
     */
    
    /* Create a complex shuffle pattern with multiple control values */
    v4si result;
    
    /* Use __builtin_shuffle with multiple vectors and immediate control */
    /* The compiler may expand this into a multi-operand operation */
    result = __builtin_shuffle(a, b, (v4si){0, 4, 2, 6});  /* 4 operands */
    
    /* Mix with another shuffle using different control values */
    v4si temp = __builtin_shuffle(c, d, (v4si){1, 5, 3, 7});  /* 4 more operands */
    
    /* Final blend/shuffle combining results with more control values */
    /* This could potentially require 10 operands during expansion */
    result = __builtin_shuffle(result, temp, (v4si){0, 1, 4, 5});  /* 6 operands total */
    
    return result;
}

/* Vector conversion with multiple operands */
static v4sf vector_conversion_chain(v4si a, v4si b, v4si c, v4si d) {
    /* Chain of conversions and operations that may require many operands */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    v4sf fc = __builtin_convertvector(c, v4sf);
    v4sf fd = __builtin_convertvector(d, v4sf);
    
    /* Complex expression with multiple constants - may expand to many operands */
    /* 10 operands: fa, 3.14f, fb, 2.71f, fc, 1.41f, fd, 1.0f, 0.5f, result */
    v4sf result = fa * (v4sf){3.14f, 3.14f, 3.14f, 3.14f} +
                  fb * (v4sf){2.71f, 2.71f, 2.71f, 2.71f} +
                  fc * (v4sf){1.41f, 1.41f, 1.41f, 1.41f} +
                  fd * (v4sf){1.0f, 1.0f, 1.0f, 1.0f} +
                  (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
    
    return result;
}

/* Complex bitwise operations with many constants */
static v4si multi_mask_operations(v4si x) {
    /* Series of bitwise operations with distinct mask constants */
    /* Each operation adds more operands to the potential expansion */
    
    /* 10 operands could be: x, mask1, mask2, mask3, mask4, mask5, 
     * intermediate results, and final result */
    v4si result = x;
    
    /* Apply multiple mask operations with different constants */
    result = result & (v4si){0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00};
    result = result | (v4si){0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF};
    result = result ^ (v4si){0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA};
    result = result & (v4si){0x55555555, 0x55555555, 0x55555555, 0x55555555};
    result = result | (v4si){0x33333333, 0x33333333, 0x33333333, 0x33333333};
    
    return result;
}

/* Custom vector permutation that might require 10 operands */
static v4si custom_vector_permute(v4si a, v4si b, v4si c, v4si d) {
    /* Attempt to create a pattern that needs:
     * - 4 source vectors
     * - 6 control indices (immediate values)
     * Total: 10 operands for expansion
     */
    
    /* Use volatile to prevent optimization */
    volatile v4si control = {0, 4, 1, 5};
    
    /* Complex expression that might expand to 10 operands */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 4, 2, 6});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){1, 5, 3, 7});
    
    /* Final shuffle with the volatile control - might trigger complex expansion */
    v4si result = __builtin_shuffle(temp1, temp2, control);
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    /* Initialize vectors with non-constant values to prevent optimization */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    /* Volatile to ensure computation isn't optimized away */
    volatile v4si result1, result2, result3, result4;
    volatile v4sf fresult;
    
    /* Loop to ensure execution but prevent excessive unrolling */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        result1 = complex_vector_shuffle(vec1, vec2, vec3, vec4);
        result2 = multi_mask_operations(vec1);
        result3 = custom_vector_permute(vec1, vec2, vec3, vec4);
        fresult = vector_conversion_chain(vec1, vec2, vec3, vec4);
        
        /* Modify vectors slightly to prevent loop invariant removal */
        vec1[0] += i;
        vec2[1] += i;
        vec3[2] += i;
        vec4[3] += i;
    }
    
    /* Use results to create side effects */
    printf("Results: %d %d %d %d\n", 
           result1[0], result2[1], result3[2], (int)fresult[3]);
    
    return 0;
}
