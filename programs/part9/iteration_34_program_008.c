/* test_optabs.c - Target coverage for optabs.cc lines 8254-8263 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually needs many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern uses multiple source vectors
     * and immediate control values to create a result that requires
     * many operands during RTL expansion */
    
    /* Step 1: Create intermediate shuffles with immediate masks */
    v4si t1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si t2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Step 2: Perform arithmetic with immediate constants */
    t1 = t1 + (v4si){1, 2, 3, 4};
    t2 = t2 * (v4si){2, 3, 2, 3};
    
    /* Step 3: Final shuffle combining all 4 source vectors
     * This complex expression may require expansion with many operands */
    v4si result = __builtin_shuffle(t1, t2, (v4si){0, 5, 2, 7});
    
    /* Additional operation with immediate to increase operand count */
    result = result & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    
    return result;
}

/* Vector blend with many immediate control bits */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blend operation that might expand to many operands */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4sf t2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Fused multiply-add style expression with multiple constants */
    v4sf result = t1 * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} + 
                  t2 * (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    /* Additional shuffle to potentially trigger complex expansion */
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Cryptographic-style permutation (conceptual) */
static v2di complex_permutation(v2di a, v2di b, v2di c, v2di d) {
    /* Simulate a complex permutation that might need many operands */
    v2di mask1 = (v2di){0xFFFFFFFF00000000ULL, 0x00000000FFFFFFFFULL};
    v2di mask2 = (v2di){0x0000FFFF0000FFFFULL, 0xFFFF0000FFFF0000ULL};
    
    v2di t1 = (a & mask1) | (b & ~mask1);
    v2di t2 = (c & mask2) | (d & ~mask2);
    
    /* Complex shuffle/permute with immediate control */
    v2di result = __builtin_shuffle(t1, t2, (v2di){0, 3});
    
    /* Additional operations with immediate constants */
    result = result ^ (v2di){0xAAAAAAAAAAAAAAAALL, 0x5555555555555555LL};
    result = result + (v2di){1, 2};
    
    return result;
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec1 = {(float)argc, (float)argc + 1.5f, (float)argc + 2.5f, (float)argc + 3.5f};
    v4sf fvec2 = {(float)argc * 1.1f, (float)argc * 1.2f, (float)argc * 1.3f, (float)argc * 1.4f};
    v4sf fvec3 = {(float)argc * 1.5f, (float)argc * 1.6f, (float)argc * 1.7f, (float)argc * 1.8f};
    v4sf fvec4 = {(float)argc * 1.9f, (float)argc * 2.0f, (float)argc * 2.1f, (float)argc * 2.2f};
    
    v2di lvec1 = {(long long)argc * 100, (long long)argc * 200};
    v2di lvec2 = {(long long)argc * 300, (long long)argc * 400};
    v2di lvec3 = {(long long)argc * 500, (long long)argc * 600};
    v2di lvec4 = {(long long)argc * 700, (long long)argc * 800};
    
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    v2di long_result = {0, 0};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call complex operations that may require many operands during expansion */
        int_result = complex_shuffle_10_operands(vec1, vec2, vec3, vec4);
        float_result = vector_blend_complex(fvec1, fvec2, fvec3, fvec4);
        long_result = complex_permutation(lvec1, lvec2, lvec3, lvec4);
        
        /* Modify inputs slightly to prevent loop invariant removal */
        vec1[0] += i;
        fvec1[0] += (float)i;
        lvec1[0] += i;
    }
    
    /* Use results to prevent elimination */
    int sum = int_result[0] + int_result[1] + int_result[2] + int_result[3];
    float fsum = float_result[0] + float_result[1] + float_result[2] + float_result[3];
    long long lsum = long_result[0] + long_result[1];
    
    printf("Results: %d, %.2f, %lld\n", sum, fsum, lsum);
    
    return sum > 0 ? 0 : 1;
}
