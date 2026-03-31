#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t rand_u32(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random data */
#define ARRAY_SIZE 1024
static float array_f32[ARRAY_SIZE];
static double array_f64[ARRAY_SIZE];
static int32_t array_i32[ARRAY_SIZE];
static int64_t array_i64[ARRAY_SIZE];
static float result_f32[ARRAY_SIZE];
static double result_f64[ARRAY_SIZE];

/* Complex expression with many temporaries to force optab expansion */
__attribute__((noinline, target("avx2,avx512f")))
void test_many_args_avx512(void) {
    volatile int i; /* Prevent loop unrolling */
    
    for (i = 0; i < ARRAY_SIZE; i += 8) {
        /* Load multiple vectors - creates many intermediate values */
        __m512 v0 = _mm512_loadu_ps(&array_f32[i]);
        __m512 v1 = _mm512_loadu_ps(&array_f32[(i + 8) % ARRAY_SIZE]);
        __m512 v2 = _mm512_loadu_ps(&array_f32[(i + 16) % ARRAY_SIZE]);
        __m512 v3 = _mm512_loadu_ps(&array_f32[(i + 24) % ARRAY_SIZE]);
        __m512 v4 = _mm512_loadu_ps(&array_f32[(i + 32) % ARRAY_SIZE]);
        
        /* Complex multi-statement expression with many temporaries */
        __m512 t0 = _mm512_add_ps(v0, v1);
        __m512 t1 = _mm512_sub_ps(v2, v3);
        __m512 t2 = _mm512_mul_ps(t0, t1);
        __m512 t3 = _mm512_fmadd_ps(v4, t2, v0);
        
        /* Create a shuffle mask with many immediate arguments */
        /* This could potentially trigger the 10-argument case */
        __m512i shuffle_mask = _mm512_setr_epi32(
            0, 8, 1, 9, 2, 10, 3, 11,
            4, 12, 5, 13, 6, 14, 7, 15
        );
        
        /* Complex permutation with many arguments */
        __m512 permuted = _mm512_permutexvar_ps(shuffle_mask, t3);
        
        /* Extended inline asm with 10-11 operands */
        /* This directly targets the uncovered lines */
        __m512 final_result;
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vfmadd132ps %4, %5, %0\n\t"
            "vpermps %6, %7, %0\n\t"
            : "=v"(final_result)
            : "v"(v0), "v"(v1), "v"(v2), "v"(v3), "v"(v4),
              "v"(permuted), "v"(_mm512_castsi512_ps(shuffle_mask))
            : "memory"
        );
        
        _mm512_storeu_ps(&result_f32[i], final_result);
    }
}

/* Alternative approach using GCC vector builtins with many arguments */
#ifdef __GNUC__
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

__attribute__((noinline, target("avx2")))
void test_many_args_vector_builtin(void) {
    volatile int i;
    
    for (i = 0; i < ARRAY_SIZE; i += 8) {
        /* Load vectors */
        v8sf a = *(v8sf*)&array_f32[i];
        v8sf b = *(v8sf*)&array_f32[(i + 8) % ARRAY_SIZE];
        v8sf c = *(v8sf*)&array_f32[(i + 16) % ARRAY_SIZE];
        v8sf d = *(v8sf*)&array_f32[(i + 24) % ARRAY_SIZE];
        v8sf e = *(v8sf*)&array_f32[(i + 32) % ARRAY_SIZE];
        
        /* Complex expression tree */
        v8sf t0 = a + b;
        v8sf t1 = c - d;
        v8sf t2 = t0 * t1;
        v8sf t3 = e * t2 + a;
        
        /* Use __builtin_shuffle with many arguments */
        /* This is a prime candidate for triggering the 10-argument case */
        v8sf shuffled = __builtin_shuffle(t3, t2, 
            (v8si){0, 8, 1, 9, 2, 10, 3, 11});
        
        /* Another shuffle with different pattern - more arguments */
        v8sf shuffled2 = __builtin_shuffle(shuffled, t1, t0,
            (v8si){4, 12, 5, 13, 6, 14, 7, 15});
        
        /* Extended asm with exactly 11 arguments */
        v8sf final;
        asm volatile (
            "vaddps %1, %2, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vblendvps %4, %5, %0, %0\n\t"
            "vpermilps $0x1B, %0, %0\n\t"
            "vfmadd213ps %6, %7, %0\n\t"
            : "=v"(final)
            : "v"(a), "v"(b), "v"(c), "v"(d), "v"(e),
              "v"(shuffled), "v"(shuffled2),
              "m"(array_f32[i]), "m"(array_f32[(i + 8) % ARRAY_SIZE]),
              "i"(0x1B)  /* Immediate constant */
            : "memory"
        );
        
        *(v8sf*)&result_f32[i] = final;
    }
}
#endif

