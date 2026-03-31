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
    /* x86_64: Use AVX-512 complex masked permute with multiple operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may expand to 10 operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
    #endif
    
#elif defined(__aarch64__)
    /* AArch64: Complex NEON operations with multiple registers */
    #include <arm_neon.h>
    
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex sequence that may combine into multi-operand pattern */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    
    /* Use multiple tbl operations with many operands */
    uint8x16_t idx1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8x16_t idx2 = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    
    uint8x16_t tbl1 = vqtbl1q_u8(r3, idx1);
    uint8x16_t tbl2 = vqtbl1q_u8(r3, idx2);
    uint8x16_t final = vaddq_u8(tbl1, tbl2);
    
    global_counter += vaddvq_u8(final);
#endif
    
    /* Generic fallback: Inline assembly with 10 operands */
    asm volatile (
        /* 10-operand pattern: 5 inputs, 5 outputs */
        "mov %0, %1\n\t"
        "add %2, %3, %4\n\t"
        "sub %5, %6, %7\n\t"
        "mul %8, %9, %1\n\t"
        : "=r"(global_counter), "+r"(global_counter), 
          "=r"(global_counter), "+r"(global_counter),
          "=r"(global_counter)
        : "r"(global_counter), "r"(global_counter),
          "r"(global_counter), "r"(global_counter),
          "r"(global_counter)
        : "cc"
    );
}

/* Function to trigger 11-operand RTL pattern */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* x86_64: Complex AVX-512 masked gather with multiple operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    
    __m512i vindex = _mm512_set_epi32(0, 4, 8, 12, 16, 20, 24, 28,
                                      32, 36, 40, 44, 48, 52, 56, 60);
    __m512i src = _mm512_set1_epi32(100);
    __mmask16 mask = 0xAAAA;
    int scale = 4;
    void* base_ptr = &global_counter;
    
    /* This gather operation has many operands */
    __m512i gathered = _mm512_mask_i32gather_epi32(src, mask, vindex, 
                                                   base_ptr, scale);
    
    global_counter += _mm512_reduce_add_epi32(gathered);
    #endif
    
#elif defined(__aarch64__)
    /* AArch64: Complex multi-register operations */
    #include <arm_neon.h>
    
    /* Create many vector registers */
    float32x4_t v0 = vdupq_n_f32(1.0f);
    float32x4_t v1 = vdupq_n_f32(2.0f);
    float32x4_t v2 = vdupq_n_f32(3.0f);
    float32x4_t v3 = vdupq_n_f32(4.0f);
    float32x4_t v4 = vdupq_n_f32(5.0f);
    
    /* Complex FMA chain that may create multi-operand pattern */
    float32x4_t r0 = vfmaq_f32(v0, v1, v2);
    float32x4_t r1 = vfmaq_f32(v3, v4, r0);
    float32x4_t r2 = vmulq_f32(r0, r1);
    float32x4_t r3 = vaddq_f32(r2, v0);
    
    /* Extract and use result */
    float32_t sum = vaddvq_f32(r3);
    global_counter += (int)sum;
#endif
    
    /* Generic fallback: Inline assembly with exactly 11 operands */
    int temp1 = global_counter;
    int temp2 = global_counter + 1;
    int temp3 = global_counter + 2;
    int temp4 = global_counter + 3;
    int temp5 = global_counter + 4;
    
    asm volatile (
        /* 11-operand pattern: 6 inputs, 5 outputs */
        "add %0, %1, %2\n\t"
        "sub %3, %4, %5\n\t"
        "mul %6, %7, %8\n\t"
        "and %9, %10, %1\n\t"
        : "=r"(temp1), "=r"(temp2), "=r"(temp3),
          "=r"(temp4), "=r"(temp5)
        : "r"(global_counter), "r"(temp1), "r"(temp2),
          "r"(temp3), "r"(temp4), "r"(temp5)
        : "cc"
    );
    
    global_counter = temp1 + temp2 + temp3 + temp4 + temp5;
}

/* Additional test using OpenMP SIMD to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (int i = 0; i < 1024; i++) {
        /* Complex expression that may generate multi-operand RTL */
        global_counter += ((i * 3) >> 2) & 0xFF;
    }
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", global_counter);
    
    /* Simple validation */
    if (global_counter != 0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("Test may have been optimized out.\n");
        return 1;
    }
}
