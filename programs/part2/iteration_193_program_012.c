/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                               int mask1, int mask2, int mask3, int mask4) {
    /* This pattern aims to create many operands during expansion */
    v4sf temp1, temp2, temp3, temp4;
    
    /* Multiple shuffle operations that may expand to many RTL operands */
#ifdef __SSE__
    temp1 = __builtin_ia32_shufps(a, b, mask1);
    temp2 = __builtin_ia32_shufps(c, d, mask2);
    temp3 = __builtin_ia32_shufps(temp1, temp2, mask3);
    temp4 = __builtin_ia32_shufps(temp3, a, mask4);
#else
    /* Fallback for non-SSE targets */
    temp1 = a + b;
    temp2 = c + d;
    temp3 = temp1 + temp2;
    temp4 = temp3 + a;
#endif
    
    return temp4;
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE float pattern_b_fma_chain(float a, float b, float c, float d,
                                  float e, float f, float g, float h) {
    /* Chain of FMAs that may flatten to many operands */
    float res;
    
#ifdef __FMA__
    res = __builtin_fma(a, b, 
           __builtin_fma(c, d,
           __builtin_fma(e, f,
           __builtin_fma(g, h, 0.0f))));
#else
    /* Manual FMA simulation */
    res = a * b + c * d + e * f + g * h;
#endif
    
    return res;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE float pattern_c_vector_reduce(v4sf v) {
    /* Manually unrolled horizontal addition */
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Extract each element - each extract may add operands */
    sum += __builtin_ia32_vec_ext_v4sf(v, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v, 3);
#else
    /* Fallback */
    float* p = (float*)&v;
    sum = p[0] + p[1] + p[2] + p[3];
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                          v4sf mask_vec) {
    v4sf result;
    
#ifdef __SSE__
    /* Complex conditional logic that may expand to many operands */
    v4sf cmp1 = __builtin_ia32_cmpleps(a, b);
    v4sf cmp2 = __builtin_ia32_cmpnltps(c, d);
    v4sf cmp3 = __builtin_ia32_cmpeqps(mask_vec, a);
    
    /* Blend operations with multiple conditions */
    v4sf temp1 = __builtin_ia32_andps(cmp1, a);
    v4sf temp2 = __builtin_ia32_andnps(cmp2, b);
    v4sf temp3 = __builtin_ia32_orps(temp1, temp2);
    
    /* Final blend based on third comparison */
    result = __builtin_ia32_blendvps(temp3, c, cmp3);
#else
    /* Fallback */
    result = a + b + c + d + mask_vec;
#endif
    
    return result;
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int64_t pattern_e_multi_operand_asm(int64_t a, int64_t b, int64_t c,
                                            int64_t d, int64_t e, int64_t f,
                                            int64_t g, int64_t h, int64_t i,
                                            int64_t j, int64_t k) {
    int64_t result;
    
    /* Inline assembly with 11 total operands (1 output + 10 inputs) */
    __asm__ volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "imul %[k], %[result]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Pattern F: AVX2 gather operation (if available) - often has many operands */
NOINLINE v8si pattern_f_gather_operation(v8si base, v8si index, v8si mask, 
                                        int scale, int* base_ptr) {
    v8si result;
    
#ifdef __AVX2__
    /* __builtin_ia32_gatherd_d may expand to many operands */
    result = __builtin_ia32_gatherd_d256(base, base_ptr, index, mask, scale);
#else
    /* Fallback simulation */
    for (int i = 0; i < 8; i++) {
        int idx = ((int*)&index)[i];
        if (((int*)&mask)[i]) {
            ((int*)&result)[i] = base_ptr[idx * scale];
        } else {
            ((int*)&result)[i] = ((int*)&base)[i];
        }
    }
#endif
    
    return result;
}

/* Main test driver with runtime variability */
int main(int argc, char** argv) {
    volatile float checksum = 0.0f;
    
    /* Initialize test data with some variability based on argc */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_d = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf mask_vec = {0.0f, 1.0f, 0.0f, 1.0f};
    
    int mask1 = (argc > 1) ? atoi(argv[1]) : 0x1B;
    int mask2 = (argc > 2) ? atoi(argv[2]) : 0x27;
    int mask3 = (argc > 3) ? atoi(argv[3]) : 0x39;
    int mask4 = (argc > 4) ? atoi(argv[4]) : 0x4A;
    
    /* Execute Pattern A */
    v4sf result_a = pattern_a_shuffle(vec_a, vec_b, vec_c, vec_d, 
                                     mask1, mask2, mask3, mask4);
    checksum += ((float*)&result_a)[0] + ((float*)&result_a)[1] +
                ((float*)&result_a)[2] + ((float*)&result_a)[3];
    
    /* Execute Pattern B */
    float scalars[8] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f};
    float result_b = pattern_b_fma_chain(scalars[0], scalars[1], scalars[2], scalars[3],
                                        scalars[4], scalars[5], scalars[6], scalars[7]);
    checksum += result_b;
    
    /* Execute Pattern C */
    float result_c = pattern_c_vector_reduce(vec_a);
    checksum += result_c;
    
    /* Execute Pattern D */
    v4sf result_d = pattern_d_conditional_select(vec_a, vec_b, vec_c, vec_d, mask_vec);
    checksum += ((float*)&result_d)[0] + ((float*)&result_d)[1] +
                ((float*)&result_d)[2] + ((float*)&result_d)[3];
    
    /* Execute Pattern E (exactly 11 operands in asm) */
    int64_t ints[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    int64_t result_e = pattern_e_multi_operand_asm(ints[0], ints[1], ints[2], ints[3],
                                                  ints[4], ints[5], ints[6], ints[7],
                                                  ints[8], ints[9], ints[10]);
    checksum += (float)result_e;
    
    /* Execute Pattern F if AVX2 available */
    v8si base_vec = {0, 1, 2, 3, 4, 5, 6, 7};
    v8si index_vec = {0, 2, 4, 6, 1, 3, 5, 7};
    v8si mask_vec8 = {-1, 0, -1, 0, -1, 0, -1, 0};
    int data_array[64];
    for (int i = 0; i < 64; i++) data_array[i] = i * 2;
    
    v8si result_f = pattern_f_gather_operation(base_vec, index_vec, mask_vec8, 1, data_array);
    for (int i = 0; i < 8; i++) {
        checksum += (float)((int*)&result_f)[i];
    }
    
    /* Use checksum to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    
    return (int)checksum % 256;
}
