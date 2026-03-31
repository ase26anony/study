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

/* Initialize arrays with pseudo-random data */
static void init_arrays(float* arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (float)(prng_next() & 0xFFFF) / 65536.0f;
    }
}

/* Complex expression with many temporaries - forces expander to handle many operands */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(float* __restrict out, 
                          const float* __restrict in1,
                          const float* __restrict in2,
                          const float* __restrict in3,
                          const float* __restrict in4,
                          int n) {
    /* Volatile counter to prevent loop unrolling */
    volatile int volatile_n = n;
    
    for (int i = 0; i < volatile_n; i += 16) {
        /* Load multiple vectors - creates many SSA values */
        __m512 v1 = _mm512_loadu_ps(&in1[i]);
        __m512 v2 = _mm512_loadu_ps(&in1[i + 16]);
        __m512 v3 = _mm512_loadu_ps(&in2[i]);
        __m512 v4 = _mm512_loadu_ps(&in2[i + 16]);
        __m512 v5 = _mm512_loadu_ps(&in3[i]);
        __m512 v6 = _mm512_loadu_ps(&in3[i + 16]);
        __m512 v7 = _mm512_loadu_ps(&in4[i]);
        __m512 v8 = _mm512_loadu_ps(&in4[i + 16]);
        
        /* Complex multi-statement expression with many temporaries */
        __m512 t1 = _mm512_add_ps(v1, v2);
        __m512 t2 = _mm512_sub_ps(v3, v4);
        __m512 t3 = _mm512_mul_ps(v5, v6);
        __m512 t4 = _mm512_div_ps(v7, v8);
        
        /* Create a shuffle mask with many immediate values */
        __m512i mask = _mm512_setr_epi32(
            0, 16, 1, 17, 2, 18, 3, 19,
            4, 20, 5, 21, 6, 22, 7, 23
        );
        
        /* Extended inline asm with 11 operands - triggers optab expansion */
        __m512 result;
        asm volatile (
            "vmovaps %[v1], %%zmm0\n\t"
            "vmovaps %[v2], %%zmm1\n\t"
            "vmovaps %[v3], %%zmm2\n\t"
            "vmovaps %[v4], %%zmm3\n\t"
            "vmovaps %[mask], %%zmm4\n\t"
            "vpermps %%zmm4, %%zmm0, %%zmm5\n\t"
            "vpermps %%zmm4, %%zmm1, %%zmm6\n\t"
            "vaddps %%zmm5, %%zmm6, %%zmm7\n\t"
            "vaddps %%zmm7, %%zmm2, %%zmm8\n\t"
            "vaddps %%zmm8, %%zmm3, %%zmm9\n\t"
            "vmovaps %%zmm9, %[result]"
            : [result] "=v" (result)
            : [v1] "v" (t1), [v2] "v" (t2), [v3] "v" (t3), [v4] "v" (t4),
              [mask] "v" (mask),
              "m" (*(const float(*)[16])&in1[i]),  /* memory constraint */
              "m" (*(const float(*)[16])&in2[i]),
              "m" (*(const float(*)[16])&in3[i]),
              "m" (*(const float(*)[16])&in4[i]),
              "i" (16)  /* immediate operand */
            : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", 
              "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "memory"
        );
        
        /* Store result */
        _mm512_storeu_ps(&out[i], result);
    }
}

/* Alternative approach using GCC vector builtins with many arguments */
#ifdef __GNUC__
typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noinline, target("avx512f")))
static v16sf complex_shuffle_10_args(v16sf a, v16sf b, v16sf c, v16sf d,
                                     v16sf e, v16sf f, v16sf g, v16sf h,
                                     v16sf i, v16sf j) {
    /* Use __builtin_shuffle with many arguments - may trigger 10-arg optab */
    v16sf t1 = __builtin_shuffle(a, b, (v16sf){
        0, 16, 1, 17, 2, 18, 3, 19,
        4, 20, 5, 21, 6, 22, 7, 23
    });
    
    v16sf t2 = __builtin_shuffle(c, d, (v16sf){
        8, 24, 9, 25, 10, 26, 11, 27,
        12, 28, 13, 29, 14, 30, 15, 31
    });
    
    /* Complex expression with many operands */
    v16sf result = t1 + t2 + e + f + g + h + i + j;
    
    /* Prevent CSE and constant propagation */
    asm volatile("" : "+v"(result));
    
    return result;
}

__attribute__((noinline, target("avx512f")))
static v16sf complex_shuffle_11_args(v16sf a, v16sf b, v16sf c, v16sf d,
                                     v16sf e, v16sf f, v16sf g, v16sf h,
                                     v16sf i, v16sf j, v16sf k) {
    /* Even more complex with 11 arguments */
    v16sf mask1 = (v16sf){
        0, 16, 2, 18, 4, 20, 6, 22,
        8, 24, 10, 26, 12, 28, 14, 30
    };
    
    v16sf mask2 = (v16sf){
        1, 17, 3, 19, 5, 21, 7, 23,
        9, 25, 11, 27, 13, 29, 15, 31
    };
    
    v16sf t1 = __builtin_shuffle(a, b, mask1);
    v16sf t2 = __builtin_shuffle(c, d, mask2);
    
    /* Chain of operations with all 11 arguments */
    v16sf result = (t1 * t2) + (e - f) * (g / h) + (i & j) | k;
    
    /* Inhibit optimization */
    asm volatile("" : "+v"(result));
    
    return result;
}
#endif

