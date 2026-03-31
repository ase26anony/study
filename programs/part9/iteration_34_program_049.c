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
    /* This should trigger a 10-operand expansion:
     * 4 vector inputs + 6 immediate indices = 10 operands
     */
    v4si result;
    
    /* Complex shuffle pattern using __builtin_shuffle with multiple vectors
     * This is conceptual - actual implementation uses multiple operations
     * that may be combined into a single expansion */
    
    /* First, create intermediate shuffles */
    v4si ab_shuffle = __builtin_shuffle(a, b, (v4si){idx0, idx1, 2, 3});
    v4si cd_shuffle = __builtin_shuffle(c, d, (v4si){idx2, idx3, 0, 1});
    
    /* Final shuffle combining results with more indices */
    result = __builtin_shuffle(ab_shuffle, cd_shuffle, 
                              (v4si){idx4, idx5, 6, 7});
    
    return result;
}

/* Vector blend with many control bits */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d,
                                int mask0, int mask1, int mask2, int mask3,
                                int mask4, int mask5) {
    /* Complex blending operation that may expand to many operands */
    v4sf temp1 = __builtin_shuffle(a, b, (v4si){mask0, mask1, 2, 3});
    v4sf temp2 = __builtin_shuffle(c, d, (v4si){mask2, mask3, 0, 1});
    
    /* Final operation with immediate control */
    v4sf result = __builtin_shuffle(temp1, temp2, 
                                   (v4si){mask4, mask5, 6, 7});
    
    return result;
}

/* Multi-operand atomic-like operation simulation */
static long long complex_atomic_op(long long *ptr, int a, int b, int c,
                                  int d, int e, int f, int g, int h) {
    /* Simulate complex atomic operation with many parameters */
    long long old = *ptr;
    long long new_val = old + (a * b) + (c * d) + (e * f) + (g * h);
    
    /* Use __sync builtin which may expand to multi-operand pattern */
    return __sync_fetch_and_add(ptr, new_val);
}

/* Main test function */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 3;
    
    /* Initialize vector variables with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fvec_d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v4si result_vec = {0, 0, 0, 0};
    v4sf fresult_vec = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Use volatile to prevent optimization */
    volatile int control_idx = 0;
    
    /* Loop to ensure code is executed but not optimized away */
    for (int i = 0; i < iterations; i++) {
        /* Update control indices to prevent constant propagation */
        control_idx = (control_idx + i) % 8;
        
        /* Test 1: Complex shuffle with many operands */
        result_vec = complex_shuffle_10_operands(
            vec_a, vec_b, vec_c, vec_d,
            (control_idx + 0) % 8,  /* idx0 */
            (control_idx + 1) % 8,  /* idx1 */
            (control_idx + 2) % 8,  /* idx2 */
            (control_idx + 3) % 8,  /* idx3 */
            (control_idx + 4) % 8,  /* idx4 */
            (control_idx + 5) % 8   /* idx5 */
        );
        
        /* Test 2: Vector blend with many control bits */
        fresult_vec = vector_blend_complex(
            fvec_a, fvec_b, fvec_c, fvec_d,
            (control_idx + 0) & 1,  /* mask0 */
            (control_idx + 1) & 1,  /* mask1 */
            (control_idx + 2) & 1,  /* mask2 */
            (control_idx + 3) & 1,  /* mask3 */
            (control_idx + 4) & 1,  /* mask4 */
            (control_idx + 5) & 1   /* mask5 */
        );
        
        /* Modify vectors slightly to prevent optimization */
        vec_a[0] += i;
        fvec_a[0] += (float)i;
    }
    
    /* Create side effect to prevent dead code elimination */
    long long atomic_var = 0;
    long long atomic_result = complex_atomic_op(&atomic_var, 1, 2, 3, 4, 5, 6, 7, 8);
    
    /* Print results to ensure they're used */
    printf("Result vector: [%d, %d, %d, %d]\n", 
           result_vec[0], result_vec[1], result_vec[2], result_vec[3]);
    printf("Float result: [%f, %f, %f, %f]\n",
           fresult_vec[0], fresult_vec[1], fresult_vec[2], fresult_vec[3]);
    printf("Atomic result: %lld\n", atomic_result);
    
    return 0;
}
