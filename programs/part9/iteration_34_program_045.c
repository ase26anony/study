/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * - 4 source vectors (a, b, c, d)
     * - 6 immediate control values for indices
     * Total: 10 operands
     */
    
    /* Create a shuffle mask with 6 immediate control values */
    const int mask0 = 0, mask1 = 4, mask2 = 1, mask3 = 5, mask4 = 2, mask5 = 6;
    
    /* Use __builtin_shufflevector which may expand to multi-operand optab */
    /* The shufflevector built-in takes: dest_type, src1, src2, index_list... */
    v4si result = __builtin_shufflevector(a, b, mask0, mask1, mask2, mask3);
    
    /* Combine with another shuffle using more vectors */
    v4si temp = __builtin_shufflevector(c, d, mask4, mask5, mask0, mask1);
    
    /* Mix the results with bitwise operations and constants */
    result = result ^ temp;
    result = result & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    result = result | (v4si){0x1, 0x100, 0x10000, 0x1000000};
    
    return result;
}

/* Vector FMA-like operation with multiple constants */
static v4sf vector_polynomial(v4sf x, v4sf coeff0, v4sf coeff1, 
                              v4sf coeff2, v4sf coeff3) {
    /* Polynomial: coeff0 + coeff1*x + coeff2*x*x + coeff3*x*x*x */
    /* Each operation might expand to multiple operands */
    
    v4sf x2 = x * x;
    v4sf x3 = x2 * x;
    
    /* This expression tree with multiple constants may require
     * many operands during expansion */
    v4sf result = coeff0 + coeff1 * x + coeff2 * x2 + coeff3 * x3;
    
    /* Additional complex operation with immediate mask */
    result = __builtin_shuffle(result, result, 
                              (v4sf){2.0f, 3.0f, 0.0f, 1.0f});
    
    return result;
}

/* Atomic-style operation simulation with many parameters */
static v2di complex_atomic_op(v2di *ptr, v2di old_val, v2di new_val) {
    /* Simulate complex atomic compare-and-swap with mask */
    v2di current = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    
    /* Create comparison mask with multiple immediate values */
    v2di mask = (v2di){0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFF00000000ULL};
    
    /* Complex bitwise operation with many constants */
    v2di masked_old = old_val & mask;
    v2di masked_new = new_val & mask;
    v2di masked_current = current & mask;
    
    /* Conditional update - this may expand to multi-operand pattern */
    v2di result = (masked_current == masked_old) ? masked_new : masked_current;
    
    /* Additional shuffle with immediate control */
    result = __builtin_shuffle(result, result, (v2di){1, 0});
    
    return result;
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf vec_x = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf coeffs0 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf coeffs1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf coeffs2 = {0.1f, 0.2f, 0.3f, 0.4f};
    v4sf coeffs3 = {0.01f, 0.02f, 0.03f, 0.04f};
    
    v2di atomic_var = {0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL};
    v2di old_val = {0x123456789ABCDEF0ULL, 0xFFFFFFFFFFFFFFFFULL};
    v2di new_val = {0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL};
    
    v4si int_result = {0};
    v4sf float_result = {0.0f};
    v2di atomic_result = {0};
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        int_result = complex_shuffle_10_operands(vec_a, vec_b, vec_c, vec_d);
        
        float_result = vector_polynomial(vec_x, coeffs0, coeffs1, 
                                        coeffs2, coeffs3);
        
        atomic_result = complex_atomic_op(&atomic_var, old_val, new_val);
        
        /* Modify inputs slightly to prevent constant propagation */
        vec_a[0] += i;
        vec_x[0] += (float)i * 0.1f;
        old_val[0] += i;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", 
           int_result[0], int_result[1], int_result[2], int_result[3]);
    printf("Float results: %f %f %f %f\n",
           float_result[0], float_result[1], 
           float_result[2], float_result[3]);
    printf("Atomic results: %llx %llx\n",
           (unsigned long long)atomic_result[0],
           (unsigned long long)atomic_result[1]);
    
    return 0;
}
