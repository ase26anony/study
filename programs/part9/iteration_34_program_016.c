/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * 4 source vectors (a, b, c, d)
     * 6 immediate control values for indices
     * Total: 10 operands
     */
    
    /* Create a complex shuffle with multiple control masks */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Combine with bitwise operations using multiple constants */
    v4si mask1 = (v4si){0xFF00FF00, 0x00FF00FF, 0xFF00FF00, 0x00FF00FF};
    v4si mask2 = (v4si){0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F};
    v4si mask3 = (v4si){0xCCCCCCCC, 0x33333333, 0xCCCCCCCC, 0x33333333};
    
    /* Complex expression with many operands */
    v4si result = (temp1 & mask1) | 
                  (temp2 & mask2) ^ 
                  ((temp1 >> 4) & mask3) |
                  ((temp2 << 4) & ~mask3);
    
    return result;
}

/* Vector conversion with multiple operands */
static v4sf vector_conversion_complex(v4si a, v4si b, v4si c, v4si d) {
    /* Convert vectors with scaling factors - may expand to multi-operand pattern */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    v4sf fc = __builtin_convertvector(c, v4sf);
    v4sf fd = __builtin_convertvector(d, v4sf);
    
    /* Fused multiply-add pattern with multiple constants */
    const v4sf coeff1 = (v4sf){3.14159f, 2.71828f, 1.41421f, 1.61803f};
    const v4sf coeff2 = (v4sf){0.57721f, 0.69314f, 0.70710f, 0.86602f};
    const v4sf coeff3 = (v4sf){0.5f, 0.25f, 0.125f, 0.0625f};
    
    /* Complex expression that may require many operands during expansion */
    v4sf result = fa * coeff1 + 
                  fb * coeff2 + 
                  fc * coeff3 + 
                  fd * (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    
    return result;
}

/* AVX2 specific operations that may use 10-operand patterns */
#ifdef __AVX2__
static v8si avx2_complex_permute(v8si a, v8si b, v8si c, v8si d) {
    /* AVX2 permute operations with multiple control masks */
    v8si mask1 = (v8si){0, 9, 2, 11, 4, 13, 6, 15};
    v8si mask2 = (v8si){8, 1, 10, 3, 12, 5, 14, 7};
    
    /* Complex shuffle/permute that may expand to many operands */
    v8si temp = __builtin_ia32_permvarsi256(a, mask1);
    v8si temp2 = __builtin_ia32_permvarsi256(b, mask2);
    
    /* Combine with multiple operations */
    v8si result = temp + temp2 - 
                  __builtin_ia32_permvarsi256(c, (v8si){7, 6, 5, 4, 3, 2, 1, 0}) +
                  __builtin_ia32_permvarsi256(d, (v8si){0, 2, 4, 6, 1, 3, 5, 7});
    
    return result;
}
#endif

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec_a = (v4si){argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = (v4si){argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = (v4si){argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = (v4si){argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4si shuffle_result = (v4si){0};
    v4sf convert_result = (v4sf){0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call complex shuffle function - may trigger 10-operand expansion */
        shuffle_result = complex_shuffle_10_operand(vec_a, vec_b, vec_c, vec_d);
        
        /* Call conversion function - may also trigger multi-operand expansion */
        convert_result = vector_conversion_complex(vec_a, vec_b, vec_c, vec_d);
        
        /* Modify inputs slightly to prevent loop invariant removal */
        vec_a[0] += i;
        vec_b[1] += i;
        vec_c[2] += i;
        vec_d[3] += i;
    }
    
#ifdef __AVX2__
    /* AVX2 operations if available */
    v8si avx_a = {argc, argc+1, argc+2, argc+3, argc+4, argc+5, argc+6, argc+7};
    v8si avx_b = {argc*2, argc*3, argc*4, argc*5, argc*6, argc*7, argc*8, argc*9};
    v8si avx_c = {argc*10, argc*11, argc*12, argc*13, argc*14, argc*15, argc*16, argc*17};
    v8si avx_d = {argc*18, argc*19, argc*20, argc*21, argc*22, argc*23, argc*24, argc*25};
    
    v8si avx_result = avx2_complex_permute(avx_a, avx_b, avx_c, avx_d);
    
    /* Use the result to prevent optimization */
    int avx_sum = 0;
    for (int i = 0; i < 8; i++) {
        avx_sum += avx_result[i];
    }
    printf("AVX2 result sum: %d\n", avx_sum);
#endif
    
    /* Use results to prevent dead code elimination */
    int shuffle_sum = shuffle_result[0] + shuffle_result[1] + 
                      shuffle_result[2] + shuffle_result[3];
    
    float convert_sum = convert_result[0] + convert_result[1] + 
                        convert_result[2] + convert_result[3];
    
    printf("Shuffle sum: %d\n", shuffle_sum);
    printf("Convert sum: %f\n", convert_sum);
    
    return (shuffle_sum > 0) ? 0 : 1;
}

/* Additional complex pattern that might trigger 11-operand case */
static v4si complex_11_operand_pattern(v4si a, v4si b, v4si c, v4si d, v4si e) {
    /* Even more complex pattern that might need 11 operands */
    v4si mask1 = (v4si){0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555};
    v4si mask2 = (v4si){0xCCCCCCCC, 0x33333333, 0xCCCCCCCC, 0x33333333};
    v4si mask3 = (v4si){0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F};
    v4si mask4 = (v4si){0xFF00FF00, 0x00FF00FF, 0xFF00FF00, 0x00FF00FF};
    
    /* Complex expression with many operands */
    v4si result = ((a & mask1) | (b & ~mask1)) +
                  ((c & mask2) ^ (d & ~mask2)) +
                  ((e & mask3) | ((a >> 2) & mask4)) -
                  ((b << 2) & mask1);
    
    return result;
}
