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
#define ARRAY_SIZE 1024
static float array_f32[ARRAY_SIZE];
static double array_f64[ARRAY_SIZE];
static int32_t array_i32[ARRAY_SIZE];
static int64_t array_i64[ARRAY_SIZE];
static float output_f32[ARRAY_SIZE];
static double output_f64[ARRAY_SIZE];

// Volatile counter to prevent loop unrolling
volatile int volatile_counter = 0;

// Function with many arguments that should trigger optab expansion
__attribute__((noinline, target("avx2,avx512f")))
void test_many_args_avx512() {
    // Force inline assembly with 11 operands
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        volatile_counter++;
        
        // Load multiple vectors
        __m512 v0 = _mm512_loadu_ps(&array_f32[i]);
        __m512 v1 = _mm512_loadu_ps(&array_f32[i + 8]);
        __m512 v2 = _mm512_loadu_ps(&array_f32[i + 16]);
        __m512 v3 = _mm512_loadu_ps(&array_f32[i + 24]);
        __m512 v4 = _mm512_loadu_ps(&array_f32[i + 32]);
        
        // Complex shuffle with many arguments - this may expand to optab with 10+ args
        __m512 shuffled = _mm512_shuffle_ps(v0, v1, _MM_SHUFFLE(3, 2, 1, 0));
        shuffled = _mm512_shuffle_ps(shuffled, v2, _MM_SHUFFLE(1, 0, 3, 2));
        
        // Extended inline asm with 11 operands
        __m512 result;
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vsubps %4, %0, %0\n\t"
            "vdivps %5, %0, %0\n\t"
            "vfmadd132ps %6, %7, %0\n\t"
            "vfnmadd132ps %8, %9, %0\n\t"
            : "=v"(result)
            : "v"(v0), "v"(v1), "v"(v2), "v"(v3), "v"(v4),
              "v"(shuffled), "v"(_mm512_set1_ps(2.0f)),
              "v"(_mm512_set1_ps(3.0f)), "v"(_mm512_set1_ps(4.0f)),
              "m"(array_f32[i])
            : "memory"
        );
        
        _mm512_storeu_ps(&output_f32[i], result);
    }
}

// Alternative function using GCC vector builtins with many arguments
__attribute__((noinline, target("avx2")))
void test_many_args_builtin() {
    typedef float v8sf __attribute__((vector_size(32)));
    typedef int v8si __attribute__((vector_size(32)));
    
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        volatile_counter++;
        
        // Load vectors
        v8sf a = *(v8sf*)&array_f32[i];
        v8sf b = *(v8sf*)&array_f32[i + 8];
        v8sf c = *(v8sf*)&array_f32[i + 16];
        v8sf d = *(v8sf*)&array_f32[i + 24];
        v8sf e = *(v8sf*)&array_f32[i + 32];
        
        // Create complex expression with many temporaries
        v8sf t1 = a + b;
        v8sf t2 = c * d;
        v8sf t3 = t1 - t2;
        v8sf t4 = e / t3;
        v8sf t5 = t4 * a;
        v8sf t6 = t5 + b;
        v8sf t7 = t6 - c;
        v8sf t8 = t7 * d;
        v8sf t9 = t8 + e;
        v8sf t10 = t9 - a;
        
        // Use __builtin_shuffle with many arguments
        // Note: Actual shuffle pattern would need to be constructed
        v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};
        v8sf shuffled = __builtin_shuffle(t1, t2, mask);
        
        // Complex inline asm with 10 operands
        v8sf result;
        asm volatile (
            "vaddps %1, %2, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vsubps %4, %0, %0\n\t"
            "vdivps %5, %0, %0\n\t"
            "vaddps %6, %0, %0\n\t"
            "vmulps %7, %0, %0\n\t"
            : "=v"(result)
            : "v"(t1), "v"(t2), "v"(t3), "v"(t4),
              "v"(t5), "v"(t6), "v"(t7), "m"(array_i32[i]),
              "m"(array_i32[i + 8])
            : "memory"
        );
        
        *(v8sf*)&output_f32[i] = result;
    }
}

