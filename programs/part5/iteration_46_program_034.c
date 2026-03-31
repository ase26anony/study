#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t prng_u32(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(__m256i* arr_i32, __m256i* arr_i64, 
                       __m256* arr_f32, __m256d* arr_f64,
                       size_t size) {
    for (size_t i = 0; i < size; i++) {
        uint32_t vals[8];
        for (int j = 0; j < 8; j++) vals[j] = prng_u32();
        arr_i32[i] = _mm256_set_epi32(vals[7], vals[6], vals[5], vals[4],
                                      vals[3], vals[2], vals[1], vals[0]);
        
        uint64_t vals64[4];
        for (int j = 0; j < 4; j++) vals64[j] = ((uint64_t)prng_u32() << 32) | prng_u32();
        arr_i64[i] = _mm256_set_epi64x(vals64[3], vals64[2], vals64[1], vals64[0]);
        
        float fvals[8];
        for (int j = 0; j < 8; j++) fvals[j] = (float)prng_u32() / (float)UINT32_MAX;
        arr_f32[i] = _mm256_set_ps(fvals[7], fvals[6], fvals[5], fvals[4],
                                   fvals[3], fvals[2], fvals[1], fvals[0]);
        
        double dvals[4];
        for (int j = 0; j < 4; j++) dvals[j] = (double)prng_u32() / (double)UINT32_MAX;
        arr_f64[i] = _mm256_set_pd(dvals[3], dvals[2], dvals[1], dvals[0]);
    }
}

/* Complex shuffle with many arguments - targeting 10-11 argument optab */
__attribute__((target("avx2,avx512f")))
__attribute__((noinline))
static __m256i complex_shuffle_10_args(__m256i a, __m256i b, __m256i c, 
                                      __m256i d, __m256i mask1, __m256i mask2,
                                      int imm1, int imm2, int imm3, int imm4) {
    /* Use inline asm with many operands to trigger optab expansion */
    __m256i result;
    
    /* Extended asm with 11 operands (including the output) */
    asm volatile (
        /* Complex operation with many inputs */
        "vpblendvb %[mask1], %[a], %[b], %[temp1]\n\t"
        "vpshufb %[mask2], %[c], %[temp2]\n\t"
        "vpaddd %[temp1], %[temp2], %[temp3]\n\t"
        "vpslld $%[imm1], %[temp3], %[temp4]\n\t"
        "vpsrld $%[imm2], %[d], %[temp5]\n\t"
        "vpor %[temp4], %[temp5], %[temp6]\n\t"
        "vpslld $%[imm3], %[temp6], %[temp7]\n\t"
        "vpsrld $%[imm4], %[temp7], %[out]"
        : [out] "=x" (result),
          [temp1] "=&x" (*(volatile __m256i*)&a),  /* Force dependency */
          [temp2] "=&x" (*(volatile __m256i*)&b),
          [temp3] "=&x" (*(volatile __m256i*)&c),
          [temp4] "=&x" (*(volatile __m256i*)&d),
          [temp5] "=&x" (mask1),
          [temp6] "=&x" (mask2),
          [temp7] "=&x" (result)  /* Self-dependency */
        : [a] "x" (a),
          [b] "x" (b),
          [c] "x" (c),
          [d] "x" (d),
          [mask1] "x" (mask1),
          [mask2] "x" (mask2),
          [imm1] "i" (imm1),
          [imm2] "i" (imm2),
          [imm3] "i" (imm3),
          [imm4] "i" (imm4)
        : "memory"
    );
    
    return result;
}

/* Another function using builtins with many arguments */
__attribute__((target("avx2")))
__attribute__((noinline))
static __m256 complex_convert_11_args(__m256 a, __m256 b, __m256 c, __m256 d,
                                     __m256 e, __m256 f, __m256 g, __m256 h,
                                     int imm1, int imm2, int imm3) {
    /* Create complex expression with many temporaries */
    volatile __m256 temp1 = _mm256_add_ps(a, b);
    volatile __m256 temp2 = _mm256_sub_ps(c, d);
    volatile __m256 temp3 = _mm256_mul_ps(e, f);
    volatile __m256 temp4 = _mm256_div_ps(g, h);
    
    /* Force dependency chain */
    asm volatile("" : "+x" (temp1), "+x" (temp2), "+x" (temp3), "+x" (temp4));
    
    /* Complex blend with many arguments */
    __m256 result = _mm256_blend_ps(
        _mm256_blend_ps(temp1, temp2, imm1),
        _mm256_blend_ps(temp3, temp4, imm2),
        imm3
    );
    
    /* Additional shuffle to increase argument count */
    result = _mm256_permutevar8x32_ps(result, 
        _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0));
    
    return result;
}

