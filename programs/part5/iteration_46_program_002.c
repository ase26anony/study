#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t prng() {
    seed = seed * 1103515245 + 12345;
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

/* Function to inhibit optimization */
static inline void inhibit_opt(volatile int* var) {
    asm volatile("" : "+r"(*var));
}

/* Complex expression with many temporaries - forces expander to handle many operands */
__attribute__((noinline, target("avx2")))
static void test_many_args(int* output, const int* input1, const int* input2, 
                          const int* input3, const int* input4, int n) {
    volatile int iter = 0;  /* Prevent loop unrolling */
    
    for (int i = 0; i < n; i += 8) {
        inhibit_opt(&iter);
        
        /* Load multiple vectors - creates many temporaries */
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(input1 + i));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(input1 + i + 8));
        __m256i v2 = _mm256_loadu_si256((const __m256i*)(input2 + i));
        __m256i v3 = _mm256_loadu_si256((const __m256i*)(input2 + i + 8));
        __m256i v4 = _mm256_loadu_si256((const __m256i*)(input3 + i));
        __m256i v5 = _mm256_loadu_si256((const __m256i*)(input3 + i + 8));
        __m256i v6 = _mm256_loadu_si256((const __m256i*)(input4 + i));
        __m256i v7 = _mm256_loadu_si256((const __m256i*)(input4 + i + 8));
        
        /* Complex multi-statement expression with many intermediate values */
        __m256i t0 = _mm256_add_epi32(v0, v1);
        __m256i t1 = _mm256_sub_epi32(v2, v3);
        __m256i t2 = _mm256_mullo_epi32(v4, v5);
        __m256i t3 = _mm256_and_si256(v6, v7);
        __m256i t4 = _mm256_or_si256(t0, t1);
        __m256i t5 = _mm256_xor_si256(t2, t3);
        __m256i t6 = _mm256_slli_epi32(t4, 3);
        __m256i t7 = _mm256_srli_epi32(t5, 2);
        
        /* Extended inline asm with 10-11 operands - targets optabs expansion */
        __m256i result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpandd %0, %0, %5\n\t"
            "vporq %0, %0, %6\n\t"
            "vpxorq %0, %0, %7\n\t"
            "vpslld $2, %0, %0\n\t"
            "vpsrld $1, %0, %0"
            : "=v"(result)
            : "v"(t6), "v"(t7), "v"(v0), "v"(v1), "v"(v2), "v"(v3), "v"(v4),
              "m"(*(const __m256i*)(input1 + i)), "m"(*(const __m256i*)(input2 + i))
            : "memory"
        );
        
        _mm256_storeu_si256((__m256i*)(output + i), result);
        
        iter++;
    }
}

/* Alternative approach using GCC vector builtins with many arguments */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
static void test_many_args_avx512(int* output, const int* input, int n) {
    volatile int iter = 0;
    
    for (int i = 0; i < n; i += 16) {
        inhibit_opt(&iter);
        
        /* Load multiple vectors */
        __m512i v0 = _mm512_loadu_si512((const __m512i*)(input + i));
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(input + i + 16));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(input + i + 32));
        __m512i v3 = _mm512_loadu_si512((const __m512i*)(input + i + 48));
        
        /* Create mask with many elements */
        __mmask16 mask = _mm512_cmpgt_epi32_mask(v0, v1);
        
        /* Complex blend operation with many arguments */
        __m512i blended;
        asm volatile (
            "vpblendmd %0, %1, %2, %3\n\t"
            "vpaddd %0, %0, %4\n\t"
            "vpsubd %0, %0, %5\n\t"
            "vpmulld %0, %0, %6\n\t"
            "vpslld $3, %0, %0\n\t"
            "vpsrld $1, %0, %0"
            : "=v"(blended)
            : "v"(v0), "v"(v1), "k"(mask), "v"(v2), "v"(v3),
              "m"(*(const __m512i*)(input + i)), 
              "m"(*(const __m512i*)(input + i + 16)),
              "m"(*(const __m512i*)(input + i + 32)),
              "m"(*(const __m512i*)(input + i + 48))
            : "memory"
        );
        
        _mm512_storeu_si512((__m512i*)(output + i), blended);
        iter++;
    }
}
#endif

