/* test_optabs.c - Target GCC optabs.cc lines 8254-8263 (case 10) */

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
    /* This complex shuffle pattern conceptually requires:
     * - 4 source vectors (a, b, c, d) = 4 operands
     * - 6 immediate control values for shuffle indices = 6 operands
     * Total: 10 operands
     */
    
    /* Use multiple shuffle operations that may combine into a single
     * complex pattern during optimization/expansion */
    v4si t1 = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    v4si t2 = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});
    
    /* Final shuffle combining intermediate results with immediate indices */
    return __builtin_shuffle(t1, t2, (v4si){0, 4, 2, 6});
}

/* Vector blend with many immediate control bits */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blending pattern that might expand to multi-operand optab */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){0, 4, 2, 6});
    v4sf t2 = __builtin_shuffle(c, d, (v4si){1, 5, 3, 7});
    
    /* Blend with immediate mask - this may require many operands */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        int mask = ((i & 1) ? 0xFFFFFFFF : 0);
        float elem = mask ? t2[i] : t1[i];
        result[i] = elem;
    }
    
    return result;
}

/* Multi-operand vector conversion pattern */
static v2df complex_conversion(v4si ints, v4sf floats) {
    /* Chain of conversions that might require many operands */
    v4sf ints_to_float = __builtin_convertvector(ints, v4sf);
    
    /* Complex shuffle before conversion */
    v4sf shuffled = __builtin_shuffle(ints_to_float, floats, 
                                     (v4si){0, 4, 2, 6});
    
    /* Convert to double - may require expansion with many operands */
    v2df low = __builtin_convertvector(__builtin_shufflevector(shuffled, shuffled, 0, 1), v2df);
    v2df high = __builtin_convertvector(__builtin_shufflevector(shuffled, shuffled, 2, 3), v2df);
    
    /* Blend results */
    return __builtin_shuffle(low, high, (v2di){0, 2});
}

/* Atomic operation simulation with many parameters */
static long long atomic_like_op(long long *ptr, int a, int b, int c, 
                               int d, int e, int f, int g, int h) {
    /* Complex expression that might be expanded as atomic-like operation
     * with many immediate operands */
    long long val = *ptr;
    long long result = val + (long long)a * b + (long long)c * d +
                      (long long)e * f + (long long)g * h;
    
    /* Use __sync builtin which may expand to complex optab pattern */
    return __sync_val_compare_and_swap(ptr, val, result);
}

/* Main test function with non-optimizable computation */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec_a = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec_a = {argc * 1.0f, argc * 1.1f, argc * 1.2f, argc * 1.3f};
    v4sf fvec_b = {argc * 1.4f, argc * 1.5f, argc * 1.6f, argc * 1.7f};
    v4sf fvec_c = {argc * 1.8f, argc * 1.9f, argc * 2.0f, argc * 2.1f};
    v4sf fvec_d = {argc * 2.2f, argc * 2.3f, argc * 2.4f, argc * 2.5f};
    
    long long atomic_var = argc;
    
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    v2df double_result = {0.0, 0.0};
    long long atomic_result = 0;
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        int_result = complex_shuffle_10op(vec_a, vec_b, vec_c, vec_d);
        float_result = vector_blend_complex(fvec_a, fvec_b, fvec_c, fvec_d);
        double_result = complex_conversion(int_result, float_result);
        atomic_result = atomic_like_op((long long*)&atomic_var, 
                                      argc, argc+1, argc+2, argc+3,
                                      argc+4, argc+5, argc+6, argc+7);
        
        /* Modify inputs slightly to prevent complete optimization */
        vec_a[0] += i;
        fvec_a[0] += i * 0.1f;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", 
           int_result[0], int_result[1], int_result[2], int_result[3]);
    printf("Float: %f %f %f %f\n",
           float_result[0], float_result[1], float_result[2], float_result[3]);
    printf("Double: %lf %lf\n", double_result[0], double_result[1]);
    printf("Atomic: %lld\n", atomic_result);
    
    return 0;
}
