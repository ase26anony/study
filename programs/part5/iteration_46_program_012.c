#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
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
static double result_f64[ARRAY_SIZE];

/* Function to inhibit constant propagation */
static inline void inhibit_opt(volatile void* ptr) {
    asm volatile("" : "+r"(ptr) : : "memory");
}

/* Target-specific function with many arguments */
__attribute__((target("avx2,avx512f"), noinline))
static void test_many_args_avx512(void) {
    /* Declare many vector variables to force register pressure */
    __m512 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    __m512i mask0, mask1, mask2, mask3;
    __mmask16 k0, k1, k2, k3;
    
    /* Initialize with volatile to prevent constant folding */
    volatile int imm0 = 1;
    volatile int imm1 = 2;
    volatile int imm2 = 3;
    volatile int imm3 = 4;
    volatile int imm4 = 5;
    volatile int imm5 = 6;
    volatile int imm6 = 7;
    volatile int imm7 = 8;
    volatile int imm8 = 9;
    volatile int imm9 = 10;
    
    inhibit_opt(&imm0);
    
    /* Prevent loop unrolling with volatile counter */
    volatile int i;
    for (i = 0; i < ARRAY_SIZE; i += 16) {
        /* Load multiple vectors from arrays */
        v0 = _mm512_loadu_ps(&array_f32[i]);
        v1 = _mm512_loadu_ps(&array_f32[i + 16]);
        v2 = _mm512_loadu_ps(&array_f32[i + 32]);
        v3 = _mm512_loadu_ps(&array_f32[i + 48]);
        v4 = _mm512_loadu_ps(&array_f32[i + 64]);
        v5 = _mm512_loadu_ps(&array_f32[i + 80]);
        
        /* Create mask vectors with immediate values */
        mask0 = _mm512_set1_epi32(imm0);
        mask1 = _mm512_set1_epi32(imm1);
        mask2 = _mm512_set1_epi32(imm2);
        mask3 = _mm512_set1_epi32(imm3);
        
        /* Create mask registers */
        k0 = (__mmask16)(imm0 & 0xFFFF);
        k1 = (__mmask16)(imm1 & 0xFFFF);
        k2 = (__mmask16)(imm2 & 0xFFFF);
        k3 = (__mmask16)(imm3 & 0xFFFF);
        
        /* METHOD 1: Complex inline asm with 11 operands */
        /* This should trigger the 11-argument case in optabs.cc */
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vblendmps %2, %3, %0{%4}\n\t"
            "vaddps %5, %0, %0\n\t"
            "vmulps %6, %0, %0\n\t"
            "vfmsubadd231ps %7, %8, %0\n\t"
            "vpermps %9, %10, %0"
            : "=v"(v6)
            : "v"(v0), "v"(v1), "v"(v2), "k"(k0),
              "v"(v3), "v"(v4), "v"(v5), "v"(mask0),
              "v"(mask1), "v"(mask2)
            : "memory"
        );
        
        /* METHOD 2: Chain of operations creating many temporaries */
        /* This creates complex expression trees that might be folded */
        v7 = _mm512_add_ps(v0, v1);
        v8 = _mm512_mul_ps(v2, v3);
        v9 = _mm512_fmadd_ps(v4, v5, v6);
        v10 = _mm512_fnmadd_ps(v7, v8, v9);
        
        /* Complex blend with many arguments */
        __m512 blended = _mm512_mask_blend_ps(k0, v0, v1);
        blended = _mm512_mask_blend_ps(k1, blended, v2);
        blended = _mm512_mask_blend_ps(k2, blended, v3);
        blended = _mm512_mask_blend_ps(k3, blended, v4);
        
        /* Store results */
        _mm512_storeu_ps(&result_f32[i], v10);
        _mm512_storeu_ps(&result_f32[i + 16], blended);
    }
}

/* ARM NEON version for completeness */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noinline, target("arch=armv8-a+simd")))
static void test_many_args_neon(void) {
    float32x4_t v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    uint32x4_t mask0, mask1, mask2, mask3;
    
    volatile int imm0 = 1;
    volatile int imm1 = 2;
    volatile int imm2 = 3;
    volatile int imm3 = 4;
    volatile int imm4 = 5;
    volatile int imm5 = 6;
    
    inhibit_opt(&imm0);
    
    volatile int i;
    for (i = 0; i < ARRAY_SIZE; i += 4) {
        /* Load vectors */
        v0 = vld1q_f32(&array_f32[i]);
        v1 = vld1q_f32(&array_f32[i + 4]);
        v2 = vld1q_f32(&array_f32[i + 8]);
        v3 = vld1q_f32(&array_f32[i + 12]);
        v4 = vld1q_f32(&array_f32[i + 16]);
        v5 = vld1q_f32(&array_f32[i + 20]);
        
        /* Create masks */
        mask0 = vdupq_n_u32(imm0);
        mask1 = vdupq_n_u32(imm1);
        mask2 = vdupq_n_u32(imm2);
        mask3 = vdupq_n_u32(imm3);
        
        /* Complex inline asm with 10 operands */
        /* This should trigger the 10-argument case in optabs.cc */
        asm volatile (
            "vld1.32 {%0}, [%1]\n\t"
            "vadd.f32 %0, %0, %2\n\t"
            "vmul.f32 %0, %0, %3\n\t"
            "vmla.f32 %0, %4, %5\n\t"
            "vmls.f32 %0, %6, %7\n\t"
            "vtbl.8 %0, {%8}, %9"
            : "=w"(v6)
            : "r"(&array_f32[i]), "w"(v1), "w"(v2),
              "w"(v3), "w"(v4), "w"(v5), "w"(mask0),
              "w"(mask1), "w"(mask2)
            : "memory"
        );
        
        /* Store result */
        vst1q_f32(&result_f32[i], v6);
    }
}
#endif

