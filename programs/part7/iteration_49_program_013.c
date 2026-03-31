/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to trigger 10-operand RTL pattern */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* x86_64: Use AVX-512 complex permute with mask - often expands to many operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute with mask - can generate many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_extract_epi64(result, 0);
    #endif
    
    /* Fallback: Multi-operand inline assembly for x86 */
    #ifndef __AVX512F__
    uint64_t a, b, c, d, e, f, g, h, i, j;
    asm volatile (
        "movq $1, %0\n\t"
        "movq $2, %1\n\t"
        "movq $3, %2\n\t"
        "movq $4, %3\n\t"
        "movq $5, %4\n\t"
        "movq $6, %5\n\t"
        "movq $7, %6\n\t"
        "movq $8, %7\n\t"
        "movq $9, %8\n\t"
        "movq $10, %9"
        : "=r"(a), "=r"(b), "=r"(c), "=r"(d), "=r"(e),
          "=r"(f), "=r"(g), "=r"(h), "=r"(i), "=r"(j)
        :
        : "memory"
    );
    global_counter += a + b + c + d + e + f + g + h + i + j;
    #endif
#elif defined(__aarch64__)
    /* AArch64: Use NEON complex operations */
    #include <arm_neon.h>
    
    /* Create multiple vector registers */
    uint64x2_t v1 = vdupq_n_u64(1);
    uint64x2_t v2 = vdupq_n_u64(2);
    uint64x2_t v3 = vdupq_n_u64(3);
    uint64x2_t v4 = vdupq_n_u64(4);
    uint64x2_t v5 = vdupq_n_u64(5);
    
    /* Complex sequence that might combine into multi-operand pattern */
    uint64x2_t t1 = vaddq_u64(v1, v2);
    uint64x2_t t2 = vaddq_u64(v3, v4);
    uint64x2_t t3 = vaddq_u64(t1, t2);
    uint64x2_t result = vaddq_u64(t3, v5);
    
    global_counter += vgetq_lane_u64(result, 0);
#else
    /* Generic fallback with complex expression */
    int64_t ops[10];
    for (int i = 0; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* Complex expression that might generate multi-operand RTL */
    int64_t result = ops[0] * ops[1] + ops[2] * ops[3] - ops[4] * ops[5] +
                     ops[6] * ops[7] / ops[8] + ops[9];
    
    global_counter += result;
#endif
}

/* Function to trigger 11-operand RTL pattern */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* x86_64: Inline assembly with exactly 11 operands */
    uint64_t a, b, c, d, e, f, g, h, i, j, k;
    
    /* Complex inline assembly with 11 output operands */
    asm volatile (
        "movq $1, %0\n\t"
        "movq $2, %1\n\t"
        "movq $3, %2\n\t"
        "movq $4, %3\n\t"
        "movq $5, %4\n\t"
        "movq $6, %5\n\t"
        "movq $7, %6\n\t"
        "movq $8, %7\n\t"
        "movq $9, %8\n\t"
        "movq $10, %9\n\t"
        "movq $11, %10"
        : "=r"(a), "=r"(b), "=r"(c), "=r"(d), "=r"(e),
          "=r"(f), "=r"(g), "=r"(h), "=r"(i), "=r"(j),
          "=r"(k)
        :
        : "memory"
    );
    
    global_counter += a + b + c + d + e + f + g + h + i + j + k;
    
    #ifdef __AVX512F__
    /* Alternative: AVX-512 masked store with complex addressing */
    #include <immintrin.h>
    __m512i data = _mm512_set1_epi64(42);
    __mmask8 mask = 0x0F;
    int64_t* aligned_ptr = (int64_t*)aligned_alloc(64, 64);
    
    if (aligned_ptr) {
        _mm512_mask_storeu_epi64(aligned_ptr, mask, data);
        global_counter += aligned_ptr[0];
        free(aligned_ptr);
    }
    #endif
#elif defined(__aarch64__)
    /* AArch64: Complex vector operation chain */
    #include <arm_neon.h>
    
    /* Create 11 vector elements worth of data */
    uint64x2_t vectors[6];
    for (int i = 0; i < 6; i++) {
        vectors[i] = vdupq_n_u64(i + 1);
    }
    
    /* Complex chain of operations that might merge */
    uint64x2_t sum1 = vaddq_u64(vectors[0], vectors[1]);
    uint64x2_t sum2 = vaddq_u64(vectors[2], vectors[3]);
    uint64x2_t sum3 = vaddq_u64(vectors[4], vectors[5]);
    uint64x2_t sum4 = vaddq_u64(sum1, sum2);
    uint64x2_t final = vaddq_u64(sum3, sum4);
    
    /* Extract and use multiple elements */
    uint64_t lane0 = vgetq_lane_u64(final, 0);
    uint64_t lane1 = vgetq_lane_u64(final, 1);
    
    global_counter += lane0 + lane1;
#else
    /* Generic: Complex expression with 11 variables */
    int64_t vars[11];
    for (int i = 0; i < 11; i++) {
        vars[i] = i + 1;
    }
    
    /* Very complex expression that might generate 11-operand RTL */
    int64_t result = 
        vars[0] * vars[1] + 
        vars[2] * vars[3] - 
        vars[4] * vars[5] + 
        vars[6] * vars[7] / 
        (vars[8] + 1) * 
        vars[9] - 
        vars[10];
    
    global_counter += result;
#endif
}

/* Additional test with OpenMP SIMD pragmas to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (int i = 0; i < 100; i++) {
        /* Complex operation that might be vectorized into multi-operand pattern */
        global_counter += i * 2 + i / 3 - i % 5;
    }
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", global_counter);
    printf("Test completed successfully.\n");
    
    return 0;
}
