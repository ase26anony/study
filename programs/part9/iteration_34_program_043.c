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
static v4si complex_shuffle_10op(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually needs:
     * 4 source vectors (a, b, c, d) = 4 operands
     * 6 immediate control values for shuffle indices = 6 operands
     * Total: 10 operands
     */
    
    /* Create a complex shuffle using multiple builtins */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});
    
    /* Final shuffle combining results - this may expand to 10 operands */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){0, 2, 4, 6});
    
    return result;
}

/* Vector blend with many constants - potential 10-operand expansion */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blend operation with multiple constants */
    v4sf result;
    
    /* Use conditional operations that may expand to multi-operand form */
    result = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    
    /* Additional operations to increase operand count */
    result = result * (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    result = result + (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    /* Complex expression that might require 10 operands when expanded */
    v4sf temp = __builtin_shuffle(c, d, (v4si){1, 3, 5, 7});
    result = __builtin_ia32_blendps(result, temp, 0x5); /* 0x5 = 0101 binary */
    
    return result;
}

/* Atomic operation simulation with many parameters */
static long long atomic_multi_op(volatile long long *ptr, int a, int b, int c, int d) {
    /* Complex atomic-like operation that might expand to many operands */
    long long old = *ptr;
    long long new_val = old + (a * b) + (c << 2) + (d & 0xFF);
    
    /* This compare-and-swap conceptually needs many operands */
    __atomic_compare_exchange_n(ptr, &old, new_val, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return old;
}

/* Main test function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fvec_d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    volatile long long atomic_var = 0;
    int result_sum = 0;
    
    /* Loop to prevent dead code elimination */
    for (int i = 0; i < iterations; i++) {
        /* Modify vectors slightly each iteration */
        vec_a[0] += i & 1;
        vec_b[1] += i & 2;
        vec_c[2] += i & 4;
        vec_d[3] += i & 8;
        
        /* Call complex shuffle - target for 10-operand expansion */
        v4si shuffle_result = complex_shuffle_10op(vec_a, vec_b, vec_c, vec_d);
        
        /* Call vector blend - another potential 10-operand target */
        v4sf blend_result = vector_blend_complex(fvec_a, fvec_b, fvec_c, fvec_d);
        
        /* Use results to prevent optimization */
        result_sum += shuffle_result[0] + (int)blend_result[0];
        
        /* Simulate atomic operation with many parameters */
        long long atomic_old = atomic_multi_op(&atomic_var, 
                                              i, i*2, i*3, i*4);
        result_sum += (int)atomic_old;
        
        /* Prevent loop unrolling */
        if (i % 10 == 0) {
            fvec_a = __builtin_shuffle(fvec_a, fvec_b, (v4si){3, 2, 1, 0});
        }
    }
    
    /* Print result to ensure side effects */
    printf("Result: %d (Atomic final: %lld)\n", result_sum, atomic_var);
    
    return result_sum > 0 ? 0 : 1;
}

/* Additional complex operation that might trigger 10-operand expansion */
v4si __attribute__((noinline)) 
complex_vector_operation(v4si a, v4si b, v4si mask) {
    /* This complex expression uses many operands:
     * 1. a, b, mask = 3 vector operands
     * 2. Multiple constants in the operations
     * 3. Combined operations may expand to 10 operands
     */
    
    /* Create a complex pattern with many immediate values */
    v4si temp = a + (v4si){1, 2, 3, 4};
    temp = temp * (v4si){2, 3, 4, 5};
    temp = temp & mask;
    
    /* Complex shuffle with immediate indices */
    v4si result = __builtin_shuffle(temp, b, 
                                   (v4si){0, 4, 2, 6}); /* 4 immediate indices */
    
    /* Additional operation with immediate */
    result = result | (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    
    return result;
}