/* ARM NEON version for completeness */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noinline))
void test_many_args_neon(void) {
    volatile int i;
    
    for (i = 0; i < ARRAY_SIZE; i += 4) {
        float32x4_t v0 = vld1q_f32(&array_f32[i]);
        float32x4_t v1 = vld1q_f32(&array_f32[(i + 4) % ARRAY_SIZE]);
        float32x4_t v2 = vld1q_f32(&array_f32[(i + 8) % ARRAY_SIZE]);
        float32x4_t v3 = vld1q_f32(&array_f32[(i + 12) % ARRAY_SIZE]);
        float32x4_t v4 = vld1q_f32(&array_f32[(i + 16) % ARRAY_SIZE]);
        
        /* Complex NEON operations */
        float32x4_t t0 = vaddq_f32(v0, v1);
        float32x4_t t1 = vsubq_f32(v2, v3);
        float32x4_t t2 = vmulq_f32(t0, t1);
        float32x4_t t3 = vmlaq_f32(v4, t2, v0);
        
        /* Lane selection with many arguments */
        float32x4_t lane_selected = {
            vgetq_lane_f32(t3, 0),
            vgetq_lane_f32(t2, 1),
            vgetq_lane_f32(t1, 2),
            vgetq_lane_f32(t0, 3)
        };
        
        /* Extended asm with many operands */
        float32x4_t result;
        asm volatile (
            "fadd %0.4s, %1.4s, %2.4s\n\t"
            "fmul %0.4s, %0.4s, %3.4s\n\t"
            "fmla %0.4s, %4.4s, %5.4s\n\t"
            "tbl %0.16b, {%6.16b}, %7.16b\n\t"
            : "=w"(result)
            : "w"(v0), "w"(v1), "w"(v2), "w"(v3), "w"(v4),
              "w"(lane_selected), "w"(t3),
              "m"(array_f32[i]), "m"(array_f32[(i + 4) % ARRAY_SIZE]),
              "I"(0)  /* Immediate */
            : "memory"
        );
        
        vst1q_f32(&result_f32[i], result);
    }
}
#endif

/* Function with complex pointer arithmetic and many temporaries */
__attribute__((noinline))
void test_many_args_scalar(void) {
    volatile int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        /* Complex expression with many intermediate values */
        /* This forces the compiler to generate many temporaries */
        float a = array_f32[i];
        float b = array_f32[(i + 1) % ARRAY_SIZE];
        float c = array_f32[(i + 2) % ARRAY_SIZE];
        float d = array_f32[(i + 3) % ARRAY_SIZE];
        float e = array_f32[(i + 4) % ARRAY_SIZE];
        float f = array_f32[(i + 5) % ARRAY_SIZE];
        float g = array_f32[(i + 6) % ARRAY_SIZE];
        float h = array_f32[(i + 7) % ARRAY_SIZE];
        float j = array_f32[(i + 8) % ARRAY_SIZE];
        float k = array_f32[(i + 9) % ARRAY_SIZE];
        
        /* Very complex expression tree */
        float t0 = a + b;
        float t1 = c - d;
        float t2 = e * f;
        float t3 = g / h;
        float t4 = t0 * t1;
        float t5 = t2 + t3;
        float t6 = t4 - t5;
        float t7 = j * k;
        float t8 = t6 + t7;
        
        /* Extended asm with 10 scalar arguments */
        float result;
        asm volatile (
            "fadds %0, %1, %2\n\t"
            "fmuls %0, %0, %3\n\t"
            "fsubs %0, %0, %4\n\t"
            "fdivs %0, %0, %5\n\t"
            "fmadds %0, %6, %7, %0\n\t"
            : "=f"(result)
            : "f"(t0), "f"(t1), "f"(t2), "f"(t3), "f"(t4),
              "f"(t5), "f"(t6), "f"(t7), "f"(t8),
              "m"(array_f32[i])  /* Memory operand */
            : "memory"
        );
        
        result_f32[i] = result;
    }
}

/* Initialize data arrays */
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_f32[i] = (float)rand_u32() / (float)UINT32_MAX;
        array_f64[i] = (double)rand_u32() / (double)UINT32_MAX;
        array_i32[i] = (int32_t)rand_u32();
        array_i64[i] = (int64_t)rand_u32() | ((int64_t)rand_u32() << 32);
    }
}

/* Compute checksum for validation */
float compute_checksum(float* arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(void) {
    init_arrays();
    
    printf("Testing many-argument operations...\n");
    
    /* Test AVX512 version if supported */
#ifdef __AVX512F__
    printf("Running AVX512 test...\n");
    test_many_args_avx512();
#endif
    
    /* Test GCC vector builtin version */
#ifdef __GNUC__
    printf("Running GCC vector builtin test...\n");
    test_many_args_vector_builtin();
#endif
    
    /* Test scalar version */
    printf("Running scalar test...\n");
    test_many_args_scalar();
    
    /* Compute and print checksum */
    float checksum = compute_checksum(result_f32, ARRAY_SIZE);
    printf("Result checksum: %f\n", checksum);
    
    return 0;
}
