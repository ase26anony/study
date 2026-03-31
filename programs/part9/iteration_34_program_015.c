/* test_optabs.c - Targeting case 10: 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * 4 source vectors (a, b, c, d) = 4 operands
     * 6 immediate control values = 6 operands
     * Total: 10 operands
     * 
     * The pattern: Select elements based on complex condition
     * [a[0], b[1], c[2], d[3]] but with conditional swapping
     */
    
    /* Create control mask with 6 immediate values */
    v4si mask1 = {0, 4, 2, 6};  /* indices for first blend */
    v4si mask2 = {1, 5, 3, 7};  /* indices for second blend */
    v4si control = {0x0F, 0xF0, 0x0F, 0xF0}; /* blend control */
    
    /* Combine vectors into wider conceptual array */
    v4si combined_low = __builtin_shufflevector(a, b, 0, 1, 2, 3);
    v4si combined_high = __builtin_shufflevector(c, d, 0, 1, 2, 3);
    
    /* Complex permutation using multiple shuffles and blends */
    v4si shuffled1 = __builtin_shufflevector(combined_low, combined_high, 
                                             mask1[0], mask1[1], mask1[2], mask1[3]);
    v4si shuffled2 = __builtin_shufflevector(combined_low, combined_high,
                                             mask2[0], mask2[1], mask2[2], mask2[3]);
    
    /* Final blend using control mask - this may expand to 10 operands */
    v4si result = __builtin_shufflevector(shuffled1, shuffled2,
                                          (control[0] & 0x0F) ? 0 : 4,
                                          (control[1] & 0xF0) ? 5 : 1,
                                          (control[2] & 0x0F) ? 2 : 6,
                                          (control[3] & 0xF0) ? 7 : 3);
    
    return result;
}

/* Vector FMA-like operation with multiple constants */
static v4sf complex_fma_operation(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* FMA with multiple constants: a*C1 + b*C2 + c*C3 + d*C4
     * Each constant is a vector of 4 floats = 4 operands
     * 4 input vectors = 4 operands
     * Result vector = 1 operand (implicit)
     * Control/rounding mode = 1 operand
     * Total: 10 operands conceptually
     */
    
    /* Multiple constant vectors */
    const v4sf C1 = {3.14159f, 2.71828f, 1.41421f, 1.61803f};
    const v4sf C2 = {0.57721f, 1.73205f, 2.23607f, 2.64575f};
    const v4sf C3 = {0.69314f, 1.09861f, 1.60943f, 2.30258f};
    const v4sf C4 = {0.78539f, 1.04719f, 1.57079f, 3.14159f};
    
    /* Complex expression that may be expanded as single operation */
    v4sf result = a * C1 + b * C2 + c * C3 + d * C4;
    
    /* Additional operation to prevent simplification */
    result = __builtin_shufflevector(result, result, 3, 2, 1, 0);
    
    return result;
}

/* Atomic-like complex operation simulating 10 operands */
static v2di atomic_style_operation(v2di mem, v2di a, v2di b, v2di c, v2di d) {
    /* Simulate complex atomic update:
     * mem = ((mem & mask1) | (a & mask2)) ^ ((b << shift1) | (c >> shift2)) + d
     * Requires many operands during expansion
     */
    
    const v2di mask1 = {0xFFFFFFFF00000000LL, 0x00000000FFFFFFFFLL};
    const v2di mask2 = {0x00000000FFFFFFFFLL, 0xFFFFFFFF00000000LL};
    const int shift1 = 8;
    const int shift2 = 16;
    
    /* Complex bitwise expression */
    v2di temp1 = mem & mask1;
    v2di temp2 = a & mask2;
    v2di temp3 = b << shift1;
    v2di temp4 = c >> shift2;
    
    v2di result = (temp1 | temp2) ^ (temp3 | temp4) + d;
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec1 = {(float)argc, (float)argc/2.0f, (float)argc/3.0f, (float)argc/4.0f};
    v4sf fvec2 = {(float)argc * 1.1f, (float)argc * 1.2f, (float)argc * 1.3f, (float)argc * 1.4f};
    v4sf fvec3 = {(float)argc * 1.5f, (float)argc * 1.6f, (float)argc * 1.7f, (float)argc * 1.8f};
    v4sf fvec4 = {(float)argc * 1.9f, (float)argc * 2.0f, (float)argc * 2.1f, (float)argc * 2.2f};
    
    v2di lvec1 = {(long long)argc, (long long)argc * 100};
    v2di lvec2 = {(long long)argc * 200, (long long)argc * 300};
    v2di lvec3 = {(long long)argc * 400, (long long)argc * 500};
    v2di lvec4 = {(long long)argc * 600, (long long)argc * 700};
    v2di lmem = {(long long)argc * 800, (long long)argc * 900};
    
    /* Accumulator to prevent dead code elimination */
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    v2di long_result = {0LL, 0LL};
    
    /* Loop to ensure execution but prevent excessive unrolling */
    for (volatile int i = 0; i < iterations; i++) {
        /* Perform complex operations that may require 10-operand expansion */
        v4si shuffle_result = complex_shuffle_10_operand(vec1, vec2, vec3, vec4);
        v4sf fma_result = complex_fma_operation(fvec1, fvec2, fvec3, fvec4);
        v2di atomic_result = atomic_style_operation(lmem, lvec1, lvec2, lvec3, lvec4);
        
        /* Accumulate results */
        int_result += shuffle_result;
        float_result += fma_result;
        long_result += atomic_result;
        
        /* Modify inputs slightly to prevent loop invariant removal */
        vec1[0] += i;
        fvec1[0] += (float)i;
        lvec1[0] += i;
    }
    
    /* Use results to prevent elimination */
    int sum_int = int_result[0] + int_result[1] + int_result[2] + int_result[3];
    float sum_float = float_result[0] + float_result[1] + float_result[2] + float_result[3];
    long long sum_long = long_result[0] + long_result[1];
    
    printf("Results: int=%d, float=%.2f, long=%lld\n", 
           sum_int, sum_float, sum_long);
    
    return (sum_int > 0) ? 0 : 1;
}

/* Additional test targeting specific vector builtins */
void test_vector_builtins() {
    /* Using __builtin_ia32_* style builtins if available */
    /* These often require many operands including immediate constants */
    
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Complex shuffle with immediate control - may require 10 operands */
    /* shufps takes 2 vectors + 1 immediate = 3 operands, but expansion may need more */
    v4sf shuffled;
    
    /* Use inline assembly hint for specific shuffle patterns */
    asm volatile ("" : "=x"(shuffled) : "0"(a), "x"(b));
    
    /* Multiple shuffle operations chained */
    for (int i = 0; i < 4; i++) {
        /* Vary the shuffle control */
        int control = i * 0x55;
        
        /* This complex expression may be expanded as multi-operand operation */
        shuffled = __builtin_shufflevector(a, b, 
                                          (control >> 0) & 0x3,
                                          (control >> 2) & 0x3,
                                          (control >> 4) & 0x3,
                                          (control >> 6) & 0x3);
        
        a = shuffled;
    }
    
    /* Prevent dead code */
    volatile v4sf dummy = shuffled;
    (void)dummy;
}
