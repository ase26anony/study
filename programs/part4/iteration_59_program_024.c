/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Use target-specific vector intrinsics */
#ifdef __AVX512F__
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

/* Strategy 2: GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function to encourage complex instruction patterns */
__attribute__((hot, noinline))
void test_vector_operations(float* result, const float* a, const float* b, 
                           const float* c, int n) {
    /* Complex expression that may require many operands when expanded */
    for (int i = 0; i < n; i += 8) {
        /* Load vectors */
        v8sf va = *(const v8sf*)(a + i);
        v8sf vb = *(const v8sf*)(b + i);
        v8sf vc = *(const v8sf*)(c + i);
        
        /* Complex expression with multiple operations */
        v8sf temp = va * vb + vc;
        temp = temp * va - vb;
        temp = temp + va * vc;
        
        /* Store result */
        *(v8sf*)(result + i) = temp;
    }
}

/* Strategy 3: Inline assembly with many operands */
__attribute__((noinline))
uint64_t test_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                               uint64_t d, uint64_t e, uint64_t f,
                               uint64_t g, uint64_t h, uint64_t i,
                               uint64_t j) {
    uint64_t r1, r2, r3, r4, r5;
    
    /* 11-operand inline assembly (5 outputs + 10 inputs = 15 total operands,
       but the compiler processes them individually) */
    asm volatile (
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "sub %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "and %2, %2, %10\n\t"
        "mov %3, %11\n\t"
        "or %3, %3, %12\n\t"
        "mov %4, %13\n\t"
        "xor %4, %4, %14\n\t"
        : "=&r"(r1), "=&r"(r2), "=&r"(r3), "=&r"(r4), "=&r"(r5)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    return r1 + r2 + r3 + r4 + r5;
}

/* Strategy 4: Built-in functions for complex math */
__attribute__((noinline))
double test_builtin_fma(double a, double b, double c, double d, double e) {
    /* Nested FMA operations that may expand to multi-operand patterns */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, t1);
    double t3 = __builtin_fma(a, c, t2);
    double t4 = __builtin_fma(b, d, t3);
    double t5 = __builtin_fma(c, e, t4);
    
    return __builtin_fma(t1, t2, __builtin_fma(t3, t4, t5));
}

/* Strategy 5: AVX-512 specific intrinsics (if available) */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
__m512 test_avx512_multi_operand(__m512 a, __m512 b, __m512 c, 
                                 __m512 d, __mmask16 k) {
    /* AVX-512 masked operations with many operands */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_mul_ps(t1, k, d, a);
    __m512 t3 = _mm512_mask_fmadd_ps(b, k, c, d);
    __m512 t4 = _mm512_mask_sub_ps(t2, k, t3, a);
    
    return _mm512_mask_blend_ps(k, t4, _mm512_maskz_fmadd_ps(k, a, b, c));
}
#endif

/* Main function to prevent dead code elimination */
int main() {
    const int N = 64;
    float a[N], b[N], c[N], result[N];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
        c[i] = i * 0.3f;
    }
    
    /* Test 1: Vector operations */
    test_vector_operations(result, a, b, c, N);
    
    /* Test 2: Multi-operand inline assembly */
    uint64_t asm_result = test_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test 3: Built-in FMA operations */
    double fma_result = test_builtin_fma(1.0, 2.0, 3.0, 4.0, 5.0);
    
    /* Test 4: AVX-512 operations if available */
    #ifdef __AVX512F__
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;
    __m512 avx_result = test_avx512_multi_operand(avx_a, avx_b, avx_c, avx_d, mask);
    float avx_sum = _mm512_reduce_add_ps(avx_result);
    #else
    float avx_sum = 0.0f;
    #endif
    
    /* Use results to prevent optimization */
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += result[i];
    }
    
    printf("Results: vector_sum=%f, asm=%lu, fma=%f, avx=%f\n", 
           sum, asm_result, fma_result, avx_sum);
    
    return (sum > 0 && asm_result > 0) ? 0 : 1;
}
