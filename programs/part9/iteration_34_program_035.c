/* test_optabs.c - Target coverage for optabs.cc lines 8254-8263 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex shuffle function that uses many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d, 
                                      int idx0, int idx1, int idx2, int idx3,
                                      int mask0, int mask1) {
    /* This should trigger case 10: with 10 operands */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){idx0, idx1, idx2, idx3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){idx3, idx2, idx1, idx0});
    
    /* Complex bitwise operations with multiple constants */
    v4si result = (temp1 & (v4si){mask0, mask1, ~mask0, ~mask1}) |
                  (temp2 & (v4si){~mask1, ~mask0, mask1, mask0});
    
    return result;
}

/* Vector conversion with many parameters */
static v4sf vector_convert_10_operand(v4si a, v4si b, v4sf c, v4sf d,
                                      float scale1, float scale2, 
                                      float bias1, float bias2,
                                      int round_mode, int saturate) {
    /* Complex conversion pattern that may expand to many operands */
    v4sf va = __builtin_convertvector(a, v4sf);
    v4sf vb = __builtin_convertvector(b, v4sf);
    
    /* Fused multiply-add with multiple constants */
    v4sf result = va * (v4sf){scale1, scale2, scale1, scale2} + 
                  vb * (v4sf){scale2, scale1, scale2, scale1} +
                  c * (v4sf){bias1, bias2, bias1, bias2} +
                  d * (v4sf){bias2, bias1, bias2, bias1};
    
    /* Conditional operations based on parameters */
    if (round_mode) {
        result = __builtin_ia32_roundps(result, round_mode);
    }
    
    if (saturate) {
        v4sf max_val = (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
        v4sf min_val = (v4sf){-1.0f, -1.0f, -1.0f, -1.0f};
        result = __builtin_ia32_minps(result, max_val);
        result = __builtin_ia32_maxps(result, min_val);
    }
    
    return result;
}

/* AVX2 specific shuffle with many operands */
#ifdef __AVX2__
static v8si avx2_complex_shuffle(v8si a, v8si b, v8si c, v8si d,
                                int idx0, int idx1, int idx2, int idx3,
                                int idx4, int idx5, int idx6, int idx7) {
    /* This complex shuffle pattern may require many operands during expansion */
    v8si temp1 = __builtin_ia32_permvar256si(a, (v8si){idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7});
    v8si temp2 = __builtin_ia32_permvar256si(b, (v8si){idx7, idx6, idx5, idx4, idx3, idx2, idx1, idx0});
    v8si temp3 = __builtin_ia32_permvar256si(c, (v8si){idx3, idx2, idx1, idx0, idx7, idx6, idx5, idx4});
    v8si temp4 = __builtin_ia32_permvar256si(d, (v8si){idx4, idx5, idx6, idx7, idx0, idx1, idx2, idx3});
    
    /* Complex blend operation */
    v8si mask = (v8si){0, -1, 0, -1, 0, -1, 0, -1};
    v8si result = __builtin_ia32_pblendd256(temp1, temp2, 0xAA);
    result = __builtin_ia32_pblendd256(result, temp3, 0x55);
    result = __builtin_ia32_pblendd256(result, temp4, 0xCC);
    
    return result;
}
#endif

/* Main test function with non-trivial loop */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = (v4si){1, 2, 3, 4};
    v4si vec_b = (v4si){5, 6, 7, 8};
    v4si vec_c = (v4si){9, 10, 11, 12};
    v4si vec_d = (v4si){13, 14, 15, 16};
    
    v4sf vec_fa = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_fb = (v4sf){5.0f, 6.0f, 7.0f, 8.0f};
    
    v4si shuffle_result = (v4si){0};
    v4sf convert_result = (v4sf){0};
    
    /* Loop to prevent optimization */
    for (volatile int i = 0; i < iterations; i++) {
        /* Test case 1: Complex shuffle with exactly 10 operands */
        shuffle_result = complex_shuffle_10_operand(
            vec_a, vec_b, vec_c, vec_d,
            i % 4, (i + 1) % 4, (i + 2) % 4, (i + 3) % 4,
            0xFF00FF00, 0x00FF00FF
        );
        
        /* Test case 2: Vector conversion with many parameters */
        convert_result = vector_convert_10_operand(
            vec_a, vec_b, vec_fa, vec_fb,
            1.5f + i * 0.1f, 2.5f - i * 0.1f,
            0.5f, -0.5f,
            i % 4,  // round_mode
            i % 2   // saturate
        );
        
        /* Modify vectors slightly to prevent constant folding */
        vec_a += (v4si){1, 1, 1, 1};
        vec_b += (v4si){2, 2, 2, 2};
    }
    
#ifdef __AVX2__
    /* AVX2 specific test with more operands */
    v8si avx_a = (v8si){1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_b = (v8si){9, 10, 11, 12, 13, 14, 15, 16};
    v8si avx_c = (v8si){17, 18, 19, 20, 21, 22, 23, 24};
    v8si avx_d = (v8si){25, 26, 27, 28, 29, 30, 31, 32};
    
    v8si avx_result = avx2_complex_shuffle(
        avx_a, avx_b, avx_c, avx_d,
        0, 7, 1, 6, 2, 5, 3, 4
    );
    
    /* Use the result to prevent dead code elimination */
    int avx_sum = 0;
    for (int i = 0; i < 8; i++) {
        avx_sum += avx_result[i];
    }
    printf("AVX2 shuffle sum: %d\n", avx_sum);
#endif
    
    /* Print results to prevent optimization */
    printf("Shuffle result: %d %d %d %d\n", 
           shuffle_result[0], shuffle_result[1], 
           shuffle_result[2], shuffle_result[3]);
    printf("Convert result: %.2f %.2f %.2f %.2f\n",
           convert_result[0], convert_result[1],
           convert_result[2], convert_result[3]);
    
    return 0;
}