/* ARM NEON version for portability */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noinline))
static float32x4_t neon_complex_operation(float32x4_t a, float32x4_t b,
                                         float32x4_t c, float32x4_t d,
                                         float32x4_t e, float32x4_t f,
                                         float32x4_t g, float32x4_t h,
                                         float32x4_t i, float32x4_t j) {
    /* Complex NEON operation with many arguments */
    float32x4_t t1 = vaddq_f32(a, b);
    float32x4_t t2 = vsubq_f32(c, d);
    float32x4_t t3 = vmulq_f32(e, f);
    float32x4_t t4 = vdivq_f32(g, h);
    
    /* Extended inline asm with 10 operands */
    float32x4_t result;
    asm volatile (
        "vadd.f32 %q[res], %q[t1], %q[t2]\n\t"
        "vmla.f32 %q[res], %q[t3], %q[t4]\n\t"
        "vadd.f32 %q[res], %q[res], %q[i]\n\t"
        "vadd.f32 %q[res], %q[res], %q[j]"
        : [res] "=w" (result)
        : [t1] "w" (t1), [t2] "w" (t2), [t3] "w" (t3), [t4] "w" (t4),
          [i] "w" (i), [j] "w" (j),
          "m" (*(const float*)&a), "m" (*(const float*)&b),
          "m" (*(const float*)&c), "m" (*(const float*)&d)
        : "memory"
    );
    
    return result;
}
#endif

/* Integer version with many bitwise operations */
__attribute__((noinline))
static uint64_t integer_complex_expr(uint8_t a, uint16_t b, uint32_t c,
                                     uint64_t d, int8_t e, int16_t f,
                                     int32_t g, int64_t h, char i, short j) {
    /* Complex expression with many type conversions and operations */
    uint64_t t1 = (uint64_t)a * (uint64_t)b;
    uint64_t t2 = (uint64_t)c ^ (uint64_t)d;
    uint64_t t3 = (uint64_t)(e * f) + (uint64_t)(g * h);
    uint64_t t4 = (uint64_t)i << (j & 0x3F);
    
    /* Multi-statement with many temporaries */
    uint64_t result = t1 + t2;
    result = result * t3;
    result = result | t4;
    result = result ^ (t1 << 3);
    result = result + (t2 >> 2);
    result = result * (t3 & 0xFFFFFFFF);
    result = result | (t4 ^ 0xAAAAAAAA);
    
    /* Extended inline asm with 10 integer operands */
    asm volatile (
        "add %[res], %[res], %[t1]\n\t"
        "imul %[res], %[res], %[t2]\n\t"
        "or %[res], %[res], %[t3]\n\t"
        "xor %[res], %[res], %[t4]"
        : [res] "+r" (result)
        : [t1] "r" (t1), [t2] "r" (t2), [t3] "r" (t3), [t4] "r" (t4),
          "m" (*(const uint8_t*)&a), "m" (*(const uint16_t*)&b),
          "m" (*(const uint32_t*)&c), "m" (*(const uint64_t*)&d),
          "i" (10), "i" (20)
        : "cc", "memory"
    );
    
    return result;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t N = ARRAY_SIZE * 4;  /* For float arrays */
    
    /* Allocate and initialize arrays */
    float* in1 = (float*)aligned_alloc(64, N * sizeof(float));
    float* in2 = (float*)aligned_alloc(64, N * sizeof(float));
    float* in3 = (float*)aligned_alloc(64, N * sizeof(float));
    float* in4 = (float*)aligned_alloc(64, N * sizeof(float));
    float* out = (float*)aligned_alloc(64, N * sizeof(float));
    
    if (!in1 || !in2 || !in3 || !in4 || !out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(in1, N);
    init_arrays(in2, N);
    init_arrays(in3, N);
    init_arrays(in4, N);
    memset(out, 0, N * sizeof(float));
    
    /* Test the many-argument function */
    test_many_args(out, in1, in2, in3, in4, N / 16);
    
    /* Compute checksum */
    double checksum = 0.0;
    for (size_t i = 0; i < N; i++) {
        checksum += (double)out[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Test integer version */
    uint64_t int_result = integer_complex_expr(
        1, 2, 3, 4, -1, -2, -3, -4, 'a', 5
    );
    printf("Integer result: %lu\n", int_result);
    
    /* Clean up */
    free(in1);
    free(in2);
    free(in3);
    free(in4);
    free(out);
    
    return 0;
}
