#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

// Simple PRNG for reproducible results
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

// Initialize arrays with pseudo-random data
static void init_arrays(__m256i* arr_i32, __m256i* arr_i64, 
                       __m256* arr_f32, __m256d* arr_f64,
                       size_t size) {
    for (size_t i = 0; i < size; i++) {
        // Initialize integer vectors
        int32_t i32_data[8];
        int64_t i64_data[4];
        for (int j = 0; j < 8; j++) i32_data[j] = (int32_t)prng_next();
        for (int j = 0; j < 4; j++) i64_data[j] = (int64_t)prng_next() << 32 | prng_next();
        arr_i32[i] = _mm256_loadu_si256((const __m256i*)i32_data);
        arr_i64[i] = _mm256_loadu_si256((const __m256i*)i64_data);
        
        // Initialize float vectors
        float f32_data[8];
        double f64_data[4];
        for (int j = 0; j < 8; j++) f32_data[j] = (float)prng_next() / (float)UINT32_MAX;
        for (int j = 0; j < 4; j++) f64_data[j] = (double)prng_next() / (double)UINT32_MAX;
        arr_f32[i] = _mm256_loadu_ps(f32_data);
        arr_f64[i] = _mm256_loadu_pd(f64_data);
    }
}

// Complex expression with many temporaries to force optab expansion
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* out, const __m256i* in1, const __m256i* in2,
                          const __m256i* in3, const __m256i* in4,
                          const __m256* f32_in, const __m256d* f64_in,
                          size_t size) {
    // Volatile counter to prevent loop unrolling
    volatile size_t counter = 0;
    
    for (size_t i = 0; i < size; i++) {
        // Load multiple vectors
        __m256i v1 = _mm256_loadu_si256(&in1[i]);
        __m256i v2 = _mm256_loadu_si256(&in2[i]);
        __m256i v3 = _mm256_loadu_si256(&in3[i]);
        __m256i v4 = _mm256_loadu_si256(&in4[i]);
        __m256 fv1 = _mm256_loadu_ps(&f32_in[i]);
        __m256d dv1 = _mm256_loadu_pd(&f64_in[i]);
        
        // Create many intermediate values with complex expressions
        // This builds up a dependency chain with many temporaries
        
        // 1. Integer arithmetic chain
        __m256i t1 = _mm256_add_epi32(v1, v2);
        __m256i t2 = _mm256_sub_epi32(v3, v4);
        __m256i t3 = _mm256_mullo_epi32(t1, t2);
        
        // 2. Floating point conversions and operations
        __m256i t4 = _mm256_cvtps_epi32(fv1);
        __m256 fv2 = _mm256_cvtepi32_ps(t4);
        __m256 fv3 = _mm256_add_ps(fv1, fv2);
        
        // 3. Complex shuffle/permute operations with many arguments
        // This is where we try to trigger the 10-11 argument optab
        
        // Method 1: Extended inline assembly with 11 operands
        __m256i result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpaddd %0, %0, %5\n\t"
            "vpsubd %0, %0, %6\n\t"
            "vpaddd %0, %0, %7\n\t"
            "vpsubd %0, %0, %8\n\t"
            "vpaddd %0, %0, %9\n\t"
            "vpsubd %0, %0, %10"
            : "=x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4),
              "x"(t1), "x"(t2), "x"(t3), "x"(t4),
              "m"(in1[i]), "m"(in2[i])  // Memory operands
            : "memory"
        );
        
        // Method 2: Complex builtin usage with many arguments
        // Create a complex mask for shuffling
        int mask[8] = {7, 6, 5, 4, 3, 2, 1, 0};
        __m256i mask_vec = _mm256_loadu_si256((const __m256i*)mask);
        
        // Use inline assembly with shuffle that could expand to many arguments
        __m256i shuffled;
        asm volatile (
            "vpermd %1, %2, %0"
            : "=x"(shuffled)
            : "x"(result), "x"(mask_vec)
        );
        
        // Combine results with floating point data
        __m256i float_as_int = _mm256_castps_si256(fv3);
        __m256i final_result = _mm256_xor_si256(shuffled, float_as_int);
        
        // Another complex expression with type conversions
        __m256d dv2 = _mm256_add_pd(dv1, _mm256_set1_pd(1.0));
        __m256i double_as_int = _mm256_castpd_si256(dv2);
        final_result = _mm256_add_epi64(final_result, double_as_int);
        
        // Store result
        _mm256_storeu_si256(&out[i], final_result);
        
        // Update volatile counter to prevent optimizations
        counter++;
    }
}