/* Main test function with hot loop */
__attribute__((target("avx2,avx512f")))
__attribute__((noinline))
static void test_many_args(__m256i* out_i32, __m256* out_f32,
                          const __m256i* in_i32, const __m256* in_f32,
                          size_t size) {
    /* Volatile counter to prevent loop unrolling */
    volatile size_t counter = 0;
    
    for (size_t i = 0; i < size; i++) {
        /* Load multiple vectors */
        __m256i v1 = in_i32[(i + 0) % size];
        __m256i v2 = in_i32[(i + 1) % size];
        __m256i v3 = in_i32[(i + 2) % size];
        __m256i v4 = in_i32[(i + 3) % size];
        
        __m256 f1 = in_f32[(i + 0) % size];
        __m256 f2 = in_f32[(i + 1) % size];
        __m256 f3 = in_f32[(i + 2) % size];
        __m256 f4 = in_f32[(i + 3) % size];
        __m256 f5 = in_f32[(i + 4) % size];
        __m256 f6 = in_f32[(i + 5) % size];
        __m256 f7 = in_f32[(i + 6) % size];
        __m256 f8 = in_f32[(i + 7) % size];
        
        /* Create mask vectors */
        __m256i mask1 = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
        __m256i mask2 = _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7);
        
        /* Call function with 10 arguments */
        __m256i shuffled = complex_shuffle_10_args(v1, v2, v3, v4,
                                                  mask1, mask2,
                                                  (i & 7) + 1,
                                                  (i & 3) + 1,
                                                  (i & 1) + 1,
                                                  (i & 7));
        
        /* Call function with 11 arguments */
        __m256 converted = complex_convert_11_args(f1, f2, f3, f4, f5, f6, f7, f8,
                                                  (i & 3),
                                                  (i & 7),
                                                  (i & 1));
        
        /* Store results */
        out_i32[i] = shuffled;
        out_f32[i] = converted;
        
        /* Update volatile counter */
        counter++;
    }
}

/* Compute checksum for validation */
static uint64_t compute_checksum(const __m256i* arr_i32, const __m256* arr_f32,
                                size_t size) {
    uint64_t checksum = 0;
    
    for (size_t i = 0; i < size; i++) {
        /* Process integer array */
        alignas(32) int32_t vals_i32[8];
        _mm256_store_si256((__m256i*)vals_i32, arr_i32[i]);
        
        for (int j = 0; j < 8; j++) {
            checksum += (uint64_t)(vals_i32[j] ^ (vals_i32[j] >> 16));
        }
        
        /* Process float array */
        alignas(32) float vals_f32[8];
        _mm256_store_ps(vals_f32, arr_f32[i]);
        
        for (int j = 0; j < 8; j++) {
            uint32_t float_as_int;
            memcpy(&float_as_int, &vals_f32[j], sizeof(float_as_int));
            checksum += float_as_int;
        }
    }
    
    return checksum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    
    /* Allocate aligned memory */
    __m256i* in_i32 = (__m256i*)_mm_malloc(ARRAY_SIZE * sizeof(__m256i), 32);
    __m256i* out_i32 = (__m256i*)_mm_malloc(ARRAY_SIZE * sizeof(__m256i), 32);
    __m256* in_f32 = (__m256*)_mm_malloc(ARRAY_SIZE * sizeof(__m256), 32);
    __m256* out_f32 = (__m256*)_mm_malloc(ARRAY_SIZE * sizeof(__m256), 32);
    
    if (!in_i32 || !out_i32 || !in_f32 || !out_f32) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize input arrays */
    init_arrays(in_i32, NULL, in_f32, NULL, ARRAY_SIZE);
    
    /* Run the test */
    test_many_args(out_i32, out_f32, in_i32, in_f32, ARRAY_SIZE);
    
    /* Compute and print checksum */
    uint64_t checksum = compute_checksum(out_i32, out_f32, ARRAY_SIZE);
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Cleanup */
    _mm_free(in_i32);
    _mm_free(out_i32);
    _mm_free(in_f32);
    _mm_free(out_f32);
    
    return 0;
}
