/* Test program to trigger 10 and 11 operand RTL patterns in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function attribute to force specific optimization */
#define FORCE_OPTIMIZE __attribute__((optimize("O3", "no-inline")))

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test function for 10-operand case using AVX-512 intrinsics */
FORCE_OPTIMIZE
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation that may generate 10-operand RTL */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* vpermi2q with mask - potentially expands to many operands */
    __m512i result = _mm512_mask2_permutex2var_epi64(src1, idx, mask, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
}

/* Test function for 11-operand case using inline assembly */
FORCE_OPTIMIZE
void test_11_operand_x86(void) {
    /* Inline assembly with 11 operands */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t f = 6, g = 7, h = 8, i = 9, j = 10;
    uint64_t result = 0;
    
    /* Complex inline asm with 11 operands (5 inputs, 1 output, 5 clobbers) */
    asm volatile (
        "mov %[a], %[res]\n\t"
        "add %[b], %[res]\n\t"
        "imul %[c], %[res]\n\t"
        "add %[d], %[res]\n\t"
        "sub %[e], %[res]\n\t"
        "xor %[f], %[res]\n\t"
        "or %[g], %[res]\n\t"
        "and %[h], %[res]\n\t"
        "add %[i], %[res]\n\t"
        "sub %[j], %[res]"
        : [res] "=&r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "cc", "memory", "rax", "rbx", "rcx"
    );
    
    global_counter += result;
}

#elif defined(__aarch64__) || defined(__arm64__)
#include <arm_neon.h>

/* Test function for 10-operand case using ARM NEON */
FORCE_OPTIMIZE
void test_10_operand_arm(void) {
    /* Complex NEON operation with multiple operands */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Chain of operations that might combine into multi-operand pattern */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vmulq_u8(r1, r2);
    uint8x16_t r4 = vbslq_u8(vdupq_n_u8(0xFF), r3, a);
    
    /* Extract and use result */
    uint8_t lane = vgetq_lane_u8(r4, 0);
    global_counter += lane;
}

/* Test function for 11-operand case using inline assembly */
FORCE_OPTIMIZE
void test_11_operand_arm(void) {
    /* Inline assembly with 11 operands for ARM */
    uint64_t regs[10];
    uint64_t result = 0;
    
    for (int i = 0; i < 10; i++) {
        regs[i] = i + 1;
    }
    
    /* Complex inline asm with 11 operands */
    asm volatile (
        "mov %[res], %[r0]\n\t"
        "add %[res], %[res], %[r1]\n\t"
        "mul %[res], %[res], %[r2]\n\t"
        "add %[res], %[res], %[r3]\n\t"
        "sub %[res], %[res], %[r4]\n\t"
        "eor %[res], %[res], %[r5]\n\t"
        "orr %[res], %[res], %[r6]\n\t"
        "and %[res], %[res], %[r7]\n\t"
        "add %[res], %[res], %[r8]\n\t"
        "sub %[res], %[res], %[r9]"
        : [res] "=&r" (result)
        : [r0] "r" (regs[0]), [r1] "r" (regs[1]), [r2] "r" (regs[2]),
          [r3] "r" (regs[3]), [r4] "r" (regs[4]), [r5] "r" (regs[5]),
          [r6] "r" (regs[6]), [r7] "r" (regs[7]), [r8] "r" (regs[8]),
          [r9] "r" (regs[9])
        : "cc", "memory"
    );
    
    global_counter += result;
}

#else
/* Generic fallback using complex inline assembly */
FORCE_OPTIMIZE
void test_10_operand_generic(void) {
    /* Generic inline assembly with 10 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5;
    long f = 6, g = 7, h = 8, i = 9;
    long result = 0;
    
    asm volatile (
        "mov %[a], %[res]\n\t"
        "add %[b], %[res]\n\t"
        "imul %[c], %[res]\n\t"
        "add %[d], %[res]\n\t"
        "sub %[e], %[res]\n\t"
        "xor %[f], %[res]\n\t"
        "or %[g], %[res]\n\t"
        "and %[h], %[res]\n\t"
        "add %[i], %[res]"
        : [res] "=&r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc", "memory"
    );
    
    global_counter += result;
}

FORCE_OPTIMIZE
void test_11_operand_generic(void) {
    /* Generic inline assembly with 11 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5;
    long f = 6, g = 7, h = 8, i = 9, j = 10;
    long result = 0;
    
    asm volatile (
        "mov %[a], %[res]\n\t"
        "add %[b], %[res]\n\t"
        "imul %[c], %[res]\n\t"
        "add %[d], %[res]\n\t"
        "sub %[e], %[res]\n\t"
        "xor %[f], %[res]\n\t"
        "or %[g], %[res]\n\t"
        "and %[h], %[res]\n\t"
        "add %[i], %[res]\n\t"
        "sub %[j], %[res]"
        : [res] "=&r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "cc", "memory"
    );
    
    global_counter += result;
}
#endif

/* Complex vector operation that might generate multi-operand RTL */
FORCE_OPTIMIZE
void test_complex_vector_chain(void) {
    /* Create a complex chain of operations that might be combined */
    int arr1[16] = {0};
    int arr2[16] = {0};
    int arr3[16] = {0};
    int arr4[16] = {0};
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        arr4[i] = i * 4;
    }
    
    /* Complex computation chain */
    for (int i = 0; i < 16; i++) {
        int t1 = arr1[i] + arr2[i];
        int t2 = arr3[i] * arr4[i];
        int t3 = t1 ^ t2;
        int t4 = (t3 << 2) | (t3 >> 30);
        arr1[i] = t4;
        
        /* Additional operations to increase complexity */
        arr2[i] = (arr1[i] + arr3[i]) * arr4[i];
        arr3[i] = arr2[i] - arr1[i];
        arr4[i] = arr3[i] ^ arr2[i] ^ arr1[i];
    }
    
    /* Use results */
    for (int i = 0; i < 16; i++) {
        global_counter += arr1[i] + arr2[i] + arr3[i] + arr4[i];
    }
}

/* Main driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
#ifdef __x86_64__
    printf("Using x86_64 intrinsics and inline assembly\n");
    test_10_operand_x86();
    test_11_operand_x86();
#elif defined(__aarch64__) || defined(__arm64__)
    printf("Using ARM NEON and inline assembly\n");
    test_10_operand_arm();
    test_11_operand_arm();
#else
    printf("Using generic inline assembly\n");
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    /* Additional complex operation */
    test_complex_vector_chain();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed successfully\n");
    
    return 0;
}
