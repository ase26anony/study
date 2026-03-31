#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for deterministic testing */
static uint32_t prng_state = 0x12345678;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Volatile counter to prevent loop unrolling */
static volatile int volatile_counter = 0;

/* Complex expression with many temporaries */
#define CREATE_MANY_TEMPS(op1, op2, op3, op4, op5, op6, op7, op8, op9, op10) \
    ((((op1) + (op2)) * ((op3) - (op4))) & \
     (((op5) | (op6)) ^ ((op7) << 2)) | \
     (((op8) >> 3) + ((op9) * 2) - (op10)))

/* Target-specific function with many arguments */
__attribute__((target("avx2,avx512f"), noinline))
void test_many_args(int8_t* restrict out, const int8_t* in1, const int8_t* in2,
                    const int16_t* in3, const int32_t* in4, const float* in5,
                    const double* in6, const uint64_t* in7, int iterations) {
    
    /* Force dependencies to inhibit CSE */
    asm volatile("" : "+r"(iterations));
    
    for (int i = volatile_counter; i < iterations; i += 16) {
        /* Load multiple vectors - creating many operands */
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(in1 + i));
        __m256i v2 = _mm256_loadu_si256((const __m256i*)(in2 + i));
        __m256i v3 = _mm256_loadu_si256((const __m256i*)(in3 + i * 2));
        __m256i v4 = _mm256_loadu_si256((const __m256i*)(in4 + i * 4));
        __m256 v5 = _mm256_loadu_ps(in5 + i * 8);
        __m256d v6 = _mm256_loadu_pd(in6 + i * 4);
        __m256i v7 = _mm256_loadu_si256((const __m256i*)(in7 + i * 4));
        
        /* Complex multi-statement expression with many temporaries */
        __m256i temp1 = _mm256_add_epi8(v1, v2);
        __m256i temp2 = _mm256_sub_epi16(v3, _mm256_set1_epi16(42));
        __m256i temp3 = _mm256_and_si256(v4, _mm256_set1_epi32(0xFF));
        __m256 temp4 = _mm256_mul_ps(v5, _mm256_set1_ps(2.0f));
        __m256d temp5 = _mm256_add_pd(v6, _mm256_set1_pd(1.0));
        
        /* Extended inline asm with 10-11 operands */
        __m256i result;
        asm volatile (
            "vpaddb %[t1], %[v2], %[t1]\n\t"
            "vpsubw %[t2], %[v3], %[t2]\n\t"
            "vpand %[t3], %[v4], %[t3]\n\t"
            "vmulps %[t4], %[v5], %[t4]\n\t"
            "vaddpd %[t5], %[v6], %[t5]\n\t"
            "vpor %[t1], %[t2], %[t1]\n\t"
            "vpxor %[t3], %[t1], %[res]\n\t"
            : [res] "=x" (result),
              [t1] "+x" (temp1),
              [t2] "+x" (temp2),
              [t3] "+x" (temp3),
              [t4] "+x" (temp4),
              [t5] "+x" (temp5)
            : [v2] "x" (v2),
              [v3] "x" (v3),
              [v4] "x" (v4),
              [v5] "x" (v5),
              [v6] "x" (v6)
            : "memory"
        );
        
        /* Vector builtin with many arguments (simulated) */
        __m256i shuffled;
#ifdef __AVX512F__
        /* AVX-512 mask register usage */
        __mmask8 mask = 0xAA;
        shuffled = _mm256_mask_shuffle_epi32(result, mask, v7, _MM_SHUFFLE(2,3,0,1));
#else
        /* Complex shuffle with many immediate arguments */
        shuffled = _mm256_shuffle_epi8(result, 
            _mm256_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16));
#endif
        
        /* Store result */
        _mm256_storeu_si256((__m256i*)(out + i), shuffled);
        
        /* Inhibit optimization */
        asm volatile("" ::: "memory");
    }
}

/* Alternative approach using GCC vector extensions */
typedef int32_t v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

__attribute__((target("avx2"), noinline))
v8si complex_vector_expr(v8si a, v8si b, v8si c, v8si d,
                         v8sf e, v8sf f, v4df g, v4df h,
                         int imm1, int imm2, int imm3) {
    
    /* This complex expression should generate many operands */
    v8si t1 = a + b;
    v8si t2 = c - d;
    v8si t3 = t1 * t2;
    v8si t4 = a & b | c ^ d;
    
    /* Type conversions that might use optabs */
    v8si t5 = __builtin_convertvector(e, v8si);
    v8si t6 = __builtin_convertvector(f, v8si);
    
    /* Complex shuffle-like operation */
    v8si result = __builtin_shuffle(t3, t4, 
        (v8si){0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15});
    
    /* Mix in immediates */
    result = result + imm1;
    result = result * imm2;
    result = result | imm3;
    
    return result;
}

/* Function with exactly 11 scalar arguments */
__attribute__((noinline))
long scalar_many_args(long a1, long a2, long a3, long a4, long a5,
                      long a6, long a7, long a8, long a9, long a10,
                      long a11) {
    /* Complex expression tree */
    long t1 = (a1 * a2) + (a3 << 2);
    long t2 = (a4 & a5) | (a6 ^ a7);
    long t3 = (a8 - a9) * (a10 >> 3);
    long t4 = CREATE_MANY_TEMPS(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    
    /* Final computation using all 11 arguments */
    return (t1 + t2 - t3) * a11 + t4;
}

int main(void) {
    const int SIZE = 1024;
    const int ITERATIONS = 100;
    
    /* Allocate and initialize arrays with different types */
    int8_t* array1 = aligned_alloc(32, SIZE * sizeof(int8_t));
    int8_t* array2 = aligned_alloc(32, SIZE * sizeof(int8_t));
    int16_t* array3 = aligned_alloc(32, SIZE * sizeof(int16_t));
    int32_t* array4 = aligned_alloc(32, SIZE * sizeof(int32_t));
    float* array5 = aligned_alloc(32, SIZE * sizeof(float));
    double* array6 = aligned_alloc(32, SIZE * sizeof(double));
    uint64_t* array7 = aligned_alloc(32, SIZE * sizeof(uint64_t));
    int8_t* output = aligned_alloc(32, SIZE * sizeof(int8_t));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int8_t)prng_next();
        array2[i] = (int8_t)prng_next();
        array3[i] = (int16_t)prng_next();
        array4[i] = (int32_t)prng_next();
        array5[i] = (float)prng_next() / 1000.0f;
        array6[i] = (double)prng_next() / 1000.0;
        array7[i] = (uint64_t)prng_next();
    }
    
    /* Call the many-argument vector function */
    test_many_args(output, array1, array2, array3, array4, 
                   array5, array6, array7, SIZE);
    
    /* Test scalar function with 11 arguments */
    long scalar_result = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        scalar_result += scalar_many_args(
            prng_next(), prng_next(), prng_next(), prng_next(), prng_next(),
            prng_next(), prng_next(), prng_next(), prng_next(), prng_next(),
            prng_next()
        );
    }
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (uint64_t)output[i];
    }
    checksum += (uint64_t)scalar_result;
    
    printf("Checksum: %lu\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    free(array6);
    free(array7);
    free(output);
    
    return 0;
}
