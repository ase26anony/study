#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for deterministic testing */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize arrays with pseudo-random data */
#define ARRAY_SIZE 1024
static float array_f32[ARRAY_SIZE];
static double array_f64[ARRAY_SIZE];
static int32_t array_i32[ARRAY_SIZE];
static int64_t array_i64[ARRAY_SIZE];
static float result_f32[ARRAY_SIZE];

/* Volatile counter to prevent loop unrolling */
static volatile int volatile_counter = 0;

/* Target-specific function with many arguments */
__attribute__((target("avx2,avx512f"))) 
__attribute__((noinline))
static void test_many_args_avx512(float* __restrict out, 
                                  const float* __restrict in1,
                                  const float* __restrict in2,
                                  const double* __restrict in3,
                                  const int32_t* __restrict in4,
                                  int n) {
#ifdef __AVX512F__
    /* Complex expression with many temporaries */
    for (int i = 0; i < n; i += 16) {
        /* Load multiple vectors */
        __m512 v1 = _mm512_loadu_ps(&in1[i]);
        __m512 v2 = _mm512_loadu_ps(&in2[i]);
        __m512d v3 = _mm512_loadu_pd((const double*)&in3[i]);
        __m512i v4 = _mm512_loadu_si512((const __m512i*)&in4[i]);
        
        /* Create many intermediate values to force complex expansion */
        __m512 temp1 = _mm512_add_ps(v1, v2);
        __m512 temp2 = _mm512_sub_ps(v1, v2);
        __m512 temp3 = _mm512_mul_ps(temp1, temp2);
        
        /* Convert double to float with many arguments in expression */
        __m512 v3f = _mm512_castpd_ps(v3);
        __m512 temp4 = _mm512_fmadd_ps(temp3, v3f, _mm512_set1_ps(1.0f));
        
        /* Complex shuffle/permutation with many arguments */
        /* This creates a dependency chain with many operands */
        __m512 shuffled;
        
        /* Inline asm with 10-11 operands to trigger optab expansion */
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vpermps %2, %3, %0\n\t"
            "vblendmps %4, %5, %0\n\t"
            "vfmadd132ps %6, %7, %0\n\t"
            "vxorps %8, %9, %0\n\t"
            : "=v"(shuffled)
            : "v"(temp4), "v"(v4), "v"(temp1), 
              "v"(temp2), "v"(temp3), "v"(v1),
              "v"(v2), "v"(v3f), "v"(_mm512_set1_ps(0.5f))
            : "memory"
        );
        
        /* Another complex operation with many arguments */
        __mmask16 mask = _mm512_cmp_ps_mask(shuffled, _mm512_setzero_ps(), _CMP_GT_OQ);
        
        /* Blend with mask - potentially expands to many arguments */
        __m512 blended = _mm512_mask_blend_ps(mask, shuffled, temp4);
        
        /* Store result */
        _mm512_storeu_ps(&out[i], blended);
        
        /* Volatile counter to prevent optimization */
        volatile_counter++;
    }
#endif
}

/* Alternative using GCC vector builtins with many arguments */
#ifdef __ARM_NEON
#include <arm_neon.h>

typedef float32x4_t v4sf;
typedef int32x4_t v4si;

__attribute__((noinline))
static v4sf complex_shuffle_10_args(v4sf a, v4sf b, v4sf c, v4sf d,
                                    v4sf e, v4sf f, v4sf g, v4sf h,
                                    v4sf i, v4sf j) {
    /* Use __builtin_shuffle with complex pattern */
    /* This may expand to optab with many arguments */
    v4sf temp1 = __builtin_shuffle(a, b, (v4si){0, 1, 2, 3});
    v4sf temp2 = __builtin_shuffle(c, d, (v4si){4, 5, 6, 7});
    v4sf temp3 = __builtin_shuffle(e, f, (v4si){0, 2, 1, 3});
    v4sf temp4 = __builtin_shuffle(g, h, (v4si){3, 2, 1, 0});
    
    /* Complex expression with many operands */
    return __builtin_shuffle(temp1, temp2, 
            __builtin_shufflevector((v4si){0,1,2,3}, (v4si){4,5,6,7},
                                   0, 4, 1, 5, 2, 6, 3, 7));
}
#endif

/* Function with exactly 11 scalar arguments */
__attribute__((noinline))
static float complex_scalar_expr(float a, float b, float c, float d,
                                 float e, float f, float g, float h,
                                 float i, float j, float k) {
    /* Complex expression that might expand to optab with 11 args */
    return ((a * b) + (c * d) - (e * f)) / 
           ((g + h) * (i - j) + k) * 
           (a + b + c + d + e + f + g + h + i + j + k);
}

/* Multi-statement expression with many temporaries */
__attribute__((noinline))
static void process_block_many_temps(float* out, const float* in, int n) {
    for (int idx = 0; idx < n; idx += 8) {
        /* Load and create many temporaries */
        float t1 = in[idx] * 2.0f;
        float t2 = in[idx+1] / 3.0f;
        float t3 = in[idx+2] + 1.5f;
        float t4 = in[idx+3] - 0.5f;
        float t5 = in[idx+4] * in[idx+5];
        float t6 = in[idx+6] + in[idx+7];
        float t7 = t1 + t2;
        float t8 = t3 * t4;
        float t9 = t5 / t6;
        float t10 = t7 - t8;
        
        /* Expression with 10+ operands */
        out[idx] = t1 + t2 * t3 - t4 / t5 + t6 * t7 - t8 / t9 + t10;
        
        /* Prevent CSE with volatile asm */
        asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3));
    }
}

/* Main test function */
int main() {
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_f32[i] = (float)prng_next() / (float)UINT32_MAX;
        array_f64[i] = (double)prng_next() / (double)UINT32_MAX;
        array_i32[i] = (int32_t)prng_next();
        array_i64[i] = (int64_t)prng_next();
    }
    
    /* Clear result array */
    memset(result_f32, 0, sizeof(result_f32));
    
    /* Test with AVX512 if available */
#ifdef __AVX512F__
    printf("Testing with AVX512...\n");
    test_many_args_avx512(result_f32, array_f32, array_f32, 
                         array_f64, array_i32, ARRAY_SIZE);
#endif
    
    /* Test scalar expression with 11 arguments */
    printf("Testing scalar expression with 11 arguments...\n");
    for (int i = 0; i < ARRAY_SIZE - 11; i++) {
        result_f32[i] += complex_scalar_expr(
            array_f32[i], array_f32[i+1], array_f32[i+2], array_f32[i+3],
            array_f32[i+4], array_f32[i+5], array_f32[i+6], array_f32[i+7],
            array_f32[i+8], array_f32[i+9], array_f32[i+10]
        );
    }
    
    /* Test with many temporaries */
    printf("Testing with many temporaries...\n");
    process_block_many_temps(result_f32, array_f32, ARRAY_SIZE);
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += result_f32[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Volatile counter: %d\n", volatile_counter);
    
    return 0;
}