/* ARM NEON version with many-lane shuffles */
#ifdef __ARM_NEON
__attribute__((noinline))
static void test_many_args_neon(int* output, const int* input, int n) {
    volatile int iter = 0;
    
    for (int i = 0; i < n; i += 4) {
        inhibit_opt(&iter);
        
        /* Load multiple vectors */
        int32x4_t v0 = vld1q_s32(input + i);
        int32x4_t v1 = vld1q_s32(input + i + 4);
        int32x4_t v2 = vld1q_s32(input + i + 8);
        int32x4_t v3 = vld1q_s32(input + i + 12);
        
        /* Complex shuffle/permute with many lane indices */
        int32x4_t shuffled;
        asm volatile (
            "vtrn.32 %q0, %q1\n\t"
            "vrev64.32 %q0, %q0\n\t"
            "vadd.i32 %q0, %q0, %q2\n\t"
            "vsub.i32 %q0, %q0, %q3\n\t"
            "vmul.i32 %q0, %q0, %q1"
            : "=w"(shuffled)
            : "w"(v0), "w"(v1), "w"(v2), "w"(v3),
              "m"(*(const int32x4_t*)(input + i)),
              "m"(*(const int32x4_t*)(input + i + 4)),
              "m"(*(const int32x4_t*)(input + i + 8)),
              "m"(*(const int32x4_t*)(input + i + 12)),
              "r"(i), "r"(n)
            : "memory"
        );
        
        vst1q_s32(output + i, shuffled);
        iter++;
    }
}
#endif

/* Generic version using GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));

__attribute__((noinline))
static void test_many_args_generic(int* output, const int* input, int n) {
    volatile int iter = 0;
    
    for (int i = 0; i < n; i += 8) {
        inhibit_opt(&iter);
        
        /* Load using memcpy to prevent optimization */
        v8si v0, v1, v2, v3;
        memcpy(&v0, input + i, sizeof(v8si));
        memcpy(&v1, input + i + 8, sizeof(v8si));
        memcpy(&v2, input + i + 16, sizeof(v8si));
        memcpy(&v3, input + i + 24, sizeof(v8si));
        
        /* Complex expression with many operations */
        v8si t0 = v0 + v1;
        v8si t1 = v2 - v3;
        v8si t2 = v0 * v1;
        v8si t3 = v2 & v3;
        v8si t4 = t0 | t1;
        v8si t5 = t2 ^ t3;
        v8si t6 = t4 << 3;
        v8si t7 = t5 >> 2;
        
        /* Use __builtin_shuffle with many indices */
        int indices[16] = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
        v8si shuffled0 = __builtin_shuffle(t6, t7, 
            indices[0], indices[1], indices[2], indices[3],
            indices[4], indices[5], indices[6], indices[7]);
        v8si shuffled1 = __builtin_shuffle(t6, t7,
            indices[8], indices[9], indices[10], indices[11],
            indices[12], indices[13], indices[14], indices[15]);
        
        /* Final operation with many arguments */
        v8si result = shuffled0 + shuffled1 + t0 - t1 * t2 / (t3 | t4) ^ t5;
        
        memcpy(output + i, &result, sizeof(v8si));
        iter++;
    }
}

int main() {
    const int N = 1024;
    int* input1 = (int*)aligned_alloc(64, N * sizeof(int));
    int* input2 = (int*)aligned_alloc(64, N * sizeof(int));
    int* input3 = (int*)aligned_alloc(64, N * sizeof(int));
    int* input4 = (int*)aligned_alloc(64, N * sizeof(int));
    int* output = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        input1[i] = (int)(prng() % 1000);
        input2[i] = (int)(prng() % 1000);
        input3[i] = (int)(prng() % 1000);
        input4[i] = (int)(prng() % 1000);
        output[i] = 0;
    }
    
    /* Test different implementations */
    #ifdef __AVX2__
    test_many_args(output, input1, input2, input3, input4, N);
    #elif defined(__ARM_NEON)
    test_many_args_neon(output, input1, N);
    #else
    test_many_args_generic(output, input1, N);
    #endif
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += (uint64_t)output[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    free(input1);
    free(input2);
    free(input3);
    free(input4);
    free(output);
    
    return 0;
}
