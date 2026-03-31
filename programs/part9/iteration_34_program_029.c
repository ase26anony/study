/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This should generate a pattern requiring 10 operands:
     * 4 source vectors + 6 immediate control values
     */
    v4si result;
    
    /* Use __builtin_shuffle with multiple vectors and complex control pattern
     * The control pattern uses 6 immediate values to select from 4 vectors
     */
    result = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    
    /* Additional shuffle with different vectors and control pattern */
    v4si temp = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Combine results with bitwise operations using multiple constants */
    result = (result & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000}) |
             (temp & (v4si){0xFF000000, 0xFF0000, 0xFF00, 0xFF});
    
    return result;
}

/* Vector conversion with many operands */
static v4sf vector_conversion_complex(v2di src1, v2di src2, v4si control) {
    /* Complex conversion pattern that may expand to many operands */
    v2df dbl1 = __builtin_convertvector(src1, v2df);
    v2df dbl2 = __builtin_convertvector(src2, v2df);
    
    /* Create a 4-element float vector from two 2-element double vectors
     * This may require complex expansion with many operands
     */
    v4sf result;
    
    /* Use shuffle to interleave results - this creates a pattern that
     * may require many operands during expansion
     */
    v2df temp1 = __builtin_shuffle(dbl1, dbl2, (v2di){0, 2});
    v2df temp2 = __builtin_shuffle(dbl1, dbl2, (v2di){1, 3});
    
    /* Convert to float with scaling factors - multiple constants involved */
    v4sf f1 = __builtin_convertvector(temp1, v4sf);
    v4sf f2 = __builtin_convertvector(temp2, v4sf);
    
    /* Complex combination with multiple constants */
    result = f1 * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} + 
             f2 * (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    return result;
}

/* Fused multiply-add with many constants */
static v4sf complex_fma_pattern(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Expression designed to require many operands during expansion:
     * 4 input vectors + 6 constant vectors = 10 operands
     */
    v4sf result;
    
    /* Complex expression with multiple constants - may trigger 10-operand case */
    result = a * (v4sf){1.1f, 2.2f, 3.3f, 4.4f} +
             b * (v4sf){0.5f, 1.0f, 1.5f, 2.0f} +
             c * (v4sf){0.25f, 0.5f, 0.75f, 1.0f} +
             d * (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    
    /* Additional shuffle to ensure complex pattern */
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Atomic-like operation simulation with many arguments */
static v4si atomic_style_operation(v4si mem, v4si val, v4si mask) {
    /* Simulate complex atomic operation pattern that may require many operands */
    v4si result;
    
    /* Complex bitwise operation with multiple constants */
    result = (mem & (v4si){0xFFFF0000, 0x0000FFFF, 0xFF00FF00, 0x00FF00FF}) |
             (val & (v4si){0x0000FFFF, 0xFFFF0000, 0x00FF00FF, 0xFF00FF00});
    
    /* Additional masking with immediate values */
    result = result & mask;
    result = result | (v4si){0x00000001, 0x00000010, 0x00000100, 0x00001000};
    
    /* Final shuffle with immediate control */
    result = __builtin_shuffle(result, result, (v4si){1, 0, 3, 2});
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    volatile int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 10) iterations = 10;
    }
    
    /* Initialize vectors with different patterns */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    v4si vec4 = {13, 14, 15, 16};
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fvec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2di dvec1 = {100, 200};
    v2di dvec2 = {300, 400};
    
    v4si mask = {0xFF00FF00, 0x00FF00FF, 0xF0F0F0F0, 0x0F0F0F0F};
    
    /* Volatile results to prevent optimization */
    volatile v4si int_result;
    volatile v4sf float_result;
    
    /* Loop to ensure execution but prevent excessive unrolling */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that should trigger complex expansions */
        int_result = complex_shuffle_10_operands(vec1, vec2, vec3, vec4);
        
        float_result = vector_conversion_complex(dvec1, dvec2, mask);
        
        float_result = complex_fma_pattern(fvec1, fvec2, fvec3, fvec4);
        
        int_result = atomic_style_operation(vec1, vec2, mask);
        
        /* Modify inputs slightly to prevent constant propagation */
        vec1[0] += i;
        fvec1[0] += (float)i;
    }
    
    /* Print results to create side effects */
    printf("Results: %d %d %d %d\n", 
           int_result[0], int_result[1], int_result[2], int_result[3]);
    printf("Float results: %f %f %f %f\n",
           float_result[0], float_result[1], float_result[2], float_result[3]);
    
    return 0;
}
