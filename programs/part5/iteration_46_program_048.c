#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Volatile counter to prevent loop unrolling */
static volatile int volatile_counter = 0;

/* Target-specific function with many arguments */
__attribute__((noinline, target("avx2,avx512f")))
void test_many_args(float* restrict out, 
                    const float* restrict in1,
                    const float* restrict in2,
                    const float* restrict in3,
                    const float* restrict in4,
                    int n) {
    /* Prevent constant propagation */
    int limit = n + volatile_counter;
    
    for (int i = 0; i < limit; i += 8) {
        /* Load multiple vectors */
        __m256 v1 = _mm256_loadu_ps(&in1[i]);
        __m256 v2 = _mm256_loadu_ps(&in2[i]);
        __m256 v3 = _mm256_loadu_ps(&in3[i]);
        __m256 v4 = _mm256_loadu_ps(&in4[i]);
        
        /* Create complex shuffle mask using multiple operations */
        __m256i mask1 = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
        __m256i mask2 = _mm256_set_epi32(15, 14, 13, 12, 11, 10, 9, 8);
        
        /* METHOD 1: Complex builtin with many arguments */
        /* This may expand to optab with many operands */
        __m256 result;
        
        /* Use inline asm with 11 operands to potentially trigger case 11 */
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vblendvps %2, %3, %0, %0\n\t"
            "vpermps %4, %5, %6\n\t"
            "vfmadd213ps %7, %8, %9\n\t"
            : "=x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4), 
              "x"(_mm256_castsi256_ps(mask1)),
              "x"(_mm256_castsi256_ps(mask2)),
              "m"(in1[i]), "m"(in2[i]), "m"(in3[i]),
              "i"(256)  /* 11th operand - immediate */
            : "memory"
        );
        
        /* METHOD 2: Chain of operations creating many temporaries */
        /* This creates a complex expression tree */
        __m256 temp1 = _mm256_add_ps(v1, v2);
        __m256 temp2 = _mm256_mul_ps(v3, v4);
        __m256 temp3 = _mm256_sub_ps(temp1, temp2);
        __m256 temp4 = _mm256_fmadd_ps(v1, v2, v3);
        __m256 temp5 = _mm256_fnmadd_ps(v4, temp1, temp2);
        
        /* Complex shuffle with many lane indices */
        /* This could expand to a 10-argument optab call */
        __m256 shuffled = _mm256_permutevar8x32_ps(
            temp3,
            _mm256_set_epi32(7, 0, 1, 2, 3, 4, 5, 6)
        );
        
        /* Blend with multiple control masks */
        __m256 blended = _mm256_blend_ps(shuffled, temp4, 0xAA);
        blended = _mm256_blend_ps(blended, temp5, 0x55);
        
        /* Final combination */
        __m256 final_result = _mm256_add_ps(result, blended);
        
        /* Store result */
        _mm256_storeu_ps(&out[i], final_result);
    }
}

/* Alternative function using ARM NEON style if compiled for ARM */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noinline))
void test_many_args_neon(float32_t* restrict out,
                         const float32_t* restrict in1,
                         const float32_t* restrict in2,
                         const float32_t* restrict in3,
                         const float32_t* restrict in4,
                         int n) {
    for (int i = 0; i < n; i += 4) {
        float32x4_t v1 = vld1q_f32(&in1[i]);
        float32x4_t v2 = vld1q_f32(&in2[i]);
        float32x4_t v3 = vld1q_f32(&in3[i]);
        float32x4_t v4 = vld1q_f32(&in4[i]);
        
        /* Complex sequence with many arguments */
        float32x4_t temp1 = vaddq_f32(v1, v2);
        float32x4_t temp2 = vmulq_f32(v3, v4);
        float32x4_t temp3 = vsubq_f32(temp1, temp2);
        
        /* Shuffle with many lane indices */
        const int lane0 = 0, lane1 = 1, lane2 = 2, lane3 = 3;
        float32x4_t shuffled = vsetq_lane_f32(vgetq_lane_f32(temp3, lane3), temp3, lane0);
        shuffled = vsetq_lane_f32(vgetq_lane_f32(temp3, lane2), shuffled, lane1);
        shuffled = vsetq_lane_f32(vgetq_lane_f32(temp3, lane1), shuffled, lane2);
        shuffled = vsetq_lane_f32(vgetq_lane_f32(temp3, lane0), shuffled, lane3);
        
        /* Inline asm with many operands for ARM */
        asm volatile (
            "vadd.f32 %0, %1, %2\n\t"
            "vmla.f32 %0, %3, %4\n\t"
            "vrev64.32 %5, %6\n\t"
            "vext.8 %7, %8, %9, #4\n\t"
            : "=w"(shuffled)
            : "w"(v1), "w"(v2), "w"(v3), "w"(v4),
              "w"(temp1), "w"(temp2), "w"(temp3),
              "w"(shuffled), "I"(8)  /* 10 operands total */
            : 
        );
        
        vst1q_f32(&out[i], shuffled);
    }
}
#endif

