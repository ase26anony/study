#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t fast_rand(void) {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
#include <immintrin.h>
typedef __m512i v512i;
typedef __m512 v512f;
typedef __mmask16 mask16;
#elif defined(__AVX2__)
#include <immintrin.h>
typedef __m256i v256i;
typedef __m256 v256f;
#elif defined(__ARM_NEON)
#include <arm_neon.h>
typedef int32x4_t v128i;
typedef float32x4_t v128f;
#else
/* Fallback scalar types */
typedef struct { int32_t v[4]; } v128i;
typedef struct { float v[4]; } v128f;
#endif

/* Prevent optimization */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR(var) asm volatile("" : "+r"(var))

/* Target-specific function attributes */
#ifdef __AVX512F__
#define TARGET_ATTR __attribute__((target("avx512f,avx512bw")))
#elif defined(__AVX2__)
#define TARGET_ATTR __attribute__((target("avx2")))
#elif defined(__ARM_NEON)
#define TARGET_ATTR __attribute__((target("+simd")))
#else
#define TARGET_ATTR
#endif

/* Complex expression with many temporaries */
NOINLINE TARGET_ATTR
static void test_many_args(int32_t* restrict out, 
                          const int32_t* restrict in1,
                          const int32_t* restrict in2,
                          const int32_t* restrict in3,
                          int n) {
    volatile int i = 0; /* Prevent loop unrolling */
    
    for (; i < n; i += 4) {
        /* Create many intermediate values to force complex expansion */
        int32_t a0 = in1[i] ^ in2[i];
        int32_t a1 = in1[i+1] | in2[i+1];
        int32_t a2 = in1[i+2] & in2[i+2];
        int32_t a3 = in1[i+3] + in2[i+3];
        
        int32_t b0 = in3[i] * 7;
        int32_t b1 = in3[i+1] * 13;
        int32_t b2 = in3[i+2] * 19;
        int32_t b3 = in3[i+3] * 31;
        
        /* Complex multi-statement expression with many temporaries */
        int32_t t0 = (a0 << 2) + (b0 >> 1);
        int32_t t1 = (a1 << 3) ^ (b1 << 1);
        int32_t t2 = (a2 >> 2) | (b2 >> 3);
        int32_t t3 = (a3 * 2) - (b3 / 2);
        
        int32_t u0 = t0 * t1 + t2;
        int32_t u1 = t1 * t2 + t3;
        int32_t u2 = t2 * t3 + t0;
        int32_t u3 = t3 * t0 + t1;
        
        /* Force dependency chain */
        VOLATILE_VAR(u0); VOLATILE_VAR(u1); VOLATILE_VAR(u2); VOLATILE_VAR(u3);
        
        /* Complex expression requiring many arguments */
        out[i]   = ((u0 & 0xFF) << 24) | ((u1 & 0xFF) << 16) | 
                   ((u2 & 0xFF) << 8)  | (u3 & 0xFF);
        out[i+1] = ((u1 & 0xFF00) >> 8) | ((u2 & 0xFF00) << 8) |
                   ((u3 & 0xFF00) << 16) | ((u0 & 0xFF00) << 24);
        out[i+2] = u0 ^ u1 ^ u2 ^ u3 ^ in1[i] ^ in2[i] ^ in3[i];
        out[i+3] = (u0 + u1 + u2 + u3) * 2 - (in1[i+3] + in2[i+3] + in3[i+3]);
    }
}

/* Function using inline asm with many operands */
NOINLINE TARGET_ATTR
static void asm_many_args(int32_t* restrict out,
                         const int32_t* restrict in1,
                         const int32_t* restrict in2,
                         const int32_t* restrict in3,
                         const int32_t* restrict in4,
                         int n) {
    for (int i = 0; i < n; i += 4) {
        /* Inline asm with 11 arguments - targeting the uncovered case */
        int32_t r0, r1, r2, r3;
        
        asm volatile (
            /* Complex operation with many inputs */
            "mov %[r0], %[a0]\n\t"
            "add %[r0], %[r0], %[a1]\n\t"
            "sub %[r0], %[r0], %[a2]\n\t"
            "xor %[r0], %[r0], %[a3]\n\t"
            "and %[r0], %[r0], %[b0]\n\t"
            "or  %[r0], %[r0], %[b1]\n\t"
            "shl %[r0], %[r0], %[imm1]\n\t"
            "shr %[r0], %[r0], %[imm2]\n\t"
            "imul %[r0], %[r0], %[imm3]\n\t"
            "add %[r0], %[r0], %[imm4]\n\t"
            "mov %[out0], %[r0]\n\t"
            
            : [out0] "=m" (out[i]),
              [r0] "=&r" (r0), [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3)
            : [a0] "r" (in1[i]), [a1] "r" (in1[i+1]), [a2] "r" (in1[i+2]), [a3] "r" (in1[i+3]),
              [b0] "r" (in2[i]), [b1] "r" (in2[i+1]),
              [imm1] "i" (2), [imm2] "i" (1), [imm3] "i" (3), [imm4] "i" (5)
            : "memory", "cc"
        );
        
        /* Another asm with 10 arguments */
        asm volatile (
            "lea %[r1], [%[a0] + %[a1] * 2]\n\t"
            "lea %[r2], [%[a2] + %[a3] * 4]\n\t"
            "lea %[r3], [%[b0] + %[b1] * 8]\n\t"
            "imul %[r1], %[r1], %[imm5]\n\t"
            "imul %[r2], %[r2], %[imm6]\n\t"
            "add %[r1], %[r1], %[r2]\n\t"
            "sub %[r1], %[r1], %[r3]\n\t"
            "mov %[out1], %[r1]\n\t"
            
            : [out1] "=m" (out[i+1]),
              [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3)
            : [a0] "r" (in3[i]), [a1] "r" (in3[i+1]), [a2] "r" (in3[i+2]), [a3] "r" (in3[i+3]),
              [b0] "r" (in4[i]), [b1] "r" (in4[i+1]),
              [imm5] "i" (7), [imm6] "i" (11)
            : "cc"
        );
    }
}

