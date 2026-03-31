/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* SSE/AVX intrinsics if available */
#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __AVX__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                int m1, int m2, int m3, int m4) {
    /* Create a complex shuffle pattern that may expand to many operands */
    v4sf result = a;
    
    /* Multiple shuffle operations chained together */
#ifdef __SSE__
    /* __builtin_ia32_shufps typically expands with multiple operands */
    result = __builtin_ia32_shufps(result, b, m1);
    result = __builtin_ia32_shufps(result, c, m2);
    result = __builtin_ia32_shufps(result, d, m3);
    
    /* Additional blend operation */
    v4sf temp = __builtin_ia32_shufps(c, d, m4);
    result = __builtin_ia32_addps(result, temp);
#endif
    
    return result;
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE float pattern_b_fma_chain(float a, float b, float c, float d,
                                   float e, float f, float g, float h,
                                   float i, float j, float k) {
    /* Chain of FMAs that may flatten to many operands */
    float result = 0.0f;
    
#ifdef __FMA__
    /* Use __builtin_fmaf if available */
    result = __builtin_fmaf(a, b, __builtin_fmaf(c, d, 
                        __builtin_fmaf(e, f, __builtin_fmaf(g, h,
                        __builtin_fmaf(i, j, k)))));
#else
    /* Manual FMA simulation */
    result = a * b + c * d + e * f + g * h + i * j + k;
#endif
    
    return result;
}

/* Pattern C: Vector reduction with explicit lane extraction */
NOINLINE float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually extract and sum all lanes - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract each element - each extract is a separate operation */
    sum += v1[0] + v1[1] + v1[2] + v1[3];
    sum += v2[0] + v2[1] + v2[2] + v2[3];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    sum += v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Additional arithmetic to create more operands */
    sum = sum * 2.0f - (v1[0] * v2[1]) + (v3[2] / v4[3]);
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                          v4sf mask1, v4sf mask2) {
    v4sf result = a;
    
#ifdef __SSE__
    /* Multiple conditional operations */
    v4sf cmp1 = __builtin_ia32_cmpps(result, b, 1);  /* CMP_LT_OS */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);
    
    /* Blend based on comparisons - may expand to many operands */
    v4sf blend1 = __builtin_ia32_andps(cmp1, mask1);
    v4sf blend2 = __builtin_ia32_andps(cmp2, mask2);
    
    result = __builtin_ia32_orps(blend1, blend2);
    
    /* Additional shuffle to mix results */
    result = __builtin_ia32_shufps(result, blend1, 0x1B);
#endif
    
    return result;
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                         int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Inline asm with 11 total operands (2 outputs, 9 inputs) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "sub %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: AVX2 gather operation simulation with many address components */
NOINLINE v4sf pattern_f_gather_simulation(v4sf base, v4si indices,
                                          v4sf scale_vec, v4sf data1,
                                          v4sf data2, v4sf mask) {
    v4sf result = base;
    
#ifdef __AVX2__
    /* Simulate gather with multiple operations */
    for (int i = 0; i < 4; i++) {
        int idx = indices[i];
        float scale = scale_vec[i];
        float val1 = data1[i];
        float val2 = data2[i];
        float m = mask[i];
        
        /* Conditional load/store pattern */
        if (m > 0.5f) {
            result[i] = val1 * scale + val2;
        } else {
            result[i] = base[i] * scale - val2;
        }
    }
#endif
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize with some data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4si indices = {0, 2, 1, 3};
    
    float checksum = 0.0f;
    
    /* Use argc to select different patterns, ensuring all get compiled */
    switch (argc % 6) {
        case 0:
            checksum += pattern_a_shuffle(vec1, vec2, vec3, vec4, 0x1B, 0x27, 0x39, 0x4E)[0];
            break;
        case 1:
            checksum += pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f,
                                           6.6f, 7.7f, 8.8f, 9.9f, 10.1f, 11.1f);
            break;
        case 2:
            checksum += pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
            break;
        case 3:
            checksum += pattern_d_conditional_blend(vec1, vec2, vec3, vec4, vec1, vec2)[0];
            break;
        case 4:
            checksum += pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
        case 5:
            checksum += pattern_f_gather_simulation(vec1, indices, vec2, vec3, vec4, vec1)[0];
            break;
    }
    
    /* Additional test: Call all patterns to ensure they're all expanded */
    if (argc > 1) {
        checksum += pattern_a_shuffle(vec1, vec2, vec3, vec4, 0x1B, 0x27, 0x39, 0x4E)[1];
        checksum += pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f,
                                       6.6f, 7.7f, 8.8f, 9.9f, 10.1f, 11.1f);
        checksum += pattern_c_vector_reduction(vec2, vec3, vec4, vec1);
        checksum += pattern_d_conditional_blend(vec2, vec3, vec4, vec1, vec2, vec3)[1];
        checksum += pattern_e_multi_operand_asm(10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
        checksum += pattern_f_gather_simulation(vec2, indices, vec3, vec4, vec1, vec2)[1];
    }
    
    /* Store to volatile global to prevent optimization */
    g_checksum = (int)checksum;
    
    printf("Checksum: %d\n", g_checksum);
    return g_checksum != 0 ? 0 : 1;
}

/* Additional helper to force expansion of complex expressions */
NOINLINE v8sf create_complex_avx_expression(v8sf a, v8sf b, v8sf c, v8sf d,
                                            v8sf e, v8sf f, v8sf g, v8sf h) {
    v8sf result = a;
    
    /* Chain many AVX operations */
#ifdef __AVX__
    result = result + b * c - d / e + f - g * h;
    
    /* Permute operations */
    result = __builtin_ia32_permdf256(result, 0x1B);
    result = __builtin_ia32_permdf256(result, 0x27);
    
    /* Blend operations */
    v8sf temp = __builtin_ia32_blendpd256(result, a, 0x3);
    result = __builtin_ia32_blendpd256(temp, b, 0x5);
#endif
    
    return result;
}
