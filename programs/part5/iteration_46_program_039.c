#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for deterministic testing */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
#include <immintrin.h>
typedef __m512i v512i;
typedef __m512 v512f;
typedef __mmask16 m16;
#elif defined(__AVX2__)
#include <immintrin.h>
typedef __m256i v256i;
typedef __m256 v256f;
#elif defined(__ARM_NEON)
#include <arm_neon.h>
typedef int32x4_t v128i;
typedef float32x4_t v128f;
#else
/* Fallback to generic types */
typedef int v128i __attribute__((vector_size(16)));
typedef float v128f __attribute__((vector_size(16)));
#endif

/* Prevent optimization of arguments */
#define NOOPT __attribute__((optimize("O0")))
#define NOINLINE __attribute__((noinline))
#define TARGET_AVX2 __attribute__((target("avx2")))
#define TARGET_AVX512F __attribute__((target("avx512f")))

/* Complex expression with many temporaries */
NOINLINE TARGET_AVX2
static void test_many_args_avx2(int* output, const int* input1, const int* input2,
                               const int* input3, const int* input4,
                               const float* finput1, const float* finput2,
                               int n) {
    volatile int vi = 0; /* Prevent loop unrolling */
    
    for (int i = 0; i < n; i += 8) {
        /* Load multiple vectors */
        v256i v1 = _mm256_loadu_si256((const __m256i*)(input1 + i));
        v256i v2 = _mm256_loadu_si256((const __m256i*)(input2 + i));
        v256i v3 = _mm256_loadu_si256((const __m256i*)(input3 + i));
        v256i v4 = _mm256_loadu_si256((const __m256i*)(input4 + i));
        
        /* Complex expression with many intermediate values */
        v256i t1 = _mm256_add_epi32(v1, v2);
        v256i t2 = _mm256_sub_epi32(v3, v4);
        v256i t3 = _mm256_mullo_epi32(t1, t2);
        v256i t4 = _mm256_slli_epi32(t3, 2);
        v256i t5 = _mm256_srli_epi32(t4, 1);
        
        /* Create a complex shuffle with many arguments through inline asm */
        /* This should trigger the 10-11 argument optab expansion */
        v256i result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpslld %0, %0, %5\n\t"
            "vpsrld %0, %0, %6\n\t"
            "vpslld %0, %0, %7\n\t"
            "vpsrld %0, %0, %8\n\t"
            "vpaddd %0, %0, %9"
            : "=x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4),
              "i"(2), "i"(1), "i"(3), "i"(2), "x"(t5)
            : "memory"
        );
        
        _mm256_storeu_si256((__m256i*)(output + i), result);
        
        /* Volatile dependency to prevent CSE */
        asm volatile("" : "+r"(vi));
    }
}

#ifdef __AVX512F__
NOINLINE TARGET_AVX512F
static void test_many_args_avx512(int* output, const int* input1, const int* input2,
                                 const int* input3, const int* input4,
                                 const float* finput1, const float* finput2,
                                 int n) {
    volatile int vi = 0;
    
    for (int i = 0; i < n; i += 16) {
        /* Load vectors */
        v512i v1 = _mm512_loadu_si512((const __m512i*)(input1 + i));
        v512i v2 = _mm512_loadu_si512((const __m512i*)(input2 + i));
        v512i v3 = _mm512_loadu_si512((const __m512i*)(input3 + i));
        v512i v4 = _mm512_loadu_si512((const __m512i*)(input4 + i));
        
        /* Complex operation chain */
        v512i t1 = _mm512_add_epi32(v1, v2);
        v512i t2 = _mm512_sub_epi32(v3, v4);
        v512i t3 = _mm512_mullo_epi32(t1, t2);
        
        /* Create mask with many conditions */
        m16 mask = _mm512_cmp_epi32_mask(t3, _mm512_setzero_si512(), _MM_CMPINT_GT);
        
        /* Complex blend operation with many arguments */
        /* This should require 10+ arguments in the optab expansion */
        v512i blended;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpslld %0, %0, %5\n\t"
            "vpsrld %0, %0, %6\n\t"
            "vpblendmd %0 %{%7%} %8, %9\n\t"
            "vpslld %0, %0, %10"
            : "=v"(blended)
            : "v"(v1), "v"(v2), "v"(v3), "v"(v4),
              "i"(2), "i"(1), "k"(mask), "v"(t1), "v"(t2), "i"(3)
            : "memory"
        );
        
        _mm512_storeu_si512((__m512i*)(output + i), blended);
        
        /* Prevent optimization */
        asm volatile("" : "+r"(vi));
    }
}
#endif

