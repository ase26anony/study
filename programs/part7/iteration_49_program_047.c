/* Test program to trigger 10 and 11 operand RTL patterns in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force no inlining to ensure separate RTL expansion */
#define NOINLINE __attribute__((noinline))

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
    #define TARGET_X86 1
    #include <immintrin.h>
    #include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
    #define TARGET_ARM 1
    #include <arm_neon.h>
#else
    #define TARGET_GENERIC 1
#endif

/* Prevent dead code elimination */
volatile int sink;

/* ============================================
 * Function to trigger 10-operand RTL pattern
 * ============================================ */
NOINLINE void test_10_operand(void) {
#if TARGET_X86
    /* AVX-512 complex permute with mask - known to generate many operands */
    #ifdef __AVX512F__
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* vpermi2q/vpermt2q with mask can generate complex patterns */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    sink = _mm512_extract_epi64(result, 0);
    #endif
    
    /* Alternative: Complex blend with multiple immediates */
    #ifdef __AVX2__
    __m256i a = _mm256_set1_epi32(1);
    __m256i b = _mm256_set1_epi32(2);
    __m256i c = _mm256_set1_epi32(3);
    
    /* Multi-operand shuffle/blend chain */
    __m256i t1 = _mm256_blend_epi32(a, b, 0xCC);
    __m256i t2 = _mm256_blend_epi32(c, t1, 0xAA);
    __m256i t3 = _mm256_permutevar8x32_epi32(t2, _mm256_set_epi32(0,1,2,3,4,5,6,7));
    __m256i t4 = _mm256_slli_epi32(t3, 2);
    __m256i t5 = _mm256_add_epi32(t4, _mm256_set1_epi32(1));
    
    sink = _mm256_extract_epi32(t5, 0);
    #endif
    
#elif TARGET_ARM
    /* ARM NEON complex operations with multiple registers */
    int32x4_t v1 = vdupq_n_s32(1);
    int32x4_t v2 = vdupq_n_s32(2);
    int32x4_t v3 = vdupq_n_s32(3);
    int32x4_t v4 = vdupq_n_s32(4);
    
    /* Complex sequence that might combine into multi-operand pattern */
    int32x4_t r1 = vaddq_s32(v1, v2);
    int32x4_t r2 = vmulq_s32(v3, v4);
    int32x4_t r3 = vmlaq_s32(r1, r2, vdupq_n_s32(5));
    
    /* Multiple lane operations */
    int32x2_t lo = vget_low_s32(r3);
    int32x2_t hi = vget_high_s32(r3);
    int32x2_t sum = vadd_s32(lo, hi);
    
    sink = vget_lane_s32(sum, 0);
    
#else
    /* Generic fallback: Inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        /* Complex multi-operand calculation */
        "add %[r], %[a], %[b]\n\t"
        "add %[r], %[r], %[c]\n\t"
        "add %[r], %[r], %[d]\n\t"
        "add %[r], %[r], %[e]\n\t"
        "add %[r], %[r], %[f]\n\t"
        "add %[r], %[r], %[g]\n\t"
        "add %[r], %[r], %[h]\n\t"
        "add %[r], %[r], %[i]\n\t"
        "add %[r], %[r], %[j]"
        : [r] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    sink = result;
#endif
}

/* =============================================
 * Function to trigger 11-operand RTL pattern
 * ============================================= */
NOINLINE void test_11_operand(void) {
#if TARGET_X86
    /* AVX-512 masked gather with complex addressing - can generate 11 operands */
    #ifdef __AVX512F__
    long long base[64] = {0};
    __m512i vindex = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i src = _mm512_set1_epi64(42);
    __mmask8 mask = 0x0F;
    int scale = 8;
    
    /* Gather with mask, scale, and displacement */
    __m512i result = _mm512_mask_i64gather_epi64(src, mask, vindex, 
                                                (const void*)base, scale);
    
    sink = _mm512_extract_epi64(result, 0);
    #endif
    
    /* Complex arithmetic chain */
    #ifdef __AVX__
    __m256 v1 = _mm256_set1_ps(1.0f);
    __m256 v2 = _mm256_set1_ps(2.0f);
    __m256 v3 = _mm256_set1_ps(3.0f);
    __m256 v4 = _mm256_set1_ps(4.0f);
    __m256 v5 = _mm256_set1_ps(5.0f);
    
    /* FMA chain that might combine */
    __m256 r1 = _mm256_fmadd_ps(v1, v2, v3);
    __m256 r2 = _mm256_fmsub_ps(v4, v5, r1);
    __m256 r3 = _mm256_add_ps(r2, _mm256_set1_ps(6.0f));
    __m256 r4 = _mm256_mul_ps(r3, _mm256_set1_ps(0.5f));
    
    float temp[8];
    _mm256_storeu_ps(temp, r4);
    sink = (int)temp[0];
    #endif
    
#elif TARGET_ARM
    /* ARM complex vector operations */
    float32x4_t f1 = vdupq_n_f32(1.0f);
    float32x4_t f2 = vdupq_n_f32(2.0f);
    float32x4_t f3 = vdupq_n_f32(3.0f);
    float32x4_t f4 = vdupq_n_f32(4.0f);
    float32x4_t f5 = vdupq_n_f32(5.0f);
    
    /* Complex FMA-like chain */
    float32x4_t r1 = vmlaq_f32(f1, f2, f3);
    float32x4_t r2 = vmlsq_f32(f4, f5, r1);
    float32x4_t r3 = vaddq_f32(r2, vdupq_n_f32(6.0f));
    
    float temp[4];
    vst1q_f32(temp, r3);
    sink = (int)temp[0];
    
#else
    /* Generic fallback: Inline assembly with exactly 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    asm volatile (
        /* 11-operand calculation */
        "mov %[r], %[a]\n\t"
        "add %[r], %[r], %[b]\n\t"
        "add %[r], %[r], %[c]\n\t"
        "add %[r], %[r], %[d]\n\t"
        "add %[r], %[r], %[e]\n\t"
        "add %[r], %[r], %[f]\n\t"
        "add %[r], %[r], %[g]\n\t"
        "add %[r], %[r], %[h]\n\t"
        "add %[r], %[r], %[i]\n\t"
        "add %[r], %[r], %[j]\n\t"
        "add %[r], %[r], %[k]"
        : [r] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j),
          [k] "r" (k)
        : "cc"
    );
    
    sink = result;
#endif
}

/* =============================================
 * Additional complex pattern that might trigger
 * multi-operand expansion during optimization
 * ============================================= */
NOINLINE void complex_vector_pattern(void) {
#if TARGET_X86 && defined(__AVX512F__)
    /* This pattern might generate complex RTL with many operands */
    __m512i v1 = _mm512_setr_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i v2 = _mm512_setr_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i mask = _mm512_set1_epi32(0xFFFFFFFF);
    
    /* Complex permute chain */
    __m512i p1 = _mm512_permutexvar_epi32(_mm512_setr_epi32(
        0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15), v1);
    
    __m512i p2 = _mm512_permutex2var_epi32(v1, 
        _mm512_setr_epi32(0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23), v2);
    
    __m512i blended = _mm512_mask_blend_epi32(0xAAAA, p1, p2);
    
    sink = _mm512_extract_epi32(blended, 0);
#endif
}

/* =============================================
 * Main driver
 * ============================================= */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    complex_vector_pattern();
    
    printf("Result: %d\n", sink);
    return 0;
}
