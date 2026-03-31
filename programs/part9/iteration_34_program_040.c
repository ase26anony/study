/* test_optabs.c - Target coverage for optabs.cc lines 8254-8263 */

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
     * 4 source vectors (a, b, c, d) = 4 operands
     * 6 immediate control values = 6 operands  
     * Total: 10 operands for the expansion
     */
    
    /* Create a complex shuffle using multiple builtins */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){3, 1, 2, 0});
    
    /* Another shuffle combining results - this may expand to 10 operands */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){1, 5, 3, 7});
    
    return result;
}

/* Vector blend with many constants - potentially 10 operands */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blend operation with multiple constants */
    v4sf result;
    
    /* This sequence creates a pattern that may require many operands
     * during expansion due to constant masks and multiple sources */
    result = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    result = result + __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Additional operations with constants */
    result = result * (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    result = result + (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    return result;
}

/* Complex multiply-add pattern with many constants */
static v2df fma_like_operation(v2df a, v2df b, v2df c, v2df d) {
    /* Pattern: result = a*b + c*d + constant_blend
     * This may expand to many operands due to constants and
     * multiple vector operations */
    
    v2df t1 = a * b;
    v2df t2 = c * d;
    
    /* Complex blend with constants */
    v2df constants = (v2df){3.14159, 2.71828};
    v2df result = __builtin_shuffle(t1, t2, (v2di){0, 3}) + constants;
    
    return result;
}

/* Atomic-like operation simulation with many parameters */
static long long complex_atomic_op(volatile long long *ptr, 
                                   long long a, long long b, 
                                   long long c, long long d) {
    /* Simulate complex atomic operation requiring many parameters */
    long long old = *ptr;
    long long new_val;
    
    /* Complex calculation with many operands */
    new_val = ((old & a) | (b & c)) ^ d;
    new_val = new_val + (a ^ b ^ c ^ d);
    
    /* Try to simulate __sync_val_compare_and_swap with more complexity */
    return __sync_val_compare_and_swap(ptr, old, new_val);
}

/* Main test function with loops to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fvec_d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2df dvec_a = {1.0, 2.0};
    v2df dvec_b = {3.0, 4.0};
    v2df dvec_c = {5.0, 6.0};
    v2df dvec_d = {7.0, 8.0};
    
    volatile long long atomic_var = 42;
    long long atomic_params[4] = {0xFFFF, 0xAAAA, 0x5555, 0x3333};
    
    v4si int_result = {0};
    v4sf float_result = {0};
    v2df double_result = {0};
    volatile long long atomic_result = 0;
    
    /* Loop to prevent optimization and ensure execution */
    for (int i = 0; i < iterations; i++) {
        /* Modify vectors slightly each iteration */
        vec_a[0] += i;
        vec_b[1] += i;
        vec_c[2] += i;
        vec_d[3] += i;
        
        /* Call operations that may trigger 10-operand expansions */
        int_result = complex_shuffle_10_operand(vec_a, vec_b, vec_c, vec_d);
        float_result = vector_blend_complex(fvec_a, fvec_b, fvec_c, fvec_d);
        double_result = fma_like_operation(dvec_a, dvec_b, dvec_c, dvec_d);
        
        /* Complex atomic-like operation */
        atomic_result = complex_atomic_op(&atomic_var, 
                                         atomic_params[0] + i,
                                         atomic_params[1] - i,
                                         atomic_params[2] ^ i,
                                         atomic_params[3] | i);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: int[0]=%d, float[0]=%.2f, double[0]=%.2f, atomic=%lld\n",
           int_result[0], float_result[0], double_result[0], atomic_result);
    
    /* Additional complex expression that might trigger the 11-operand case */
    if (iterations > 1) {
        /* Very complex expression with many constants */
        v4si final_vec = int_result + 
                        __builtin_shuffle(vec_a, vec_b, (v4si){3, 2, 1, 0}) +
                        __builtin_shuffle(vec_c, vec_d, (v4si){0, 1, 2, 3}) +
                        (v4si){100, 200, 300, 400};
        
        printf("Final vector: [%d, %d, %d, %d]\n",
               final_vec[0], final_vec[1], final_vec[2], final_vec[3]);
    }
    
    return 0;
}
