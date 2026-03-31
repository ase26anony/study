#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
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
/* Fallback to generic types */
typedef int32_t v128i __attribute__((vector_size(16)));
typedef float v128f __attribute__((vector_size(16)));
#endif

/* Function to inhibit optimization */
static inline void inhibit_opt(volatile int* var) {
    asm volatile("" : "+r"(*var) : : "memory");
}

/* Complex expression with many temporaries - forces expander to handle many operands */
__attribute__((noinline, target("avx2,avx512f")))
void test_many_args(int* output, const int* input1, const int* input2, 
                    const int* input3, const int* input4, int n) {
    volatile int iter_counter = 0; /* Prevent loop unrolling */
    
    for (int i = 0; i < n; i += 16) {
        inhibit_opt(&iter_counter);
        
#ifdef __AVX512F__
        /* AVX-512 implementation with many arguments */
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(input1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(input2 + i));
        __m512i v3 = _mm512_loadu_si512((const __m512i*)(input3 + i));
        __m512i v4 = _mm512_loadu_si512((const __m512i*)(input4 + i));
        
        /* Complex shuffle with many arguments - potentially triggering 10-11 arg optab */
        __m512i shuffled = _mm512_shuffle_epi32(v1, _MM_PERM_ABCD);
        
        /* Extended inline asm with 11 operands - targeting uncovered lines */
        __m512i result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpaddd %0, %0, %5\n\t"
            "vpsubd %0, %0, %6\n\t"
            "vpmulld %0, %0, %7\n\t"
            "vpaddd %0, %0, %8\n\t"
            "vpsubd %0, %0, %9\n\t"
            "vpmulld %0, %0, %10"
            : "=v"(result)
            : "v"(v1), "v"(v2), "v"(v3), "v"(v4),
              "v"(shuffled), "v"(_mm512_set1_epi32(1)),
              "v"(_mm512_set1_epi32(2)), "v"(_mm512_set1_epi32(3)),
              "v"(_mm512_set1_epi32(4)), "v"(_mm512_set1_epi32(5))
            : "memory"
        );
        
        _mm512_storeu_si512((__m512i*)(output + i), result);
#elif defined(__AVX2__)
        /* AVX2 implementation with complex expressions */
        __m256i v1_lo = _mm256_loadu_si256((const __m256i*)(input1 + i));
        __m256i v1_hi = _mm256_loadu_si256((const __m256i*)(input1 + i + 8));
        __m256i v2_lo = _mm256_loadu_si256((const __m256i*)(input2 + i));
        __m256i v2_hi = _mm256_loadu_si256((const __m256i*)(input2 + i + 8));
        
        /* Multi-statement expression with many temporaries */
        __m256i t1 = _mm256_add_epi32(v1_lo, v2_lo);
        __m256i t2 = _mm256_sub_epi32(v1_hi, v2_hi);
        __m256i t3 = _mm256_mullo_epi32(t1, t2);
        __m256i t4 = _mm256_slli_epi32(t3, 2);
        __m256i t5 = _mm256_srli_epi32(t4, 1);
        
        /* Complex blend operation with many arguments */
        __m256i blend_mask = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
        __m256i result_lo = _mm256_blendv_epi8(t1, t2, blend_mask);
        __m256i result_hi = _mm256_blendv_epi8(t3, t4, blend_mask);
        
        /* Extended asm with 10 operands */
        __m256i final_result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpaddd %0, %0, %5\n\t"
            "vpsubd %0, %0, %6\n\t"
            "vpmulld %0, %0, %7\n\t"
            "vpaddd %0, %0, %8\n\t"
            "vpsubd %0, %0, %9"
            : "=v"(final_result)
            : "v"(result_lo), "v"(result_hi), 
              "v"(_mm256_set1_epi32(1)), "v"(_mm256_set1_epi32(2)),
              "v"(_mm256_set1_epi32(3)), "v"(_mm256_set1_epi32(4)),
              "v"(_mm256_set1_epi32(5)), "v"(_mm256_set1_epi32(6)),
              "v"(_mm256_set1_epi32(7))
            : "memory"
        );
        
        _mm256_storeu_si256((__m256i*)(output + i), final_result);
        _mm256_storeu_si256((__m256i*)(output + i + 8), final_result);
#else
        /* Generic implementation with complex pointer arithmetic */
        for (int j = 0; j < 16 && (i + j) < n; j++) {
            /* Complex expression with many temporaries */
            int a = input1[i + j];
            int b = input2[i + j];
            int c = input3[i + j];
            int d = input4[i + j];
            
            /* Multi-step computation forcing many operands */
            int t1 = a + b;
            int t2 = c - d;
            int t3 = t1 * t2;
            int t4 = t3 << 2;
            int t5 = t4 >> 1;
            int t6 = t5 & 0xFF;
            int t7 = t6 | 0x80;
            int t8 = t7 ^ 0x55;
            int t9 = t8 * 3;
            int t10 = t9 / 2;
            
            /* Extended asm with 11 arguments */
            int result;
            asm volatile (
                "add %0, %1, %2\n\t"
                "sub %0, %0, %3\n\t"
                "mul %0, %0, %4\n\t"
                "add %0, %0, %5\n\t"
                "sub %0, %0, %6\n\t"
                "mul %0, %0, %7\n\t"
                "add %0, %0, %8\n\t"
                "sub %0, %0, %9\n\t"
                "mul %0, %0, %10"
                : "=r"(result)
                : "r"(t1), "r"(t2), "r"(t3), "r"(t4),
                  "r"(t5), "r"(t6), "r"(t7), "r"(t8),
                  "r"(t9), "r"(t10)
                : "memory"
            );
            
            output[i + j] = result;
        }
