/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                       int idx0, int idx1, int idx2, int idx3,
                                       int idx4, int idx5) {
    /* This complex shuffle pattern conceptually requires:
       - 4 source vectors (a, b, c, d)
       - 6 immediate control indices
       Total: 10 operands
       
       The compiler may expand this into an internal function
       that requires 10 operands during RTL expansion */
    
    /* Create a complex permutation by combining multiple shuffles */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){idx0, idx1, 2, 3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){0, 1, idx2, idx3});
    
    /* Final shuffle with the remaining control indices */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){idx4, 5, idx5, 7});
    
    return result;
}

/* Vector blend with many control bits - may expand to multi-operand pattern */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d,
                                int mask0, int mask1, int mask2, int mask3,
                                float blend_factor) {
    /* Complex blending operation that might require many operands */
    v4sf ab_blend = a * (1.0f - blend_factor) + b * blend_factor;
    v4sf cd_blend = c * (1.0f - blend_factor) + d * blend_factor;
    
    /* Conditional selection based on multiple masks */
    v4sf result = __builtin_shuffle(ab_blend, cd_blend, 
                                   (v4si){mask0 ? 0 : 4, 
                                          mask1 ? 1 : 5,
                                          mask2 ? 2 : 6,
                                          mask3 ? 3 : 7});
    
    return result;
}

/* Multi-operand atomic operation simulation */
static long long complex_atomic_op(volatile long long *ptr, 
                                  long long a, long long b, long long c,
                                  long long d, long long e, long long f,
                                  int opcode) {
    /* Complex expression that might be expanded with many operands */
    long long result = 0;
    
    /* Multi-step operation that could be expanded into a complex pattern */
    result = a * b + c * d - e * f;
    result = (result << opcode) | (result >> (64 - opcode));
    
    /* Use atomic operation to ensure not optimized away */
    __atomic_fetch_add(ptr, result, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf vec_fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_fc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_fd = {13.0f, 14.0f, 15.0f, 16.0f};
    
    volatile long long atomic_var = 0;
    long long atomic_args[6] = {100, 200, 300, 400, 500, 600};
    
    v4si final_result_int = {0, 0, 0, 0};
    v4sf final_result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent optimization and ensure execution */
    for (volatile int i = 0; i < iterations; i++) {
        /* Test 1: Complex shuffle with 10 conceptual operands */
        v4si shuffle_result = complex_shuffle_10_operands(
            vec_a, vec_b, vec_c, vec_d,
            i % 4, (i + 1) % 4, (i + 2) % 4,
            (i + 3) % 4, (i + 4) % 4, (i + 5) % 4);
        
        /* Accumulate results */
        final_result_int += shuffle_result;
        
        /* Test 2: Complex vector blend */
        v4sf blend_result = vector_blend_complex(
            vec_fa, vec_fb, vec_fc, vec_fd,
            i & 1, (i >> 1) & 1, (i >> 2) & 1, (i >> 3) & 1,
            0.5f + (i * 0.1f));
        
        final_result_float += blend_result;
        
        /* Test 3: Multi-operand atomic-like operation */
        long long atomic_result = complex_atomic_op(
            &atomic_var,
            atomic_args[0] + i,
            atomic_args[1] - i,
            atomic_args[2] * (i + 1),
            atomic_args[3] / (i + 1),
            atomic_args[4] ^ i,
            atomic_args[5] | i,
            i % 32);
        
        /* Use results to prevent optimization */
        atomic_var += atomic_result;
    }
    
    /* Print results to create side effects */
    printf("Integer result: %d %d %d %d\n",
           final_result_int[0], final_result_int[1],
           final_result_int[2], final_result_int[3]);
    
    printf("Float result: %f %f %f %f\n",
           final_result_float[0], final_result_float[1],
           final_result_float[2], final_result_float[3]);
    
    printf("Atomic var: %lld\n", (long long)atomic_var);
    
    return 0;
}