#ifdef __AVX2__
/* Vector version using AVX2 intrinsics with many arguments */
NOINLINE __attribute__((target("avx2")))
static void vector_many_args(int32_t* restrict out,
                            const int32_t* restrict in1,
                            const int32_t* restrict in2,
                            const int32_t* restrict in3,
                            int n) {
    for (int i = 0; i < n; i += 8) {
        /* Load multiple vectors */
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(in1 + i));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(in2 + i));
        __m256i v2 = _mm256_loadu_si256((const __m256i*)(in3 + i));
        
        /* Create many intermediate vectors */
        __m256i t0 = _mm256_add_epi32(v0, v1);
        __m256i t1 = _mm256_sub_epi32(v0, v1);
        __m256i t2 = _mm256_mullo_epi32(v0, v2);
        __m256i t3 = _mm256_slli_epi32(v1, 2);
        __m256i t4 = _mm256_srli_epi32(v2, 1);
        
        /* Complex blend operation - could expand to many arguments */
        __m256i mask = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
        __m256i shuffled = _mm256_permutevar8x32_epi32(t0, mask);
        
        /* Another complex operation */
        __m256i blended = _mm256_blend_epi32(t1, t2, 0xAA); /* 10101010 */
        __m256i blended2 = _mm256_blend_epi32(t3, t4, 0x55); /* 01010101 */
        
        /* Final combination */
        __m256i result = _mm256_add_epi32(shuffled, blended);
        result = _mm256_sub_epi32(result, blended2);
        result = _mm256_xor_si256(result, _mm256_set1_epi32(0xFFFFFFFF));
        
        _mm256_storeu_si256((__m256i*)(out + i), result);
    }
}
#endif

#ifdef __ARM_NEON
/* ARM NEON version with many arguments */
NOINLINE __attribute__((target("+simd")))
static void neon_many_args(int32_t* restrict out,
                          const int32_t* restrict in1,
                          const int32_t* restrict in2,
                          const int32_t* restrict in3,
                          int n) {
    for (int i = 0; i < n; i += 4) {
        /* Load vectors */
        int32x4_t v0 = vld1q_s32(in1 + i);
        int32x4_t v1 = vld1q_s32(in2 + i);
        int32x4_t v2 = vld1q_s32(in3 + i);
        
        /* Multiple operations creating many temporaries */
        int32x4_t t0 = vaddq_s32(v0, v1);
        int32x4_t t1 = vsubq_s32(v0, v1);
        int32x4_t t2 = vmulq_s32(v0, v2);
        int32x4_t t3 = vshlq_n_s32(v1, 2);
        int32x4_t t4 = vshrq_n_s32(v2, 1);
        
        /* Complex shuffle/permute */
        const int32_t mask[4] = {2, 3, 0, 1};
        int32x4_t shuffled = vqtbl1q_s8(t0, vld1q_s8((const int8_t*)mask));
        
        /* Blend operations */
        int32x4_t blended = vbslq_s32(vdupq_n_u32(0xF0F0F0F0), t1, t2);
        int32x4_t blended2 = vbslq_s32(vdupq_n_u32(0x0F0F0F0F), t3, t4);
        
        /* Final result */
        int32x4_t result = vaddq_s32(shuffled, blended);
        result = vsubq_s32(result, blended2);
        
        vst1q_s32(out + i, result);
    }
}
#endif

int main(void) {
    const int N = 1024;
    int32_t* in1 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* in2 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* in3 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* in4 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* out1 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* out2 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* out3 = aligned_alloc(64, N * sizeof(int32_t));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        in1[i] = fast_rand() % 1000;
        in2[i] = fast_rand() % 1000;
        in3[i] = fast_rand() % 1000;
        in4[i] = fast_rand() % 1000;
    }
    
    /* Test different functions */
    test_many_args(out1, in1, in2, in3, N);
    asm_many_args(out2, in1, in2, in3, in4, N);
    
#ifdef __AVX2__
    vector_many_args(out3, in1, in2, in3, N);
#elif defined(__ARM_NEON)
    neon_many_args(out3, in1, in2, in3, N);
#else
    memcpy(out3, out1, N * sizeof(int32_t));
#endif
    
    /* Compute checksums */
    uint64_t checksum1 = 0, checksum2 = 0, checksum3 = 0;
    for (int i = 0; i < N; i++) {
        checksum1 += out1[i];
        checksum2 += out2[i];
        checksum3 += out3[i];
    }
    
    printf("Checksum 1: %lu\n", checksum1);
    printf("Checksum 2: %lu\n", checksum2);
    printf("Checksum 3: %lu\n", checksum3);
    
    free(in1); free(in2); free(in3); free(in4);
    free(out1); free(out2); free(out3);
    
    return 0;
}
