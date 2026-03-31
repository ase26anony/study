#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t prng_u32(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Prevent optimization of variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex expression with many temporaries */
NOOPT static inline int complex_expr_10_args(
    int a, int b, int c, int d, int e,
    int f, int g, int h, int i, int j) {
    /* Force many intermediate temporaries */
    int t1 = a * b + c;
    int t2 = d << (e & 3);
    int t3 = f ^ g ^ h;
    int t4 = i * j - t1;
    int t5 = t2 | t3;
    int t6 = t4 & ~t5;
    int t7 = t6 + (a ^ d);
    int t8 = t7 * (b + e);
    int t9 = t8 >> (f & 7);
    return t9 ^ (g * h) ^ (i + j);
}

NOOPT static inline int complex_expr_11_args(
    int a, int b, int c, int d, int e,
    int f, int g, int h, int i, int j, int k) {
    /* Even more temporaries for 11 args */
    int t1 = a + b - c;
    int t2 = d * e / (f + 1);
    int t3 = g ^ h ^ i;
    int t4 = j << (k & 3);
    int t5 = t1 | t2;
    int t6 = t3 & t4;
    int t7 = t5 ^ t6;
    int t8 = (a * c) + (b * d);
    int t9 = t7 - t8;
    int t10 = t9 * (e + f);
    return t10 ^ (g * h) ^ (i * j) ^ k;
}

#ifdef __AVX512F__
__attribute__((target("avx512f,avx512bw")))
NOOPT static inline __m512i avx512_complex_shuffle_10_args(
    __m512i a, __m512i b, __m512i c, __m512i d,
    int imm1, int imm2, int imm3, int imm4,
    __mmask64 mask1, __mmask64 mask2) {
    
    /* Complex operation chain that might use 10-arg optab */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_sub_epi32(c, d);
    
    /* Shuffle with many arguments - may trigger optab */
    __m512i shuffled;
    asm volatile (
        "vpaddd %[t1], %[t2], %[shuf]\n\t"
        "vpshufd %[imm1], %[shuf], %[shuf]\n\t"
        "vpermq %[imm2], %[shuf], %[shuf]\n\t"
        : [shuf] "=v" (shuffled)
        : [t1] "v" (t1), [t2] "v" (t2),
          [imm1] "i" (imm1), [imm2] "i" (imm2)
        : 
    );
    
    /* Blend with masks - potentially 10+ args when expanded */
    __m512i blended = _mm512_mask_blend_epi32(mask1, shuffled, t1);
    blended = _mm512_mask_blend_epi64(mask2, blended, t2);
    
    return blended;
}

__attribute__((target("avx512f")))
NOOPT static inline __m512 avx512_math_11_args(
    __m512 a, __m512 b, __m512 c, __m512 d, __m512 e,
    float f, float g, float h, float i, float j, float k) {
    
    /* Complex floating-point expression with many args */
    __m512 t1 = _mm512_add_ps(a, b);
    __m512 t2 = _mm512_sub_ps(c, d);
    __m512 t3 = _mm512_mul_ps(e, _mm512_set1_ps(f));
    __m512 t4 = _mm512_div_ps(t1, _mm512_set1_ps(g));
    
    /* FMA chain - each FMA takes 3 args, chain of 4 = 11 total */
    __m512 result = _mm512_fmadd_ps(t1, t2, t3);
    result = _mm512_fnmadd_ps(result, t4, _mm512_set1_ps(h));
    result = _mm512_fmadd_ps(result, _mm512_set1_ps(i), _mm512_set1_ps(j));
    result = _mm512_fmsub_ps(result, _mm512_set1_ps(k), t1);
    
    return result;
}
#endif

#ifdef __AVX2__
__attribute__((target("avx2")))
NOOPT static void test_many_args_avx2(
    int32_t* restrict in1, int32_t* restrict in2,
    int32_t* restrict in3, int32_t* restrict in4,
    int32_t* restrict out, size_t n) {
    
    volatile size_t i; /* Prevent loop unrolling */
    
    for (i = 0; i < n; i += 8) {
        /* Load vectors */
        __m256i v1 = _mm256_loadu_si256((__m256i*)(in1 + i));
        __m256i v2 = _mm256_loadu_si256((__m256i*)(in2 + i));
        __m256i v3 = _mm256_loadu_si256((__m256i*)(in3 + i));
        __m256i v4 = _mm256_loadu_si256((__m256i*)(in4 + i));
        
        /* Complex inline asm with many operands - 10 args */
        __m256i result;
        asm volatile (
            "vpaddd %[v2], %[v1], %[tmp1]\n\t"
            "vpsubd %[v4], %[v3], %[tmp2]\n\t"
            "vpmulld %[tmp1], %[tmp2], %[tmp3]\n\t"
            "vpslld $2, %[tmp3], %[tmp4]\n\t"
            "vpsrld $1, %[tmp4], %[tmp5]\n\t"
            "vpblendd $0xAA, %[tmp3], %[tmp5], %[res]\n\t"
            : [res] "=x" (result),
              [tmp1] "=&x" (v1), [tmp2] "=&x" (v2),
              [tmp3] "=&x" (v3), [tmp4] "=&x" (v4),
              [tmp5] "=&x" (*(__m256i*)in1) /* Force dependency */
            : [v1] "0" (v1), [v2] "1" (v2),
              [v3] "2" (v3), [v4] "3" (v4)
            : "memory"
        );
        
        /* Store result */
        _mm256_storeu_si256((__m256i*)(out + i), result);
        
        /* Scalar complex expression with 11 args */
        int scalar_result = complex_expr_11_args(
            in1[i], in2[i], in3[i], in4[i],
            in1[i+1], in2[i+1], in3[i+1], in4[i+1],
            in1[i+2], in2[i+2], in3[i+2]
        );
        
        /* Mix scalar result into vector */
        out[i] ^= scalar_result;
    }
}
#endif

