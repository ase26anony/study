#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

// Simple PRNG for reproducible results
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

// Force no inlining to ensure function boundaries
__attribute__((noinline, target("avx2,avx512f")))
void test_many_args(float* restrict out, 
                    const float* restrict in1,
                    const float* restrict in2,
                    const float* restrict in3,
                    const float* restrict in4,
                    int n) {
    // Volatile counter to prevent loop unrolling
    volatile int i = 0;
    
    for (; i < n; i += 8) {
        // Load multiple vectors
        __m256 v1 = _mm256_loadu_ps(&in1[i]);
        __m256 v2 = _mm256_loadu_ps(&in2[i]);
        __m256 v3 = _mm256_loadu_ps(&in3[i]);
        __m256 v4 = _mm256_loadu_ps(&in4[i]);
        
        // Create complex shuffle mask with many immediate arguments
        // This should trigger the 10-argument case
        __m256 shuffled;
        
        // Method 1: Complex builtin with many arguments
        // Using inline asm with 10-11 operands
        __asm__ volatile (
            "vmovaps %1, %%ymm0\n\t"
            "vmovaps %2, %%ymm1\n\t"
            "vmovaps %3, %%ymm2\n\t"
            "vmovaps %4, %%ymm3\n\t"
            // Complex blend operation with many control inputs
            "vblendvps %%ymm0, %%ymm1, %%ymm2, %%ymm4\n\t"
            "vblendvps %%ymm3, %%ymm4, %%ymm0, %%ymm5\n\t"
            "vpermps %%ymm5, %%ymm1, %%ymm6\n\t"
            "vmovaps %%ymm6, %0\n\t"
            : "=m" (shuffled)
            : "m" (v1), "m" (v2), "m" (v3), "m" (v4),
              "i" (0xCC), "i" (0xAA), "i" (0xF0), 
              "i" (0x0F), "i" (0x33), "i" (0x55)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
        );
        
        // Store result
        _mm256_storeu_ps(&out[i], shuffled);
    }
}

// Another function targeting 11 arguments with vector operations
__attribute__((noinline, target("avx512f")))
void test_11_args(double* restrict out,
                  const double* restrict in1,
                  const double* restrict in2,
                  const double* restrict in3,
                  const double* restrict in4,
                  const double* restrict in5,
                  int n) {
    volatile int i = 0;
    
    for (; i < n; i += 8) {
        // Load vectors
        __m512d v1 = _mm512_loadu_pd(&in1[i]);
        __m512d v2 = _mm512_loadu_pd(&in2[i]);
        __m512d v3 = _mm512_loadu_pd(&in3[i]);
        __m512d v4 = _mm512_loadu_pd(&in4[i]);
        __m512d v5 = _mm512_loadu_pd(&in5[i]);
        
        // Complex expression with many temporaries
        __m512d temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
        
        // Create fake dependencies to prevent optimization
        __asm__ volatile("" : "+x" (v1), "+x" (v2), "+x" (v3), "+x" (v4), "+x" (v5));
        
        // Multi-statement expression forcing many temporaries
        temp1 = _mm512_add_pd(v1, v2);
        temp2 = _mm512_mul_pd(v3, v4);
        temp3 = _mm512_sub_pd(temp1, temp2);
        temp4 = _mm512_fmadd_pd(v5, v1, v2);
        temp5 = _mm512_fnmadd_pd(v3, v4, v5);
        temp6 = _mm512_add_pd(temp3, temp4);
        temp7 = _mm512_sub_pd(temp5, temp6);
        temp8 = _mm512_mul_pd(temp7, _mm512_set1_pd(2.0));
        
        // Inline asm with 11 arguments
        __m512d result;
        __asm__ volatile (
            "vmovapd %1, %%zmm0\n\t"
            "vmovapd %2, %%zmm1\n\t"
            "vmovapd %3, %%zmm2\n\t"
            "vmovapd %4, %%zmm3\n\t"
            "vmovapd %5, %%zmm4\n\t"
            // Complex permute with many control inputs
            "vpermpd %%zmm0, %6, %%zmm5\n\t"
            "vpermpd %%zmm1, %7, %%zmm6\n\t"
            "vpermpd %%zmm2, %8, %%zmm7\n\t"
            "vpermpd %%zmm3, %9, %%zmm8\n\t"
            "vpermpd %%zmm4, %10, %%zmm9\n\t"
            "vaddpd %%zmm5, %%zmm6, %%zmm10\n\t"
            "vaddpd %%zmm7, %%zmm8, %%zmm11\n\t"
            "vaddpd %%zmm10, %%zmm11, %%zmm12\n\t"
            "vaddpd %%zmm12, %%zmm9, %0\n\t"
            : "=m" (result)
            : "m" (temp1), "m" (temp2), "m" (temp3), "m" (temp4), "m" (temp5),
              "i" (0x1B), "i" (0x27), "i" (0x39), "i" (0x4A), "i" (0x5C), "i" (0x6D)
            : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", 
              "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "memory"
        );
        
        _mm512_storeu_pd(&out[i], result);
    }
}

