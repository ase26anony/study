/* test_optabs_10_operands.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle that conceptually needs many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This shuffle pattern uses multiple source vectors and immediate controls
     * The expansion may need: 4 source vectors + 6 control indices = 10 operands
     */
    v4si result;
    
    /* Use __builtin_shuffle with multiple vectors and complex control */
    /* First, create intermediate shuffles */
    v4si ab_shuf = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si cd_shuf = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Final shuffle combining all 4 vectors indirectly */
    /* This complex expression may trigger multi-operand expansion */
    result = __builtin_shuffle(ab_shuf, cd_shuf, (v4si){3, 0, 5, 2});
    
    return result;
}

/* Vector blend with many control bits - may expand to 10 operands */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                                 int mask1, int mask2, int mask3, int mask4) {
    /* Complex blend operation that might use optabs with many operands */
    v4sf temp1 = __builtin_shuffle(a, b, (v4si){mask1 & 3, mask1 >> 2, mask1 >> 4, mask1 >> 6});
    v4sf temp2 = __builtin_shuffle(c, d, (v4si){mask2 & 3, mask2 >> 2, mask2 >> 4, mask2 >> 6});
    
    /* Another layer of shuffle with immediate controls */
    v4sf result = __builtin_shuffle(temp1, temp2, 
                                   (v4si){mask3 & 3, mask3 >> 2, mask3 >> 4, mask3 >> 6});
    
    /* Additional operation that might need expansion */
    result = result * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} + 
             (v4sf){mask4 * 0.1f, mask4 * 0.2f, mask4 * 0.3f, mask4 * 0.4f};
    
    return result;
}

/* Test function with atomic operations that might need many operands */
static long long test_atomic_complex(volatile long long *ptr, 
                                     long long a, long long b, 
                                     long long c, long long d,
                                     long long e, long long f) {
    /* Complex atomic expression - some architectures might expand this
     * to an operation with many operands */
    long long old = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    long long new_val = ((old * a + b) * c + d) * e + f;
    
    /* Compare exchange with many parameters in the calculation */
    __atomic_compare_exchange_n(ptr, &old, new_val, 0, 
                                __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    
    return old;
}

/* Main test that creates complex expressions */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fvec_d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    volatile long long atomic_var = 1000;
    
    v4si final_int_result = {0, 0, 0, 0};
    v4sf final_float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent optimization and trigger expansions */
    for (int i = 0; i < iterations; i++) {
        /* Modify vectors slightly each iteration */
        vec_a[0] += i;
        vec_b[1] += i;
        vec_c[2] += i;
        vec_d[3] += i;
        
        fvec_a[0] += i * 0.1f;
        fvec_b[1] += i * 0.2f;
        fvec_c[2] += i * 0.3f;
        fvec_d[3] += i * 0.4f;
        
        /* Call functions that may trigger 10-operand expansions */
        v4si shuffle_result = complex_shuffle_10_operands(vec_a, vec_b, vec_c, vec_d);
        
        /* Complex blend with many immediate constants */
        v4sf blend_result = vector_blend_complex(fvec_a, fvec_b, fvec_c, fvec_d,
                                                0x1A2B, 0x3C4D, 0x5E6F, i);
        
        /* Complex atomic operation */
        long long atomic_result = test_atomic_complex(&atomic_var,
                                                     i + 1, i + 2, i + 3, i + 4,
                                                     i + 5, i + 6);
        
        /* Accumulate results (prevents dead code elimination) */
        for (int j = 0; j < 4; j++) {
            final_int_result[j] += shuffle_result[j];
            final_float_result[j] += blend_result[j];
        }
        
        final_int_result[0] += (int)atomic_result;
    }
    
    /* Print results to ensure side effects */
    printf("Final int result: %d %d %d %d\n", 
           final_int_result[0], final_int_result[1], 
           final_int_result[2], final_int_result[3]);
    
    printf("Final float result: %.2f %.2f %.2f %.2f\n",
           final_float_result[0], final_float_result[1],
           final_float_result[2], final_float_result[3]);
    
    return 0;
}
