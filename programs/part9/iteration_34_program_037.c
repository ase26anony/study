/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle function requiring many operands */
static v4si complex_shuffle_10op(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * - 4 source vectors (a, b, c, d)
     * - 6 immediate control values for indices
     * Total: 10 operands for expansion
     */
    
    /* Create a data-dependent control mask using bit operations */
    v4si mask1 = a & 0x3;          /* Lower 2 bits as indices 0-3 */
    v4si mask2 = b & 0xC;          /* Next 2 bits as indices 4-7 */
    v4si mask3 = c & 0x30;         /* Next 2 bits as indices 8-11 */
    v4si mask4 = d & 0xC0;         /* Next 2 bits as indices 12-15 */
    
    /* Combine masks - each operation adds complexity */
    v4si combined_mask = mask1 | (mask2 >> 2) | (mask3 >> 4) | (mask4 >> 6);
    
    /* Force use of multiple constants in shuffle-like operation */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 1, 2, 3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){2, 3, 0, 1});
    
    /* Complex expression with many operands - designed to require
     * expansion with 10 operands during RTL generation */
    v4si result = (temp1 + 0x12345678) & 0x0F0F0F0F;
    result = result | ((temp2 + 0x87654321) & 0xF0F0F0F0);
    
    /* Additional shuffle with immediate control */
    result = __builtin_shuffle(result, combined_mask, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Vector conversion with many operands */
static v4sf vector_conversion_10op(v4si a, v4si b, v4sf c, v4sf d) {
    /* Mix integer and float vectors with conversions */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    
    /* Complex FMA-like expression with multiple constants */
    v4sf result = fa * 3.14159f + fb * 2.71828f;
    result = result + c * 1.41421f + d * 1.73205f;
    
    /* Additional operations with constants */
    result = result * 0.5f + 1.0f;
    result = __builtin_shuffle(result, result, (v4si){1, 0, 3, 2});
    
    return result;
}

/* Atomic-style operation simulation */
static v2di atomic_style_10op(v2di a, v2di b, v2di c, v2di d) {
    /* Simulate complex atomic update pattern */
    v2di mask1 = (v2di){0xFFFFFFFF00000000ULL, 0x00000000FFFFFFFFULL};
    v2di mask2 = (v2di){0x0000FFFF0000FFFFULL, 0xFFFF0000FFFF0000ULL};
    
    /* Multi-step operation designed to expand to many operands */
    v2di temp = a & mask1;
    temp = temp | (b & mask2);
    temp = temp ^ c;
    temp = temp + d;
    
    /* Cross-lane operations */
    v2di swapped = __builtin_shuffle(temp, temp, (v2di){1, 0});
    v2di result = temp + swapped;
    
    /* Final mix with constants */
    result = result & (v2di){0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL};
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    volatile int iterations = 1;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 100) iterations = 100;
    }
    
    /* Initialize vectors with non-constant data to prevent optimization */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec1 = {argc * 1.0f, argc * 1.1f, argc * 1.2f, argc * 1.3f};
    v4sf fvec2 = {argc * 1.4f, argc * 1.5f, argc * 1.6f, argc * 1.7f};
    
    v2di lvec1 = {argc * 100LL, argc * 200LL};
    v2di lvec2 = {argc * 300LL, argc * 400LL};
    v2di lvec3 = {argc * 500LL, argc * 600LL};
    v2di lvec4 = {argc * 700LL, argc * 800LL};
    
    v4si int_result = {0};
    v4sf float_result = {0};
    v2di long_result = {0};
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that should trigger 10-operand expansions */
        int_result = complex_shuffle_10op(vec1, vec2, vec3, vec4);
        float_result = vector_conversion_10op(vec1, vec2, fvec1, fvec2);
        long_result = atomic_style_10op(lvec1, lvec2, lvec3, lvec4);
        
        /* Mix results to create data dependencies */
        vec1 = vec1 + int_result;
        fvec1 = fvec1 + float_result;
        lvec1 = lvec1 + long_result;
    }
    
    /* Use results to prevent dead code elimination */
    int sum = int_result[0] + int_result[1] + int_result[2] + int_result[3];
    float fsum = float_result[0] + float_result[1] + float_result[2] + float_result[3];
    long long lsum = long_result[0] + long_result[1];
    
    printf("Results: %d, %f, %lld\n", sum, fsum, lsum);
    
    return sum > 0 ? 0 : 1;
}