/* Generic version for non-AVX targets */
NOOPT static void test_many_args_generic(
    int32_t* restrict in1, int32_t* restrict in2,
    int32_t* restrict in3, int32_t* restrict in4,
    int32_t* restrict out, size_t n) {
    
    volatile size_t i;
    
    for (i = 0; i < n; i++) {
        /* Complex expression with exactly 10 arguments */
        int val1 = complex_expr_10_args(
            in1[i], in2[i], in3[i], in4[i],
            in1[(i + 1) % n], in2[(i + 2) % n],
            in3[(i + 3) % n], in4[(i + 4) % n],
            in1[(i + 5) % n], in2[(i + 6) % n]
        );
        
        /* Another with 11 arguments */
        int val2 = complex_expr_11_args(
            in1[i], in2[i], in3[i], in4[i],
            in1[(i + 1) % n], in2[(i + 2) % n],
            in3[(i + 3) % n], in4[(i + 4) % n],
            in1[(i + 5) % n], in2[(i + 6) % n],
            in3[(i + 7) % n]
        );
        
        /* Combine results */
        out[i] = val1 ^ val2;
        
        /* Inline asm with many memory operands - 10+ args */
        asm volatile (
            "addl %[a], %[b]\n\t"
            "subl %[c], %[d]\n\t"
            "andl %[e], %[f]\n\t"
            "orl  %[g], %[h]\n\t"
            "xorl %[i], %[j]\n\t"
            : [b] "+r" (out[i]),
              [d] "+r" (out[(i + 1) % n]),
              [f] "+r" (out[(i + 2) % n]),
              [h] "+r" (out[(i + 3) % n]),
              [j] "+r" (out[(i + 4) % n])
            : [a] "m" (in1[i]), [c] "m" (in2[i]),
              [e] "m" (in3[i]), [g] "m" (in4[i]),
              [i] "m" (in1[(i + 1) % n])
            : "memory", "cc"
        );
    }
}

/* NEON version for ARM */
#ifdef __ARM_NEON
#include <arm_neon.h>

NOOPT static void test_many_args_neon(
    int32_t* restrict in1, int32_t* restrict in2,
    int32_t* restrict in3, int32_t* restrict in4,
    int32_t* restrict out, size_t n) {
    
    volatile size_t i;
    
    for (i = 0; i < n; i += 4) {
        /* Load NEON vectors */
        int32x4_t v1 = vld1q_s32(in1 + i);
        int32x4_t v2 = vld1q_s32(in2 + i);
        int32x4_t v3 = vld1q_s32(in3 + i);
        int32x4_t v4 = vld1q_s32(in4 + i);
        
        /* Complex NEON operations that may use many args */
        int32x4_t t1 = vaddq_s32(v1, v2);
        int32x4_t t2 = vsubq_s32(v3, v4);
        int32x4_t t3 = vmulq_s32(t1, t2);
        
        /* Vector shuffle with table lookup - potentially many args */
        uint8x16_t tbl_idx = {0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15};
        int32x4_t shuffled = vqtbl1q_s8(vreinterpretq_s8_s32(t3), tbl_idx);
        
        /* Blend with condition */
        uint32x4_t mask = vcltq_s32(t1, t2);
        int32x4_t result = vbslq_s32(mask, shuffled, t3);
        
        /* Store result */
        vst1q_s32(out + i, result);
        
        /* Inline asm with many vector operands */
        asm volatile (
            "vadd.i32 %q[t1], %q[v1], %q[v2]\n\t"
            "vsub.i32 %q[t2], %q[v3], %q[v4]\n\t"
            "vmul.i32 %q[res], %q[t1], %q[t2]\n\t"
            "vext.8   %q[res], %q[res], %q[res], #4\n\t"
            : [res] "=w" (result),
              [t1] "=&w" (t1), [t2] "=&w" (t2)
            : [v1] "w" (v1), [v2] "w" (v2),
              [v3] "w" (v3), [v4] "w" (v4)
            : 
        );
    }
}
#endif

int main(void) {
    const size_t N = 1024;
    int32_t* in1 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* in2 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* in3 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* in4 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* out = aligned_alloc(64, N * sizeof(int32_t));
    
    /* Initialize with pseudo-random data */
    for (size_t i = 0; i < N; i++) {
        in1[i] = (int32_t)prng_u32();
        in2[i] = (int32_t)prng_u32();
        in3[i] = (int32_t)prng_u32();
        in4[i] = (int32_t)prng_u32();
        out[i] = 0;
    }
    
    /* Call appropriate test based on architecture */
#if defined(__AVX512F__)
    printf("Using AVX-512 path\n");
    test_many_args_avx2(in1, in2, in3, in4, out, N);
#elif defined(__AVX2__)
    printf("Using AVX2 path\n");
    test_many_args_avx2(in1, in2, in3, in4, out, N);
#elif defined(__ARM_NEON)
    printf("Using NEON path\n");
    test_many_args_neon(in1, in2, in3, in4, out, N);
#else
    printf("Using generic path\n");
    test_many_args_generic(in1, in2, in3, in4, out, N);
#endif
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (size_t i = 0; i < N; i++) {
        checksum += (uint64_t)out[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Cleanup */
    free(in1);
    free(in2);
    free(in3);
    free(in4);
    free(out);
    
    return 0;
}
