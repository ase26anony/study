/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to test 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 intrinsic that often expands to multi-operand RTL */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i a = _mm512_set1_epi64(1);
    __m512i b = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 k = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(a, k, idx, b);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON intrinsic with multiple operands */
    #include <arm_neon.h>
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t mask = vdupq_n_u8(0x80);
    
    /* Table lookup with multiple registers */
    uint8x16x4_t table = {a, b, a, b};
    uint8x16_t result = vqtbl4q_u8(table, vdupq_n_u8(3));
    
    /* Use result */
    global_counter += vgetq_lane_u8(result, 0);
#endif

    /* Generic multi-operand inline assembly fallback */
    __asm__ volatile (
        /* 10 operands: 5 inputs, 1 output, 4 clobbers */
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "sub %0, %3\n\t"
        "mul %0, %4\n\t"
        "div %0, %5"
        : "=r"(global_counter)      /* operand 0: output */
        : "r"(global_counter),      /* operand 1: input 1 */
          "r"(1),                   /* operand 2: input 2 */
          "r"(2),                   /* operand 3: input 3 */
          "r"(3),                   /* operand 4: input 4 */
          "r"(4)                    /* operand 5: input 5 */
        : "memory", "cc", "rax", "rdx"  /* operands 6-9: clobbers */
    );
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* AVX-512 masked operation with immediate */
    #ifdef __AVX512BW__
    #include <immintrin.h>
    __m512i src = _mm512_set1_epi16(100);
    __m512i a = _mm512_set1_epi16(200);
    __m512i b = _mm512_set1_epi16(300);
    __mmask32 k = 0xAAAAAAAA;
    
    /* Complex blend with immediate control */
    __m512i result = _mm512_mask_blend_epi16(k, a, b);
    
    /* Additional operation to create complex RTL */
    result = _mm512_add_epi16(result, src);
    result = _mm512_slli_epi16(result, 3);
    
    global_counter += _mm512_reduce_add_epi16(result);
    #endif
    
#elif defined(__aarch64__)
    /* ARM SVE-like pattern (if available) */
    #include <arm_neon.h>
    /* Complex multiple register operation */
    uint8x16_t v0 = vdupq_n_u8(0);
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    
    /* Extended table lookup */
    uint8x16x4_t tbl1 = {v0, v1, v2, v3};
    uint8x16x4_t tbl2 = {v3, v2, v1, v0};
    
    uint8x16_t idx = vdupq_n_u8(5);
    uint8x16_t r1 = vqtbl4q_u8(tbl1, idx);
    uint8x16_t r2 = vqtbl4q_u8(tbl2, idx);
    
    uint8x16_t result = veorq_u8(r1, r2);
    global_counter += vgetq_lane_u8(result, 0);
#endif

    /* Explicit 11-operand inline assembly */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result;
    
    __asm__ volatile (
        /* Complex operation using 11 operands */
        "imul %1, %2\n\t"
        "add %1, %3\n\t"
        "sub %1, %4\n\t"
        "xor %1, %5\n\t"
        "or %1, %6\n\t"
        "and %1, %7\n\t"
        "shl %1, %8\n\t"
        "shr %1, %9\n\t"
        "mov %0, %1\n\t"
        "add %0, %10"
        : "=r"(result)              /* operand 0: output */
        : "r"(op1),                 /* operand 1: input/output */
          "r"(op2),                 /* operand 2: input */
          "r"(op3),                 /* operand 3: input */
          "r"(op4),                 /* operand 4: input */
          "r"(op5),                 /* operand 5: input */
          "r"(op6),                 /* operand 6: input */
          "r"(op7),                 /* operand 7: input */
          "r"(op8),                 /* operand 8: input */
          "r"(op9),                 /* operand 9: input */
          "r"(op10)                 /* operand 10: input */
        : "memory", "cc"            /* implicit clobbers */
    );
    
    global_counter += result;
}

/* Additional test with vector operations that may combine */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    /* Chain of operations that might combine into multi-operand RTL */
    __m512i v1 = _mm512_set1_epi32(global_counter);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    /* Complex sequence */
    __m512i t1 = _mm512_add_epi32(v1, v2);
    __m512i t2 = _mm512_mullo_epi32(t1, v3);
    __m512i t3 = _mm512_slli_epi32(t2, 2);
    __m512i t4 = _mm512_sub_epi32(t3, v4);
    __m512i t5 = _mm512_and_epi32(t4, _mm512_set1_epi32(0xFF));
    
    /* Permute with many operands */
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i result = _mm512_permutexvar_epi32(idx, t5);
    
    global_counter += _mm512_reduce_add_epi32(result);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", global_counter);
    return global_counter != 0 ? 0 : 1;
}
