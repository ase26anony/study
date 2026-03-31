/* test_optabs_high_operand.c
 * Test program to cover 10 and 11 operand switch cases in optabs.cc
 * Compile with: gcc -O3 -march=native -fno-tree-vectorize -fprofile-arcs -ftest-coverage test_optabs_high_operand.c -o test_optabs
 */

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

/* SSE/AVX intrinsics */
#ifdef __SSE__
#include <xmmintrin.h>
#endif
#ifdef __SSE2__
#include <emmintrin.h>
#endif
#ifdef __SSE3__
#include <pmmintrin.h>
#endif
#ifdef __SSSE3__
#include <tmmintrin.h>
#endif
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif
#ifdef __AVX__
#include <immintrin.h>
#endif

/* ARM NEON intrinsics */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Pattern 1: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern1_shuffle_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                                              int mask1, int mask2, int mask3, int mask4) {
    v4sf result;
    
#ifdef __SSE__
    /* This creates many operands: 4 vectors + 4 masks + result */
    __m128 va = _mm_loadu_ps((float*)&a);
    __m128 vb = _mm_loadu_ps((float*)&b);
    __m128 vc = _mm_loadu_ps((float*)&c);
    __m128 vd = _mm_loadu_ps((float*)&d);
    
    /* Multiple shuffle operations that may be combined */
    __m128 t1 = _mm_shuffle_ps(va, vb, mask1);
    __m128 t2 = _mm_shuffle_ps(vc, vd, mask2);
    __m128 t3 = _mm_shuffle_ps(t1, t2, mask3);
    __m128 t4 = _mm_shuffle_ps(t2, t1, mask4);
    
    /* Complex blend with many operands */
    __m128 blend_mask = _mm_set_ps(0.0f, 1.0f, 0.0f, 1.0f);
    __m128 r = _mm_blendv_ps(t3, t4, blend_mask);
    
    _mm_storeu_ps((float*)&result, r);
#else
    /* Fallback for non-SSE */
    result = a + b + c + d;
#endif
    
    return result;
}

/* Pattern 2: Fused multiply-add chain with many accumulators */
NOINLINE static float pattern2_fma_chain(float a, float b, float c, float d,
                                         float e, float f, float g, float h,
                                         float i, float j, float k, float l) {
    float result;
    
#ifdef __FMA__
    /* Chain of FMA operations creating deep expression tree */
    result = __builtin_fma(a, b, 
              __builtin_fma(c, d, 
               __builtin_fma(e, f, 
                __builtin_fma(g, h, 
                 __builtin_fma(i, j, 
                  __builtin_fma(k, l, 0.0f))))));
#else
    /* Manual FMA emulation */
    result = a * b + c * d + e * f + g * h + i * j + k * l;
#endif
    
    return result;
}

/* Pattern 3: Vector reduction with explicit scalarization */
NOINLINE static float pattern3_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    float result = 0.0f;
    
    /* Extract each element manually - creates many extract operations */
    float* p1 = (float*)&v1;
    float* p2 = (float*)&v2;
    float* p3 = (float*)&v3;
    float* p4 = (float*)&v4;
    
    /* 16 extract operations total */
    for (int i = 0; i < 4; i++) {
        result += p1[i] + p2[i] + p3[i] + p4[i];
    }
    
    return result;
}

/* Pattern 4: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern4_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                                 v4sf thresh1, v4sf thresh2) {
    v4sf result;
    
#ifdef __SSE__
    __m128 va = _mm_loadu_ps((float*)&a);
    __m128 vb = _mm_loadu_ps((float*)&b);
    __m128 vc = _mm_loadu_ps((float*)&c);
    __m128 vd = _mm_loadu_ps((float*)&d);
    __m128 vt1 = _mm_loadu_ps((float*)&thresh1);
    __m128 vt2 = _mm_loadu_ps((float*)&thresh2);
    
    /* Multiple comparisons create many operands */
    __m128 cmp1 = _mm_cmpgt_ps(va, vt1);
    __m128 cmp2 = _mm_cmpgt_ps(vb, vt2);
    __m128 cmp3 = _mm_cmpgt_ps(vc, vt1);
    __m128 cmp4 = _mm_cmpgt_ps(vd, vt2);
    
    /* Complex logical operations */
    __m128 mask1 = _mm_and_ps(cmp1, cmp2);
    __m128 mask2 = _mm_or_ps(cmp3, cmp4);
    __m128 final_mask = _mm_xor_ps(mask1, mask2);
    
    /* Conditional select */
    __m128 r = _mm_blendv_ps(va, vb, final_mask);
    __m128 r2 = _mm_blendv_ps(vc, vd, final_mask);
    __m128 final = _mm_add_ps(r, r2);
    
    _mm_storeu_ps((float*)&result, final);
