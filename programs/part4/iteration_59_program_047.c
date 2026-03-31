/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Target-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

/* GCC vector extensions for generic testing */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex inline assembly with many operands */
static void test_many_operand_asm(void) {
    /* 11 operands: 5 outputs + 5 inputs + 1 clobber */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int out1, out2, out3, out4, out5;
    
    asm volatile (
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "sub %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "imul %2, %2, %10\n\t"
        "mov %3, %5\n\t"
        "and %3, %3, %7\n\t"
        "mov %4, %6\n\t"
        "or  %4, %4, %8\n\t"
        : "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4), "=r"(out5)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e)
        : "cc"
    );
    
    printf("ASM results: %d %d %d %d %d\n", out1, out2, out3, out4, out5);
}

#ifdef __x86_64__
/* AVX-512 intrinsics with many operands */
FORCE_INLINE __m512 test_avx512_many_operands(__m512 a, __m512 b, __m512 c, 
                                              __m512 d, __mmask16 k) {
    /* Complex expression that might expand to multiple 10-11 operand patterns */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);  /* mask + 3 vectors = 4 operands in RTL */
    __m512 t2 = _mm512_mask_mul_ps(t1, k, d, a);
    __m512 t3 = _mm512_mask_sub_ps(t2, k, t2, b);
    __m512 t4 = _mm512_mask_fmadd_ps(t3, k, c, d, a);  /* FMA with mask: potentially many operands */
    
    /* Nested operations to prevent optimization */
    __m512 result = _mm512_mask_add_ps(
        _mm512_mask_mul_ps(t4, k, t1, t2),
        k,
        _mm512_mask_sub_ps(a, k, b, c),
        _mm512_mask_div_ps(d, k, t3, t1)
    );
    
    return result;
}

/* Test fused multiply-add chains */
FORCE_INLINE float test_fma_chain(float a, float b, float c, float d, float e) {
    /* Chain of FMA operations - each __builtin_fma has 3 operands,
       but combined expressions might create patterns with more */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, t1);
    float t3 = __builtin_fma(a, c, t2);
    float t4 = __builtin_fma(b, d, t3);
    float t5 = __builtin_fma(c, e, t4);
    
    /* Complex expression forcing many temporaries */
    return __builtin_fma(
        __builtin_fma(a, b, __builtin_fma(c, d, e)),
        __builtin_fma(t1, t2, __builtin_fma(t3, t4, t5)),
        __builtin_fma(a, d, __builtin_fma(b, e, c))
    );
}
#endif

#ifdef __aarch64__
/* ARM NEON/SVE style many-operand operations */
FORCE_INLINE float32x4_t test_neon_many_operands(float32x4_t a, float32x4_t b,
                                                 float32x4_t c, float32x4_t d,
                                                 uint32x4_t mask) {
    /* Complex lane operations and permutations */
    float32x4_t t1 = vaddq_f32(a, b);
    float32x4_t t2 = vmulq_f32(c, d);
    
    /* Use lane operations that might expand to multiple operands */
    float32x4_t t3 = vfmaq_laneq_f32(t1, t2, a, 1);  /* FMA with lane selection */
    float32x4_t t4 = vfmaq_laneq_f32(t3, b, c, 2);
    
    /* Table lookup style operations (can have many operands) */
    uint8x16x4_t tbl_test = vld4q_u8((const uint8_t*)&a);
    
    /* Combine results */
    float32x4_t result = vaddq_f32(
        vmulq_f32(t4, vdupq_n_f32(2.0f)),
        vbslq_f32(mask, t1, t2)
    );
    
    return result;
}
#endif

/* GCC vector extensions with complex expressions */
FORCE_INLINE v8sf test_vector_extensions(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex expression that might generate many-operand patterns */
    v8sf t1 = a + b * c;
    v8sf t2 = (a - b) / (c + d);
    v8sf t3 = t1 * t2 + a / b;
    v8sf t4 = (t3 - a) * (b + c) / d;
    
    /* Broadcast scalar to vector (adds operand) */
    v8sf scalar_bcast = (v8sf){3.14f, 3.14f, 3.14f, 3.14f, 
                                3.14f, 3.14f, 3.14f, 3.14f};
    
    /* Final complex expression */
    return (t1 + t2) * (t3 - t4) / scalar_bcast + a * b - c / d;
}

/* OpenMP SIMD reduction with vector types */
void test_omp_reduction(float* arr, int n, float* result) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i += 8) {
        v8sf chunk;
        memcpy(&chunk, &arr[i], sizeof(v8sf));
        sum = sum + chunk * chunk - chunk / 2.0f;
    }
    
    memcpy(result, &sum, sizeof(v8sf));
}

/* Main test driver */
int main() {
    printf("Testing many-operand instruction patterns\n");
    
    /* Test inline assembly path */
    test_many_operand_asm();
    
    #ifdef __x86_64__
    printf("Testing x86_64 AVX-512 patterns\n");
    
    /* Initialize AVX-512 vectors */
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;
    
    __m512 avx_result = test_avx512_many_operands(avx_a, avx_b, avx_c, avx_d, mask);
    
    /* Test FMA chain */
    float fma_result = test_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f);
    printf("FMA chain result: %f\n", fma_result);
    
    #elif defined(__aarch64__)
    printf("Testing AArch64 NEON patterns\n");
    
    float32x4_t neon_a = vdupq_n_f32(1.0f);
    float32x4_t neon_b = vdupq_n_f32(2.0f);
    float32x4_t neon_c = vdupq_n_f32(3.0f);
    float32x4_t neon_d = vdupq_n_f32(4.0f);
    uint32x4_t neon_mask = vdupq_n_u32(0xFFFFFFFF);
    
    float32x4_t neon_result = test_neon_many_operands(neon_a, neon_b, neon_c, neon_d, neon_mask);
    #endif
    
    /* Test GCC vector extensions */
    printf("Testing GCC vector extensions\n");
    v8sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vec_b = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf vec_c = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf vec_d = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    
    v8sf vec_result = test_vector_extensions(vec_a, vec_b, vec_c, vec_d);
    
    /* Test OpenMP reduction */
    printf("Testing OpenMP SIMD reduction\n");
    float arr[1024];
    float omp_result[8];
    
    for (int i = 0; i < 1024; i++) {
        arr[i] = (float)i;
    }
    
    test_omp_reduction(arr, 1024, omp_result);
    
    /* Use results to prevent optimization */
    float sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += omp_result[i];
    }
    
    printf("Final result: %f\n", sum);
    
    return (int)sum;
}