/* Generic version using GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

__attribute__((noinline))
static void test_many_args_generic(void) {
    /* Create many vector temporaries */
    v8sf v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v8si m0, m1, m2, m3, m4, m5;
    
    volatile int imm0 = 1;
    volatile int imm1 = 2;
    volatile int imm2 = 3;
    volatile int imm3 = 4;
    volatile int imm4 = 5;
    volatile int imm5 = 6;
    volatile int imm6 = 7;
    volatile int imm7 = 8;
    volatile int imm8 = 9;
    volatile int imm9 = 10;
    volatile int imm10 = 11;
    
    inhibit_opt(&imm0);
    
    /* Complex expression with many operations */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        /* Load data */
        memcpy(&v0, &array_f32[i], sizeof(v0));
        memcpy(&v1, &array_f32[i + 8], sizeof(v1));
        memcpy(&v2, &array_f32[i + 16], sizeof(v2));
        memcpy(&v3, &array_f32[i + 24], sizeof(v3));
        memcpy(&v4, &array_f32[i + 32], sizeof(v4));
        memcpy(&v5, &array_f32[i + 40], sizeof(v5));
        
        /* Create mask vectors */
        m0 = (v8si){imm0, imm1, imm2, imm3, imm4, imm5, imm6, imm7};
        m1 = (v8si){imm1, imm2, imm3, imm4, imm5, imm6, imm7, imm8};
        m2 = (v8si){imm2, imm3, imm4, imm5, imm6, imm7, imm8, imm9};
        m3 = (v8si){imm3, imm4, imm5, imm6, imm7, imm8, imm9, imm10};
        
        /* Complex shuffle operation using __builtin_shuffle */
        /* This builtin can take many arguments when expanded */
        v6 = __builtin_shuffle(v0, v1, (v8si){
            imm0, imm1, imm2, imm3, imm4, imm5, imm6, imm7
        });
        
        v7 = __builtin_shuffle(v2, v3, (v8si){
            imm1, imm2, imm3, imm4, imm5, imm6, imm7, imm8
        });
        
        /* Chain operations to create complex expression tree */
        v8 = v0 + v1 * v2 - v3 / v4;
        v9 = v5 * v6 + v7 - v8;
        v10 = __builtin_shuffle(v8, v9, m0);
        
        /* Final complex operation that might be folded */
        v8sf result = (v0 * v1) + (v2 * v3) - (v4 * v5) + 
                     (v6 * v7) - (v8 * v9) + (v10 * v0);
        
        /* Store with inline asm to prevent optimization */
        asm volatile (
            "vmovups %1, %0\n\t"
            : "=m"(result_f32[i])
            : "v"(result)
            : "memory"
        );
    }
}

/* Function using many integer arguments */
__attribute__((noinline))
static int64_t complex_integer_expr(
    int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
    int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
    int64_t k
) {
    /* This function has exactly 11 arguments */
    /* Complex expression that might be optimized into a single operation */
    return ((a * b) + (c * d) - (e * f) + (g * h) - (i * j)) * k;
}

/* Initialize data arrays */
static void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_f32[i] = (float)prng_next() / (float)UINT32_MAX;
        array_f64[i] = (double)prng_next() / (double)UINT32_MAX;
        array_i32[i] = (int32_t)prng_next();
        array_i64[i] = (int64_t)prng_next() | ((int64_t)prng_next() << 32);
    }
}

/* Compute checksum */
static double compute_checksum(void) {
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += result_f32[i];
    }
    return sum;
}

int main(void) {
    printf("Testing many-argument optab expansion...\n");
    
    /* Initialize with pseudo-random data */
    init_arrays();
    
    /* Call target-specific functions */
#ifdef __AVX512F__
    printf("Using AVX-512 path...\n");
    test_many_args_avx512();
#elif defined(__ARM_NEON)
    printf("Using NEON path...\n");
    test_many_args_neon();
#else
    printf("Using generic path...\n");
    test_many_args_generic();
#endif
    
    /* Also test the 11-argument integer function */
    volatile int64_t args[11];
    for (int i = 0; i < 11; i++) {
        args[i] = prng_next();
    }
    
    int64_t result = complex_integer_expr(
        args[0], args[1], args[2], args[3], args[4],
        args[5], args[6], args[7], args[8], args[9],
        args[10]
    );
    
    inhibit_opt(&result);
    
    /* Compute and print checksum */
    double checksum = compute_checksum();
    printf("Checksum: %f\n", checksum);
    printf("Integer result: %lld\n", (long long)result);
    
    return 0;
}