#else
    result = a + b + c + d;
#endif
    
    return result;
}

/* Pattern 5: Inline assembly with exactly 11 operands */
NOINLINE static int pattern5_multi_operand_asm(int a, int b, int c, int d, int e,
                                               int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline assembly with 10 input/output operands plus clobbers */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "imul %[i], %[j]\n\t"
        "add %%eax, %%ebx\n\t"
        "mov %[result], %%eax"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "eax", "ebx", "cc"
    );
    
    return result;
}

/* Pattern 6: AVX2 gather operation with many indices (if available) */
NOINLINE static v8sf pattern6_gather_operation(float* base, v8si indices,
                                               v8sf scale_vec, v8sf default_val) {
    v8sf result = default_val;
    
#ifdef __AVX2__
    /* AVX2 gather can have many operands: base, indices, scale, mask, etc. */
    __m256i vidx = _mm256_loadu_si256((__m256i*)&indices);
    __m256 vscale = _mm256_loadu_ps((float*)&scale_vec);
    __m256 vdefault = _mm256_loadu_ps((float*)&default_val);
    
    /* Gather with all parameters */
    __m256 gathered = _mm256_i32gather_ps(base, vidx, 4);
    __m256 scaled = _mm256_mul_ps(gathered, vscale);
    __m256 masked = _mm256_blendv_ps(vdefault, scaled, _mm256_cmp_ps(scaled, vdefault, _CMP_GT_OQ));
    
    _mm256_storeu_ps((float*)&result, masked);
#endif
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    volatile float checksum = 0.0f;
    
    /* Initialize test data with some variability based on argc */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf thresh1 = {2.5f, 2.5f, 2.5f, 2.5f};
    v4sf thresh2 = {7.5f, 7.5f, 7.5f, 7.5f};
    
    float scalars[12];
    for (int i = 0; i < 12; i++) {
        scalars[i] = (float)(i + 1) * (argc > 1 ? atof(argv[1]) : 1.0f);
    }
    
    /* Test Pattern 1: Complex shuffle */
    v4sf r1 = pattern1_shuffle_complex(v1, v2, v3, v4, 0x1B, 0x4E, 0xB1, 0x27);
    checksum += ((float*)&r1)[0] + ((float*)&r1)[1] + ((float*)&r1)[2] + ((float*)&r1)[3];
    
    /* Test Pattern 2: FMA chain */
    float r2 = pattern2_fma_chain(scalars[0], scalars[1], scalars[2], scalars[3],
                                  scalars[4], scalars[5], scalars[6], scalars[7],
                                  scalars[8], scalars[9], scalars[10], scalars[11]);
    checksum += r2;
    
    /* Test Pattern 3: Vector reduction */
    float r3 = pattern3_vector_reduction(v1, v2, v3, v4);
    checksum += r3;
    
    /* Test Pattern 4: Conditional select */
    v4sf r4 = pattern4_conditional_select(v1, v2, v3, v4, thresh1, thresh2);
    checksum += ((float*)&r4)[0] + ((float*)&r4)[1] + ((float*)&r4)[2] + ((float*)&r4)[3];
    
    /* Test Pattern 5: Multi-operand inline assembly */
    int r5 = pattern5_multi_operand_asm(argc, argc*2, argc*3, argc*4, argc*5,
                                        argc*6, argc*7, argc*8, argc*9, argc*10);
    checksum += (float)r5;
    
    /* Test Pattern 6: Gather operation (if available) */
#ifdef __AVX2__
    float base_array[64];
    for (int i = 0; i < 64; i++) base_array[i] = (float)i;
    
    v8si indices = {0, 8, 16, 24, 32, 40, 48, 56};
    v8sf scale = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    v8sf default_val = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    
    v8sf r6 = pattern6_gather_operation(base_array, indices, scale, default_val);
    float* r6p = (float*)&r6;
    for (int i = 0; i < 8; i++) checksum += r6p[i];
#endif
    
    /* Use checksum to prevent dead code elimination */
    printf("Checksum: %f\n", checksum);
    
    return (int)checksum % 256;
}