// Alternative function using AVX-512 with mask registers
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
static void test_avx512_many_args(__m512i* out, const __m512i* in1, const __m512i* in2,
                                 const __m512i* in3, const __m512i* in4,
                                 const __m512i* in5, const __m512i* in6,
                                 size_t size) {
    volatile size_t counter = 0;
    
    for (size_t i = 0; i < size; i++) {
        // Load 6 input vectors
        __m512i v1 = _mm512_loadu_si512(&in1[i]);
        __m512i v2 = _mm512_loadu_si512(&in2[i]);
        __m512i v3 = _mm512_loadu_si512(&in3[i]);
        __m512i v4 = _mm512_loadu_si512(&in4[i]);
        __m512i v5 = _mm512_loadu_si512(&in5[i]);
        __m512i v6 = _mm512_loadu_si512(&in6[i]);
        
        // Create complex mask with many conditions
        __mmask16 mask1 = 0xAAAA;  // 1010101010101010
        __mmask16 mask2 = 0x5555;  // 0101010101010101
        __mmask16 mask3 = 0xCCCC;  // 1100110011001100
        __mmask16 mask4 = 0x3333;  // 0011001100110011
        
        // Complex blending with multiple masks - could trigger many-argument optab
        __m512i t1 = _mm512_mask_add_epi32(v1, mask1, v2, v3);
        __m512i t2 = _mm512_mask_sub_epi32(v4, mask2, v5, v6);
        __m512i t3 = _mm512_mask_mullo_epi32(t1, mask3, t2, v1);
        __m512i t4 = _mm512_mask_add_epi32(t3, mask4, v2, v3);
        
        // Extended inline assembly with 11 arguments
        __m512i result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpaddd %0, %0, %5\n\t"
            "vpsubd %0, %0, %6\n\t"
            "vpaddd %0, %0, %7\n\t"
            "vpsubd %0, %0, %8\n\t"
            "vpaddd %0, %0, %9\n\t"
            "vpsubd %0, %0, %10"
            : "=x"(result)
            : "x"(t1), "x"(t2), "x"(t3), "x"(t4),
              "x"(v1), "x"(v2), "x"(v3), "x"(v4),
              "m"(in1[i]), "m"(in2[i])
            : "memory"
        );
        
        // Complex shuffle with immediate indices
        __m512i shuffled = _mm512_shuffle_epi32(result, _MM_PERM_ABCD);
        
        // More operations to create complex expression tree
        __m512i final_result = _mm512_xor_si512(shuffled, v5);
        final_result = _mm512_add_epi32(final_result, v6);
        
        _mm512_storeu_si512(&out[i], final_result);
        counter++;
    }
}
#endif