#endif
        
        iter_counter++;
    }
}

/* Vector permutation with many arguments */
__attribute__((noinline, target("avx2")))
void vector_permute_complex(int* output, const int* input, int n) {
    volatile int counter = 0;
    
    for (int i = 0; i < n; i += 8) {
        inhibit_opt(&counter);
        
#ifdef __AVX2__
        __m256i v = _mm256_loadu_si256((const __m256i*)(input + i));
        
        /* Complex permutation pattern - could expand to many arguments */
        __m256i perm_mask = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
        __m256i permuted = _mm256_permutevar8x32_epi32(v, perm_mask);
        
        /* Extended asm with 10 memory operands */
        __m256i result;
        asm volatile (
            "vpermq %0, %1, %2\n\t"
            "vpshufd %0, %0, %3\n\t"
            "vpaddd %0, %0, %4\n\t"
            "vpsubd %0, %0, %5\n\t"
            "vpmulld %0, %0, %6\n\t"
            "vpand %0, %0, %7\n\t"
            "vpor %0, %0, %8\n\t"
            "vpxor %0, %0, %9"
            : "=v"(result)
            : "v"(v), "v"(permuted),
              "i"(0x1B), /* Immediate constant */
              "v"(_mm256_set1_epi32(1)),
              "v"(_mm256_set1_epi32(2)),
              "v"(_mm256_set1_epi32(3)),
              "v"(_mm256_set1_epi32(0xFF)),
              "v"(_mm256_set1_epi32(0x80)),
              "v"(_mm256_set1_epi32(0x55))
            : "memory"
        );
        
        _mm256_storeu_si256((__m256i*)(output + i), result);
#else
        /* Generic implementation */
        for (int j = 0; j < 8 && (i + j) < n; j++) {
            int idx = (j + 3) % 8;
            output[i + j] = input[i + idx] + j;
        }
#endif
        
        counter++;
    }
}

/* Function using GCC vector builtins with many arguments */
__attribute__((noinline))
void use_vector_builtins(int* output, const int* input, int n) {
    typedef int v4si __attribute__((vector_size(16)));
    
    for (int i = 0; i < n; i += 4) {
        /* Load vectors */
        v4si v1 = *(const v4si*)(input + i);
        v4si v2 = *(const v4si*)(input + i + 4);
        
        /* Create complex shuffle mask */
        int mask_arr[4] = {3, 2, 1, 0};
        v4si mask = *(v4si*)mask_arr;
        
        /* Use __builtin_shuffle with many arguments */
        v4si shuffled = __builtin_shuffle(v1, v2, mask);
        
        /* Complex expression chain */
        v4si t1 = v1 + v2;
        v4si t2 = v1 - v2;
        v4si t3 = t1 * t2;
        v4si t4 = t3 << 2;
        v4si t5 = t4 >> 1;
        v4si t6 = shuffled + t5;
        
        /* Extended asm with 11 vector arguments */
        v4si result;
        asm volatile (
            "paddd %0, %1, %2\n\t"
            "psubd %0, %0, %3\n\t"
            "pmulld %0, %0, %4\n\t"
            "paddd %0, %0, %5\n\t"
            "psubd %0, %0, %6\n\t"
            "pmulld %0, %0, %7\n\t"
            "paddd %0, %0, %8\n\t"
            "psubd %0, %0, %9\n\t"
            "pmulld %0, %0, %10"
            : "=x"(result)
            : "x"(t1), "x"(t2), "x"(t3), "x"(t4),
              "x"(t5), "x"(t6), "x"(shuffled),
              "x"(*(v4si*)mask_arr), "x"(v1), "x"(v2)
            : "memory"
        );
        
        *(v4si*)(output + i) = result;
    }
}

int main() {
    const int N = 1024;
    int* input1 = (int*)aligned_alloc(64, N * sizeof(int));
    int* input2 = (int*)aligned_alloc(64, N * sizeof(int));
    int* input3 = (int*)aligned_alloc(64, N * sizeof(int));
    int* input4 = (int*)aligned_alloc(64, N * sizeof(int));
    int* output1 = (int*)aligned_alloc(64, N * sizeof(int));
    int* output2 = (int*)aligned_alloc(64, N * sizeof(int));
    int* output3 = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        input1[i] = (int)prng_next() % 1000;
        input2[i] = (int)prng_next() % 1000;
        input3[i] = (int)prng_next() % 1000;
        input4[i] = (int)prng_next() % 1000;
    }
    
    /* Call functions that use many-argument operations */
    test_many_args(output1, input1, input2, input3, input4, N);
    vector_permute_complex(output2, input1, N);
    use_vector_builtins(output3, input1, N);
    
    /* Compute checksums to ensure computation happened */
    long long checksum1 = 0, checksum2 = 0, checksum3 = 0;
    for (int i = 0; i < N; i++) {
        checksum1 += output1[i];
        checksum2 += output2[i];
        checksum3 += output3[i];
    }
    
    printf("Checksum 1: %lld\n", checksum1);
    printf("Checksum 2: %lld\n", checksum2);
    printf("Checksum 3: %lld\n", checksum3);
    
    /* Cleanup */
    free(input1);
    free(input2);
    free(input3);
    free(input4);
    free(output1);
    free(output2);
    free(output3);
    
    return 0;
}
