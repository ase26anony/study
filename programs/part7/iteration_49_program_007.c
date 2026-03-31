/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to test 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 intrinsic that often expands to many operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i a = _mm512_set1_epi64(1);
    __m512i b = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 k = 0xFF;
    
    /* Complex permute operation that may use many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(a, k, idx, b);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_extract_epi64(result, 0);
    #endif
    
    /* Multi-operand inline assembly for x86_64 */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result_asm;
    
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        "subq %[d], %[b]\n\t"
        "xorq %[e], %[b]\n\t"
        "orq  %[f], %[b]\n\t"
        "andq %[g], %[b]\n\t"
        "shlq $3, %[b]\n\t"
        "shrq $1, %[b]\n\t"
        "movq %[b], %[r]\n\t"
        : [r] "=r" (result_asm)
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3),
          [d] "r" (op4), [e] "r" (op5), [f] "r" (op6),
          [g] "r" (op7), "0" (op8)
        : "cc", "memory"
    );
    
    global_counter += result_asm;
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE intrinsics */
    #ifdef __ARM_NEON
    #include <arm_neon.h>
    uint64x2_t v1 = vdupq_n_u64(1);
    uint64x2_t v2 = vdupq_n_u64(2);
    uint64x2_t v3 = vdupq_n_u64(3);
    uint64x2_t v4 = vdupq_n_u64(4);
    
    /* Complex sequence that might generate multi-operand pattern */
    uint64x2_t r1 = vaddq_u64(v1, v2);
    uint64x2_t r2 = vmulq_u64(v3, v4);
    uint64x2_t result = veorq_u64(r1, r2);
    
    global_counter += vgetq_lane_u64(result, 0);
    #endif
    
    /* Multi-operand inline assembly for AArch64 */
    unsigned long x0 = 1, x1 = 2, x2 = 3, x3 = 4, x4 = 5;
    unsigned long x5 = 6, x6 = 7, x7 = 8, x8 = 9, x9 = 10;
    unsigned long res;
    
    asm volatile (
        "mul   %[res], %[a], %[b]\n\t"
        "add   %[res], %[res], %[c]\n\t"
        "sub   %[res], %[res], %[d]\n\t"
        "eor   %[res], %[res], %[e]\n\t"
        "orr   %[res], %[res], %[f]\n\t"
        "and   %[res], %[res], %[g]\n\t"
        "lsl   %[res], %[res], #3\n\t"
        "lsr   %[res], %[res], #1\n\t"
        : [res] "=&r" (res)
        : [a] "r" (x0), [b] "r" (x1), [c] "r" (x2),
          [d] "r" (x3), [e] "r" (x4), [f] "r" (x5),
          [g] "r" (x6), "0" (x7)
        : "cc"
    );
    
    global_counter += res;
#else
    /* Generic multi-operand inline assembly fallback */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        "mov   %[r], %[a]\n\t"
        "add   %[r], %[r], %[b]\n\t"
        "sub   %[r], %[r], %[c]\n\t"
        "imul  %[r], %[r], %[d]\n\t"
        "xor   %[r], %[r], %[e]\n\t"
        "or    %[r], %[r], %[f]\n\t"
        "and   %[r], %[r], %[g]\n\t"
        "shl   %[r], $3\n\t"
        "shr   %[r], $1\n\t"
        : [r] "=&r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), "0" (h)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Complex inline assembly with exactly 11 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5;
    long f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result;
    
    asm volatile (
        "mov   %[r], %[a]\n\t"
        "add   %[r], %[b]\n\t"
        "sub   %[r], %[c]\n\t"
        "imul  %[r], %[d]\n\t"
        "xor   %[r], %[e]\n\t"
        "or    %[r], %[f]\n\t"
        "and   %[r], %[g]\n\t"
        "shl   %[r], $2\n\t"
        "shr   %[r], $1\n\t"
        "add   %[r], %[h]\n\t"
        "sub   %[r], %[i]\n\t"
        : [r] "=&r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          "0" (j)
        : "cc", "memory"
    );
    
    global_counter += result;
    
    /* AVX-512 masked operation with multiple parameters */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi32(1);
    __m512i src2 = _mm512_set1_epi32(2);
    __m512i src3 = _mm512_set1_epi32(3);
    __mmask16 mask = 0xAAAA;
    int imm8 = 5;
    
    /* This intrinsic often expands to many operands */
    __m512i res = _mm512_mask3_fmadd_epi32(src1, src2, src3, mask);
    global_counter += _mm512_extract_epi32(res, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM inline assembly with 11 operands */
    unsigned long x0 = 1, x1 = 2, x2 = 3, x3 = 4, x4 = 5;
    unsigned long x5 = 6, x6 = 7, x7 = 8, x8 = 9, x9 = 10, x10 = 11;
    unsigned long res;
    
    asm volatile (
        "mov   %[r], %[a]\n\t"
        "add   %[r], %[r], %[b]\n\t"
        "sub   %[r], %[r], %[c]\n\t"
        "mul   %[r], %[r], %[d]\n\t"
        "eor   %[r], %[r], %[e]\n\t"
        "orr   %[r], %[r], %[f]\n\t"
        "and   %[r], %[r], %[g]\n\t"
        "lsl   %[r], %[r], #2\n\t"
        "lsr   %[r], %[r], #1\n\t"
        "add   %[r], %[r], %[h]\n\t"
        "sub   %[r], %[r], %[i]\n\t"
        : [r] "=&r" (res)
        : [a] "r" (x0), [b] "r" (x1), [c] "r" (x2),
          [d] "r" (x3), [e] "r" (x4), [f] "r" (x5),
          [g] "r" (x6), [h] "r" (x7), [i] "r" (x8),
          "0" (x9)
        : "cc"
    );
    
    global_counter += res;
    
    #ifdef __ARM_NEON
    /* Complex NEON operation chain */
    #include <arm_neon.h>
    uint32x4_t v1 = vdupq_n_u32(1);
    uint32x4_t v2 = vdupq_n_u32(2);
    uint32x4_t v3 = vdupq_n_u32(3);
    uint32x4_t v4 = vdupq_n_u32(4);
    uint32x4_t v5 = vdupq_n_u32(5);
    
    uint32x4_t t1 = vaddq_u32(v1, v2);
    uint32x4_t t2 = vmulq_u32(v3, v4);
    uint32x4_t t3 = vsubq_u32(t1, t2);
    uint32x4_t final = veorq_u32(t3, v5);
    
    global_counter += vgetq_lane_u32(final, 0);
    #endif
#else
    /* Generic 11-operand inline assembly */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    asm volatile (
        "mov   %[r], %[a]\n\t"
        "add   %[r], %[b]\n\t"
        "sub   %[r], %[c]\n\t"
        "imul  %[r], %[d]\n\t"
        "xor   %[r], %[e]\n\t"
        "or    %[r], %[f]\n\t"
        "and   %[r], %[g]\n\t"
        "shl   %[r], $2\n\t"
        "shr   %[r], $1\n\t"
        "add   %[r], %[h]\n\t"
        "sub   %[r], %[i]\n\t"
        : [r] "=&r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          "0" (j)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Main driver */
int main(void) {
    printf("Testing 10 and 11 operand RTL patterns...\n");
    
    /* Call both test functions */
    test_10_operand();
    test_11_operand();
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", global_counter);
    
    return global_counter == 0 ? 0 : 1;
}