// Function with complex multi-statement expression using many temporaries
__attribute__((noinline))
static int64_t complex_expression_test(int a, int b, int c, int d, int e,
                                      int f, int g, int h, int i, int j,
                                      int k, int l, int m, int n, int o) {
    // Create many intermediate values with different types
    char t1 = (char)(a + b);
    short t2 = (short)(c - d);
    int t3 = e * f;
    long t4 = (long)g * h;
    
    // Pointer arithmetic with multiple temporaries
    int arr[16];
    int* p1 = &arr[a % 16];
    int* p2 = &arr[b % 16];
    int* p3 = &arr[c % 16];
    int* p4 = &arr[d % 16];
    int* p5 = &arr[e % 16];
    
    // Complex expression with many operations
    int r1 = *p1 + *p2 - *p3 * *p4 / (*p5 + 1);
    int r2 = t1 * t2 + t3 - (int)t4;
    
    // Bitwise operations with many arguments
    int bits = (a & b) | (c & d) ^ (e & f) | (g & h) ^ (i & j) | (k & l) ^ (m & n);
    
    // Final complex expression
    int64_t result = (int64_t)r1 * r2 + bits - t1 + t2 - t3 + t4;
    
    // Use inline assembly to prevent optimization
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

int main() {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_SIZE = ARRAY_SIZE / 8;  // 8 elements per __m256i
    
    // Allocate aligned memory for better performance
    __m256i* arr_i32_1 = (__m256i*)aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* arr_i32_2 = (__m256i*)aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* arr_i32_3 = (__m256i*)aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* arr_i32_4 = (__m256i*)aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* arr_i64 = (__m256i*)aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256* arr_f32 = (__m256*)aligned_alloc(32, VEC_SIZE * sizeof(__m256));
    __m256d* arr_f64 = (__m256d*)aligned_alloc(32, VEC_SIZE * sizeof(__m256d));
    __m256i* output = (__m256i*)aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    
    if (!arr_i32_1 || !arr_i32_2 || !arr_i32_3 || !arr_i32_4 || 
        !arr_i64 || !arr_f32 || !arr_f64 || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    init_arrays(arr_i32_1, arr_i64, arr_f32, arr_f64, VEC_SIZE);
    memcpy(arr_i32_2, arr_i32_1, VEC_SIZE * sizeof(__m256i));
    memcpy(arr_i32_3, arr_i32_1, VEC_SIZE * sizeof(__m256i));
    memcpy(arr_i32_4, arr_i32_1, VEC_SIZE * sizeof(__m256i));
    
    // Run the test with many-argument operations
    test_many_args(output, arr_i32_1, arr_i32_2, arr_i32_3, arr_i32_4,
                   arr_f32, arr_f64, VEC_SIZE);
    
    // Compute checksum
    int64_t checksum = 0;
    int32_t* out_data = (int32_t*)output;
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        checksum += out_data[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    // Test complex scalar expression
    int64_t scalar_result = complex_expression_test(
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    printf("Scalar test result: %ld\n", scalar_result);
    
#ifdef __AVX512F__
    // Test AVX-512 version if available
    __m512i* avx512_in1 = (__m512i*)aligned_alloc(64, 64 * sizeof(__m512i));
    __m512i* avx512_in2 = (__m512i*)aligned_alloc(64, 64 * sizeof(__m512i));
    __m512i* avx512_in3 = (__m512i*)aligned_alloc(64, 64 * sizeof(__m512i));
    __m512i* avx512_in4 = (__m512i*)aligned_alloc(64, 64 * sizeof(__m512i));
    __m512i* avx512_in5 = (__m512i*)aligned_alloc(64, 64 * sizeof(__m512i));
    __m512i* avx512_in6 = (__m512i*)aligned_alloc(64, 64 * sizeof(__m512i));
    __m512i* avx512_out = (__m512i*)aligned_alloc(64, 64 * sizeof(__m512i));
    
    if (avx512_in1 && avx512_in2 && avx512_in3 && avx512_in4 &&
        avx512_in5 && avx512_in6 && avx512_out) {
        test_avx512_many_args(avx512_out, avx512_in1, avx512_in2, avx512_in3,
                             avx512_in4, avx512_in5, avx512_in6, 64);
        
        // Clean up AVX-512 allocations
        free(avx512_in1); free(avx512_in2); free(avx512_in3);
        free(avx512_in4); free(avx512_in5); free(avx512_in6);
        free(avx512_out);
    }
#endif
    
    // Clean up
    free(arr_i32_1); free(arr_i32_2); free(arr_i32_3); free(arr_i32_4);
    free(arr_i64); free(arr_f32); free(arr_f64);
    free(output);
    
    return 0;
}