// Function using GCC vector builtins with many arguments
__attribute__((noinline))
void test_vector_builtins(int32_t* restrict out,
                          const int32_t* restrict in1,
                          const int32_t* restrict in2,
                          const int32_t* restrict in3,
                          const int32_t* restrict in4,
                          int n) {
    typedef int32_t v8si __attribute__((vector_size(32)));
    
    volatile int i = 0;
    for (; i < n; i += 8) {
        // Load vectors
        v8si v1 = *(v8si*)&in1[i];
        v8si v2 = *(v8si*)&in1[i];
        v8si v3 = *(v8si*)&in2[i];
        v8si v4 = *(v8si*)&in3[i];
        v8si v5 = *(v8si*)&in4[i];
        
        // Complex shuffle with many arguments using GCC builtins
        // This is designed to potentially trigger the 10-argument case
        v8si mask = {0, 2, 4, 6, 8, 10, 12, 14};
        
        // Create a complex expression that might be expanded into many operands
        v8si result = __builtin_shuffle(v1, v2, v3, v4, v5,
                                        mask, mask, mask, mask, mask);
        
        // Force the compiler to consider all arguments
        __asm__ volatile("" : "+x" (result));
        
        // Store with another complex operation
        *(v8si*)&out[i] = result + v1 - v2 * v3 / (v4 | v5);
    }
}

// Complex mathematical function with many arguments
__attribute__((noinline, target("avx2")))
float complex_math_10_args(float a, float b, float c, float d, float e,
                          float f, float g, float h, float i, float j) {
    // Create a complex expression that might be optimized into vector ops
    __m256 va = _mm256_set_ps(j, i, h, g, f, e, d, c);
    __m256 vb = _mm256_set_ps(a, b, c, d, e, f, g, h);
    
    __m256 vc = _mm256_add_ps(va, vb);
    __m256 vd = _mm256_mul_ps(vc, _mm256_set1_ps(2.0f));
    __m256 ve = _mm256_sub_ps(vd, _mm256_set1_ps(1.0f));
    
    // Horizontal sum
    __m128 vlow = _mm256_castps256_ps128(ve);
    __m128 vhigh = _mm256_extractf128_ps(ve, 1);
    vlow = _mm_add_ps(vlow, vhigh);
    
    __m128 shuf = _mm_movehdup_ps(vlow);
    __m128 sums = _mm_add_ps(vlow, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    
    return _mm_cvtss_f32(sums);
}

int main() {
    const int N = 1024;
    
    // Allocate and initialize arrays with pseudo-random data
    float* f1 = aligned_alloc(32, N * sizeof(float));
    float* f2 = aligned_alloc(32, N * sizeof(float));
    float* f3 = aligned_alloc(32, N * sizeof(float));
    float* f4 = aligned_alloc(32, N * sizeof(float));
    float* fout = aligned_alloc(32, N * sizeof(float));
    
    double* d1 = aligned_alloc(64, N * sizeof(double));
    double* d2 = aligned_alloc(64, N * sizeof(double));
    double* d3 = aligned_alloc(64, N * sizeof(double));
    double* d4 = aligned_alloc(64, N * sizeof(double));
    double* d5 = aligned_alloc(64, N * sizeof(double));
    double* dout = aligned_alloc(64, N * sizeof(double));
    
    int32_t* i1 = aligned_alloc(32, N * sizeof(int32_t));
    int32_t* i2 = aligned_alloc(32, N * sizeof(int32_t));
    int32_t* i3 = aligned_alloc(32, N * sizeof(int32_t));
    int32_t* i4 = aligned_alloc(32, N * sizeof(int32_t));
    int32_t* iout = aligned_alloc(32, N * sizeof(int32_t));
    
    // Initialize with pseudo-random data
    for (int i = 0; i < N; i++) {
        f1[i] = (prng_next() % 1000) / 100.0f;
        f2[i] = (prng_next() % 1000) / 100.0f;
        f3[i] = (prng_next() % 1000) / 100.0f;
        f4[i] = (prng_next() % 1000) / 100.0f;
        
        d1[i] = (prng_next() % 1000) / 100.0;
        d2[i] = (prng_next() % 1000) / 100.0;
        d3[i] = (prng_next() % 1000) / 100.0;
        d4[i] = (prng_next() % 1000) / 100.0;
        d5[i] = (prng_next() % 1000) / 100.0;
        
        i1[i] = prng_next() % 1000;
        i2[i] = prng_next() % 1000;
        i3[i] = prng_next() % 1000;
        i4[i] = prng_next() % 1000;
    }
    
    // Test functions with many arguments
    test_many_args(fout, f1, f2, f3, f4, N);
    test_11_args(dout, d1, d2, d3, d4, d5, N);
    test_vector_builtins(iout, i1, i2, i3, i4, N);
    
    // Also test scalar function with 10 arguments
    float scalar_result = 0;
    for (int i = 0; i < N; i += 10) {
        scalar_result += complex_math_10_args(
            f1[i], f2[i], f3[i], f4[i], fout[i],
            d1[i], d2[i], d3[i], d4[i], d5[i]
        );
    }
    
    // Compute checksums
    float fsum = 0;
    double dsum = 0;
    int64_t isum = 0;
    
    for (int i = 0; i < N; i++) {
        fsum += fout[i];
        dsum += dout[i];
        isum += iout[i];
    }
    
    printf("Checksums:\n");
    printf("Float output: %f\n", fsum);
    printf("Double output: %lf\n", dsum);
    printf("Int output: %ld\n", isum);
    printf("Scalar result: %f\n", scalar_result);
    
    // Cleanup
    free(f1); free(f2); free(f3); free(f4); free(fout);
    free(d1); free(d2); free(d3); free(d4); free(d5); free(dout);
    free(i1); free(i2); free(i3); free(i4); free(iout);
    
    return 0;
}
