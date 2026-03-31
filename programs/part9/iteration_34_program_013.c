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
static v4si complex_shuffle_10op(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires many operands:
     * - 4 source vectors (a, b, c, d)
     * - 6 immediate control values for shuffle indices
     * Total: 10 operands
     */
    
    /* Create a complex shuffle by combining multiple operations */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 4, 2, 6});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){1, 5, 3, 7});
    
    /* Final shuffle with complex pattern - this may expand to 10 operands */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){0, 2, 4, 6});
    
    return result;
}

/* Vector blend with multiple control masks - potential for 10+ operands */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blending operation that might expand to many operands */
    v4sf temp = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4sf result = __builtin_shuffle(temp, c, (v4si){4, 1, 6, 3});
    
    /* Additional operation with immediate constants */
    result = result + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    
    return result;
}

/* Multi-operand expression with constants */
static v4sf complex_fma_like(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Fused multiply-add like expression with multiple constants
     * This may expand to an optab with many operands:
     * a * 1.5 + b * 2.5 + c * 3.5 + d * 4.5
     */
    v4sf result = a * (v4sf){1.5f, 1.5f, 1.5f, 1.5f} +
                  b * (v4sf){2.5f, 2.5f, 2.5f, 2.5f} +
                  c * (v4sf){3.5f, 3.5f, 3.5f, 3.5f} +
                  d * (v4sf){4.5f, 4.5f, 4.5f, 4.5f};
    
    return result;
}

/* Table lookup simulation using vector operations */
static v4si vector_table_lookup(v4si indices, v4si table0, v4si table1, 
                                v4si table2, v4si table3) {
    /* Simulate a 4-vector table lookup - may require many operands */
    v4si mask = indices & 3;
    v4si result;
    
    /* Complex conditional selection - each condition is an operand */
    result = (mask == 0) ? table0 :
             (mask == 1) ? table1 :
             (mask == 2) ? table2 : table3;
    
    return result;
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf vec_af = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_bf = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_cf = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_df = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v4si result_int = {0, 0, 0, 0};
    v4sf result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Mix different operations to create complex patterns */
        if (i % 3 == 0) {
            result_int = complex_shuffle_10op(vec_a, vec_b, vec_c, vec_d);
        } else if (i % 3 == 1) {
            result_float = vector_blend_complex(vec_af, vec_bf, vec_cf, vec_df);
        } else {
            result_float = complex_fma_like(vec_af, vec_bf, vec_cf, vec_df);
        }
        
        /* Modify inputs slightly to prevent constant propagation */
        vec_a[0] += i;
        vec_af[0] += 0.1f * i;
    }
    
    /* Use results to prevent elimination */
    int sum_int = result_int[0] + result_int[1] + result_int[2] + result_int[3];
    float sum_float = result_float[0] + result_float[1] + 
                      result_float[2] + result_float[3];
    
    printf("Result sums: int=%d, float=%.2f\n", sum_int, sum_float);
    
    /* Additional test with AVX2-like operations if available */
#ifdef __AVX2__
    /* Use permutevar operations that may require many operands */
    v4si idx = {3, 2, 1, 0};
    v4si permuted = __builtin_shuffle(vec_a, idx);
    printf("Permuted: %d %d %d %d\n", 
           permuted[0], permuted[1], permuted[2], permuted[3]);
#endif
    
    return 0;
}