/* Generic version using GCC vector extensions */
NOINLINE NOOPT
static void test_many_args_generic(int* output, const int* input1, const int* input2,
                                  const int* input3, const int* input4,
                                  const float* finput1, const float* finput2,
                                  int n) {
    volatile int vi = 0;
    
    /* Use GCC vector builtins with many arguments */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    for (int i = 0; i < n; i += 4) {
        /* Load data */
        v4si v1 = *(const v4si*)(input1 + i);
        v4si v2 = *(const v4si*)(input2 + i);
        v4si v3 = *(const v4si*)(input3 + i);
        v4si v4 = *(const v4si*)(input4 + i);
        
        /* Complex shuffle with __builtin_shuffle - can take many arguments */
        /* Create a shuffle mask with 10+ elements */
        int mask[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        
        /* Force the compiler to use many arguments by creating complex expression */
        v4si temp1 = v1 + v2;
        v4si temp2 = v3 - v4;
        v4si temp3 = temp1 * temp2;
        v4si temp4 = temp3 << 2;
        v4si temp5 = temp4 >> 1;
        
        /* Complex inline asm with 11 arguments */
        v4si result;
        asm volatile (
            "movdqa %1, %0\n\t"
            "paddd %0, %2\n\t"
            "psubd %0, %3\n\t"
            "pmulld %0, %4\n\t"
            "pslld $2, %0\n\t"
            "psrld $1, %0\n\t"
            "pslld $3, %0\n\t"
            "psrld $2, %0\n\t"
            "paddd %0, %5\n\t"
            "psubd %0, %6"
            : "=x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(temp3), "x"(temp5),
              "m"(mask[0]), "m"(mask[4]), "m"(mask[8]), "m"(mask[12])
            : "memory"
        );
        
        *(v4si*)(output + i) = result;
        
        /* Prevent optimization */
        asm volatile("" : "+r"(vi));
    }
}

/* Function that uses math library with many arguments */
NOINLINE
static double complex_math_expression(double a, double b, double c, double d,
                                     double e, double f, double g, double h,
                                     double i, double j, double k) {
    /* Complex expression that might be optimized to a single optab call */
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    
    /* Force many argument usage */
    double result = ((v1 * v2 + v3) / (d - e)) * (f + g) - (h * i) / (j - k);
    
    /* Inline asm with 11 double arguments */
    asm volatile (
        "vmulsd %1, %2, %0\n\t"
        "vaddsd %0, %3, %0\n\t"
        "vdivsd %0, %4, %0\n\t"
        "vmulsd %0, %5, %0\n\t"
        "vsubsd %0, %6, %0\n\t"
        "vdivsd %0, %7, %0"
        : "=x"(result)
        : "x"(a), "x"(b), "x"(c), "x"(d), "x"(e), "x"(f), "x"(g),
          "x"(h), "x"(i), "x"(j), "x"(k)
        : "memory"
    );
    
    return result;
}

int main(void) {
    const int N = 1024;
    int* input1 = malloc(N * sizeof(int));
    int* input2 = malloc(N * sizeof(int));
    int* input3 = malloc(N * sizeof(int));
    int* input4 = malloc(N * sizeof(int));
    float* finput1 = malloc(N * sizeof(float));
    float* finput2 = malloc(N * sizeof(float));
    int* output = malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        input1[i] = (int)(prng_next() % 1000);
        input2[i] = (int)(prng_next() % 1000);
        input3[i] = (int)(prng_next() % 1000);
        input4[i] = (int)(prng_next() % 1000);
        finput1[i] = (float)(prng_next() % 1000) / 100.0f;
        finput2[i] = (float)(prng_next() % 1000) / 100.0f;
    }
    
    /* Call the many-argument functions */
#ifdef __AVX512F__
    test_many_args_avx512(output, input1, input2, input3, input4, finput1, finput2, N);
#elif defined(__AVX2__)
    test_many_args_avx2(output, input1, input2, input3, input4, finput1, finput2, N);
#else
    test_many_args_generic(output, input1, input2, input3, input4, finput1, finput2, N);
#endif
    
    /* Also test the complex math expression */
    double math_result = complex_math_expression(1.0, 2.0, 3.0, 4.0, 5.0,
                                                6.0, 7.0, 8.0, 9.0, 10.0, 11.0);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += (uint64_t)output[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    printf("Math result: %f\n", math_result);
    
    /* Cleanup */
    free(input1);
    free(input2);
    free(input3);
    free(input4);
    free(finput1);
    free(finput2);
    free(output);
    
    return 0;
}
