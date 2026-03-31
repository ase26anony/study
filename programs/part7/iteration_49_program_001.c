/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to trigger 10-operand case */
__attribute__((optimize("O3")))
__attribute__((noinline))
void test_10_operand(void) {
#ifdef __x86_64__
    /* x86_64: Use AVX-512 intrinsics that often expand to multi-operand patterns */
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent elimination */
    global_counter += _mm512_reduce_add_epi64(result);
    
#elif defined(__aarch64__)
    /* ARM NEON: Complex vector operations */
    #include <arm_neon.h>
    
    uint8x16x4_t vec4 = {
        vdupq_n_u8(1),
        vdupq_n_u8(2),
        vdupq_n_u8(3),
        vdupq_n_u8(4)
    };
    
    /* Complex table lookup with multiple registers */
    uint8x16_t indices = vcombine_u8(
        vcreate_u8(0x0706050403020100ULL),
        vcreate_u8(0x0F0E0D0C0B0A0908ULL)
    );
    
    uint8x16_t result = vqtbl4q_u8(vec4, indices);
    
    /* Use result */
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic: Multi-operand inline assembly */
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, op10;
    
    asm volatile (
        "mov %0, #1\n\t"
        "mov %1, #2\n\t"
        "mov %2, #3\n\t"
        "mov %3, #4\n\t"
        "mov %4, #5\n\t"
        "mov %5, #6\n\t"
        "mov %6, #7\n\t"
        "mov %7, #8\n\t"
        "mov %8, #9\n\t"
        "mov %9, #10\n\t"
        : "=r"(op1), "=r"(op2), "=r"(op3), "=r"(op4),
          "=r"(op5), "=r"(op6), "=r"(op7), "=r"(op8),
          "=r"(op9), "=r"(op10)
        :
        : "cc"
    );
    
    global_counter += op1 + op10;
#endif
}

/* Function to trigger 11-operand case */
__attribute__((optimize("O3")))
__attribute__((noinline))
void test_11_operand(void) {
#ifdef __x86_64__
    /* AVX-512: Complex masked operation with immediate */
    #include <immintrin.h>
    
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0x5555;
    
    /* Complex sequence that might combine into multi-operand pattern */
    __m512i temp = _mm512_mask_add_epi32(a, mask1, b, c);
    __m512i result = _mm512_mask_sub_epi32(temp, mask2, c, a);
    
    global_counter += _mm512_reduce_add_epi32(result);
    
#elif defined(__aarch64__)
    /* ARM: Complex vector permute with multiple registers */
    #include <arm_neon.h>
    
    uint8x16_t v0 = vdupq_n_u8(0);
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    uint8x16_t indices = vcombine_u8(
        vcreate_u8(0x0F0E0D0C0B0A0908ULL),
        vcreate_u8(0x0706050403020100ULL)
    );
    
    /* Complex table lookup chain */
    uint8x16_t t1 = vqtbl1q_u8(v0, indices);
    uint8x16_t t2 = vqtbl1q_u8(v1, t1);
    uint8x16_t t3 = vqtbl1q_u8(v2, t2);
    uint8x16_t t4 = vqtbl1q_u8(v3, t3);
    uint8x16_t result = vqtbl1q_u8(v4, t4);
    
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic: Explicit 11-operand inline assembly */
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, op10, op11;
    
    asm volatile (
        "mov %0, #1\n\t"
        "mov %1, #2\n\t"
        "mov %2, #3\n\t"
        "mov %3, #4\n\t"
        "mov %4, #5\n\t"
        "mov %5, #6\n\t"
        "mov %6, #7\n\t"
        "mov %7, #8\n\t"
        "mov %8, #9\n\t"
        "mov %9, #10\n\t"
        "mov %10, #11\n\t"
        : "=r"(op1), "=r"(op2), "=r"(op3), "=r"(op4),
          "=r"(op5), "=r"(op6), "=r"(op7), "=r"(op8),
          "=r"(op9), "=r"(op10), "=r"(op11)
        :
        : "cc"
    );
    
    global_counter += op1 + op11;
#endif
}

/* Additional test with complex vector operations */
__attribute__((optimize("O3")))
__attribute__((noinline))
void test_vector_chain(void) {
#ifdef __x86_64__
    #include <immintrin.h>
    
    /* Complex chain that might generate multi-operand RTL */
    __m512i v1 = _mm512_set_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i v2 = _mm512_set_epi64(8, 7, 6, 5, 4, 3, 2, 1);
    __m512i v3 = _mm512_set1_epi64(42);
    __mmask8 m = 0x0F;
    
    /* Complex expression that might be combined */
    __m512i r1 = _mm512_maskz_mullo_epi64(m, v1, v2);
    __m512i r2 = _mm512_mask_add_epi64(v3, m, r1, v1);
    __m512i result = _mm512_mask_sub_epi64(r2, ~m & 0xFF, v2, v3);
    
    global_counter += _mm512_reduce_add_epi64(result);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call test functions multiple times */
    for (int i = 0; i < 10; i++) {
        test_10_operand();
        test_11_operand();
        test_vector_chain();
    }
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return global_counter > 0 ? 0 : 1;
}