/* Function with complex integer operations creating many temporaries */
__attribute__((noinline))
void test_many_int_args(uint64_t* restrict out,
                        const uint32_t* restrict in1,
                        const uint16_t* restrict in2,
                        const uint8_t* restrict in3,
                        int n) {
    /* Create complex expression with many intermediate values */
    for (int i = 0; i < n; ++i) {
        /* Many arithmetic operations creating complex expression tree */
        uint64_t a = in1[i];
        uint64_t b = in2[i];
        uint64_t c = in3[i];
        
        /* Chain of operations - each creates a temporary */
        uint64_t t1 = a + b;
        uint64_t t2 = a * c;
        uint64_t t3 = b << 3;
        uint64_t t4 = c >> 2;
        uint64_t t5 = t1 ^ t2;
        uint64_t t6 = t3 | t4;
        uint64_t t7 = t5 & t6;
        uint64_t t8 = t7 + a;
        uint64_t t9 = t8 - b;
        uint64_t t10 = t9 * c;
        
        /* Final complex expression with 10+ terms */
        out[i] = ((a * b) + (c << 4)) ^ 
                 ((t1 & t2) | (t3 ^ t4)) +
                 ((t5 - t6) * (t7 / 8)) &
                 ((t8 | t9) ^ (t10 << 1));
    }
}

int main(void) {
    const int N = 1024;
    
    /* Allocate and initialize arrays */
    float* in1 = aligned_alloc(32, N * sizeof(float));
    float* in2 = aligned_alloc(32, N * sizeof(float));
    float* in3 = aligned_alloc(32, N * sizeof(float));
    float* in4 = aligned_alloc(32, N * sizeof(float));
    float* out = aligned_alloc(32, N * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; ++i) {
        in1[i] = (float)prng_next() / (float)UINT32_MAX;
        in2[i] = (float)prng_next() / (float)UINT32_MAX;
        in3[i] = (float)prng_next() / (float)UINT32_MAX;
        in4[i] = (float)prng_next() / (float)UINT32_MAX;
    }
    
    /* Call the target function */
    test_many_args(out, in1, in2, in3, in4, N);
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < N; ++i) {
        checksum += out[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Test integer version */
    uint32_t* int_in1 = malloc(N * sizeof(uint32_t));
    uint16_t* int_in2 = malloc(N * sizeof(uint16_t));
    uint8_t* int_in3 = malloc(N * sizeof(uint8_t));
    uint64_t* int_out = malloc(N * sizeof(uint64_t));
    
    for (int i = 0; i < N; ++i) {
        int_in1[i] = prng_next();
        int_in2[i] = prng_next() & 0xFFFF;
        int_in3[i] = prng_next() & 0xFF;
    }
    
    test_many_int_args(int_out, int_in1, int_in2, int_in3, N);
    
    /* Compute integer checksum */
    uint64_t int_checksum = 0;
    for (int i = 0; i < N; ++i) {
        int_checksum += int_out[i];
    }
    
    printf("Integer checksum: %lu\n", int_checksum);
    
    /* Cleanup */
    free(in1);
    free(in2);
    free(in3);
    free(in4);
    free(out);
    free(int_in1);
    free(int_in2);
    free(int_in3);
    free(int_out);
    
    return 0;
}
