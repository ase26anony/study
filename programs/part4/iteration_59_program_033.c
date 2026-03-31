/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Use target-specific vector intrinsics with many operands */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Strategy 2: GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage complex instruction patterns */
__attribute__((hot, noinline))
void test_vector_intrinsics() {
    /* This function uses various approaches to generate multi-operand patterns */
    
#ifdef __AVX512F__
    /* AVX-512 masked operations can generate many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __mmask16 mask = 0xAAAA;
    
    /* Fused multiply-add with mask - potentially expands to many operands */
    __m512 result1 = _mm512_mask_fmadd_ps(a, mask, b, c);
    
    /* Complex permute/shuffle with mask */
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512 result2 = _mm512_mask_permutexvar_ps(a, mask, idx, b);
    
    /* Store with mask and scale factor */
    float array[16] __attribute__((aligned(64)));
    _mm512_mask_store_ps(array, mask, result1);
#endif

#ifdef __ARM_NEON
    /* ARM NEON multi-vector loads can generate many operands */
    int8x16x4_t vec4;
    int8_t data[64] = {0};
    vec4 = vld4q_s8(data);
    
    /* Complex table lookup with multiple registers */
    uint8x16_t tab_result;
    uint8x16_t indices = vdupq_n_u8(0);
    uint8x16x4_t tables;
    tables.val[0] = vdupq_n_u8(1);
    tables.val[1] = vdupq_n_u8(2);
    tables.val[2] = vdupq_n_u8(3);
    tables.val[3] = vdupq_n_u8(4);
    
    /* This intrinsic often expands to multiple operands */
    tab_result = vqtbl4q_u8(tables, indices);
#endif

    /* GCC vector extensions with complex expressions */
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf v3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    
    /* Complex expression that might generate FMA with many operands */
    v8sf result = v1 * v2 + v3;
    v8sf result2 = v2 * v3 - v1;
    v8sf final_result = result + result2;
    
    /* Use result to prevent elimination */
    volatile v8sf sink = final_result;
}

/* Strategy 3: Inline assembly with many operands */
__attribute__((noinline))
void test_multi_operand_asm() {
    /* Create 11 distinct variables for inline assembly */
    int64_t out1, out2, out3;
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6, in7 = 7;
    
    /* Extended asm with 10 operands (2 outputs, 7 inputs, 1 clobber) */
    asm volatile (
        "mov %[o1], %[i1]\n\t"
        "add %[o1], %[i2]\n\t"
        "mov %[o2], %[i3]\n\t"
        "sub %[o2], %[i4]\n\t"
        "mov %[o3], %[i5]\n\t"
        "imul %[o3], %[i6], %[i7]"
        : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
          [i7] "r" (in7)
        : "cc"
    );
    
    /* Use results to prevent elimination */
    volatile int64_t sink = out1 + out2 + out3;
}

/* Strategy 4: Built-in functions for complex math */
__attribute__((noinline, optimize("no-associative-math")))
float test_builtin_fma_chain() {
    /* Chain of FMA operations that might expand to multi-operand patterns */
    float a = 1.0f, b = 2.0f, c = 3.0f, d = 4.0f, e = 5.0f;
    
    /* Use __builtin_fmaf which maps directly to FMA instruction */
    float r1 = __builtin_fmaf(a, b, c);
    float r2 = __builtin_fmaf(r1, d, e);
    float r3 = __builtin_fmaf(a, r2, __builtin_fmaf(b, c, d));
    
    /* Complex expression preventing early folding */
    float result = r1 + r2 * r3 - __builtin_fmaf(a, b, __builtin_fmaf(c, d, e));
    
    return result;
}

/* Strategy 5: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
__attribute__((noinline))
v8sf test_omp_reduction() {
    v8sf array[100];
    v8sf sum = {0};
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        array[i] = (v8sf){i, i+1, i+2, i+3, i+4, i+5, i+6, i+7};
    }
    
    /* OpenMP SIMD reduction - may generate complex reduction patterns */
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum = sum + array[i];
    }
    
    return sum;
}
#endif

/* Main function that exercises all strategies */
int main() {
    printf("Testing multi-operand instruction patterns...\n");
    
    /* Test 1: Vector intrinsics */
    test_vector_intrinsics();
    
    /* Test 2: Multi-operand inline assembly */
    test_multi_operand_asm();
    
    /* Test 3: Built-in FMA chain */
    float fma_result = test_builtin_fma_chain();
    printf("FMA chain result: %f\n", fma_result);
    
    /* Test 4: OpenMP reduction if available */
    #ifdef _OPENMP
    v8sf omp_result = test_omp_reduction();
    volatile float sink = omp_result[0];
    #endif
    
    /* Additional test: Complex nested expressions */
    {
        /* Create a complex expression that might generate many temporaries */
        v8sf x = {1.0f}, y = {2.0f}, z = {3.0f};
        v8sf r = x * y + z * x - y * z + x / y;
        volatile v8sf sink2 = r;
    }
    
    return 0;
}