// Function using mixed types and many arguments
__attribute__((noinline))
void test_mixed_types_many_args() {
    // Complex expression with many intermediate values
    for (int i = 0; i < ARRAY_SIZE - 16; i++) {
        volatile_counter++;
        
        // Use different types to force conversions
        char c1 = (char)array_i32[i] & 0xFF;
        short s1 = (short)array_i32[i + 1];
        int i1 = array_i32[i + 2];
        long l1 = array_i64[i + 3];
        float f1 = array_f32[i + 4];
        double d1 = array_f64[i + 5];
        
        // Build complex expression with many operations
        double result = (double)c1 * 1.5 +
                       (double)s1 * 2.5 +
                       (double)i1 * 3.5 +
                       (double)l1 * 4.5 +
                       f1 * 5.5 +
                       d1 * 6.5 +
                       (double)(c1 & s1) * 7.5 +
                       (double)(i1 | l1) * 8.5 +
                       (double)(c1 ^ s1) * 9.5 +
                       (double)(i1 << 2) * 10.5;
        
        // Inline asm with 11 memory operands
        asm volatile (
            "add %1, %0\n\t"
            "sub %2, %0\n\t"
            "mul %3, %0\n\t"
            "div %4, %0\n\t"
            "add %5, %0\n\t"
            "sub %6, %0\n\t"
            "mul %7, %0\n\t"
            "div %8, %0\n\t"
            "add %9, %0\n\t"
            "sub %10, %0\n\t"
            : "+r"(result)
            : "m"(array_i32[i]), "m"(array_i32[i + 1]),
              "m"(array_i32[i + 2]), "m"(array_i32[i + 3]),
              "m"(array_i32[i + 4]), "m"(array_i32[i + 5]),
              "m"(array_i32[i + 6]), "m"(array_i32[i + 7]),
              "m"(array_i32[i + 8]), "m"(array_i32[i + 9])
            : "memory"
        );
        
        output_f64[i] = result;
    }
}

// ARM NEON version for completeness
#ifdef __ARM_NEON
#include <arm_neon.h>
__attribute__((noinline))
void test_many_args_neon() {
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        volatile_counter++;
        
        float32x4_t v0 = vld1q_f32(&array_f32[i]);
        float32x4_t v1 = vld1q_f32(&array_f32[i + 4]);
        float32x4_t v2 = vld1q_f32(&array_f32[i + 8]);
        float32x4_t v3 = vld1q_f32(&array_f32[i + 12]);
        float32x4_t v4 = vld1q_f32(&array_f32[i + 16]);
        
        // Complex NEON operations
        float32x4_t t0 = vaddq_f32(v0, v1);
        float32x4_t t1 = vmulq_f32(v2, v3);
        float32x4_t t2 = vsubq_f32(t0, t1);
        float32x4_t t3 = vdivq_f32(v4, t2);
        
        // Extended inline asm with many operands
        float32x4_t result;
        asm volatile (
            "vadd.f32 %0, %1, %2\n\t"
            "vmla.f32 %0, %3, %4\n\t"
            "vmls.f32 %0, %5, %6\n\t"
            "vadd.f32 %0, %0, %7\n\t"
            "vsub.f32 %0, %0, %8\n\t"
            : "=w"(result)
            : "w"(t0), "w"(t1), "w"(t2), "w"(t3),
              "w"(v0), "w"(v1), "w"(v2), "w"(v3),
              "m"(array_f32[i]), "m"(array_f32[i + 4])
            : "memory"
        );
        
        vst1q_f32(&output_f32[i], result);
    }
}
#endif

int main() {
    // Initialize arrays with pseudo-random data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_f32[i] = (float)prng_next() / (float)UINT32_MAX * 100.0f;
        array_f64[i] = (double)prng_next() / (double)UINT32_MAX * 100.0;
        array_i32[i] = (int32_t)prng_next();
        array_i64[i] = (int64_t)prng_next() | ((int64_t)prng_next() << 32);
    }
    
    // Clear output arrays
    memset(output_f32, 0, sizeof(output_f32));
    memset(output_f64, 0, sizeof(output_f64));
    
    printf("Testing many-argument optab expansion...\n");
    
    // Test AVX512 version if supported
#ifdef __AVX512F__
    printf("Running AVX512 test...\n");
    test_many_args_avx512();
#endif
    
    // Test AVX2 version
#ifdef __AVX2__
    printf("Running AVX2 builtin test...\n");
    test_many_args_builtin();
#endif
    
    // Test mixed types
    printf("Running mixed types test...\n");
    test_mixed_types_many_args();
    
    // Test NEON if available
#ifdef __ARM_NEON
    printf("Running NEON test...\n");
    test_many_args_neon();
#endif
    
    // Compute checksum
    double checksum_f32 = 0.0;
    double checksum_f64 = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum_f32 += output_f32[i];
        checksum_f64 += output_f64[i];
    }
    
    printf("Checksum float32: %f\n", checksum_f32);
    printf("Checksum float64: %f\n", checksum_f64);
    printf("Volatile iterations: %d\n", volatile_counter);
    
    return 0;
}
