/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
       1-4: Input vectors a, b, c, d
       5-10: 6 immediate control values for shuffle pattern
       Total: 10 operands */
    
    /* Create a complex shuffle using multiple builtins and constants */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Mix with immediate constants - each constant adds an operand */
    v4si result = temp1 + (v4si){1, 2, 3, 4};
    result = result * temp2;
    result = result & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    
    return result;
}

/* Vector blend with many immediate control bits */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blend operation that might expand to 10 operands:
       4 input vectors + 6 control values */
    
    /* Create pattern with multiple constants */
    v4sf pattern1 = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    v4sf pattern2 = __builtin_shuffle(c, d, (v4si){0, 1, 2, 3});
    
    /* Complex expression with many constants */
    v4sf result = pattern1 * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} +
                  pattern2 * (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    /* Additional operations with constants */
    result = result + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    
    return result;
}

/* Cryptographic-style permutation */
static v2di crypto_permute(v2di a, v2di b, v2di c, v2di d) {
    /* Simulate a complex permutation that might require many operands */
    
    /* Multiple shuffle operations with immediate controls */
    v2di shuffled1 = __builtin_shuffle(a, b, (v2di){0, 3});
    v2di shuffled2 = __builtin_shuffle(c, d, (v2di){2, 1});
    
    /* Bitwise operations with multiple mask constants */
    v2di mask1 = (v2di){0xFFFFFFFF00000000ULL, 0x00000000FFFFFFFFULL};
    v2di mask2 = (v2di){0xFFFF0000FFFF0000ULL, 0x0000FFFF0000FFFFULL};
    
    v2di result = (shuffled1 & mask1) | (shuffled2 & mask2);
    
    /* Additional operations with more constants */
    result = result ^ (v2di){0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL};
    
    return result;
}

/* Complex multiply-add with many coefficients */
static v4sf complex_fma(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Fused multiply-add with many constant coefficients */
    v4sf result = a * (v4sf){1.1f, 2.2f, 3.3f, 4.4f} +
                  b * (v4sf){0.9f, 1.8f, 2.7f, 3.6f} +
                  c * (v4sf){0.5f, 1.0f, 1.5f, 2.0f} +
                  d * (v4sf){0.25f, 0.5f, 0.75f, 1.0f};
    
    /* Additional constant term */
    result = result + (v4sf){0.01f, 0.02f, 0.03f, 0.04f};
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    volatile int iterations = 3; /* Prevent optimization */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 3;
    }
    
    /* Initialize vectors with different patterns */
    v4si vec_int1 = (v4si){1, 2, 3, 4};
    v4si vec_int2 = (v4si){5, 6, 7, 8};
    v4si vec_int3 = (v4si){9, 10, 11, 12};
    v4si vec_int4 = (v4si){13, 14, 15, 16};
    
    v4sf vec_float1 = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_float2 = (v4sf){5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_float3 = (v4sf){9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_float4 = (v4sf){13.0f, 14.0f, 15.0f, 16.0f};
    
    v2di vec_long1 = (v2di){0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL};
    v2di vec_long2 = (v2di){0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL};
    v2di vec_long3 = (v2di){0xCCCCCCCCCCCCCCCCULL, 0x3333333333333333ULL};
    v2di vec_long4 = (v2di){0xF0F0F0F0F0F0F0F0ULL, 0x0F0F0F0F0F0F0F0FULL};
    
    /* Accumulators to prevent dead code elimination */
    v4si int_result = (v4si){0, 0, 0, 0};
    v4sf float_result = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    v2di long_result = (v2di){0, 0};
    
    /* Loop to ensure execution but prevent excessive unrolling */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions with many operands/constants */
        int_result = int_result + complex_shuffle_10_operand(
            vec_int1 + i, vec_int2 + i, vec_int3 + i, vec_int4 + i);
        
        float_result = float_result + vector_blend_complex(
            vec_float1 + (float)i, vec_float2 + (float)i,
            vec_float3 + (float)i, vec_float4 + (float)i);
        
        long_result = long_result + crypto_permute(
            vec_long1, vec_long2, vec_long3, vec_long4);
        
        float_result = float_result + complex_fma(
            vec_float1, vec_float2, vec_float3, vec_float4);
    }
    
    /* Use results to prevent optimization */
    int sum_int = int_result[0] + int_result[1] + int_result[2] + int_result[3];
    float sum_float = float_result[0] + float_result[1] + 
                      float_result[2] + float_result[3];
    long long sum_long = long_result[0] + long_result[1];
    
    printf("Results: int=%d, float=%.2f, long=%lld\n", 
           sum_int, sum_float, sum_long);
    
    return sum_int > 0 ? 0 : 1;
}
