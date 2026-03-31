/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force aggressive optimization on specific functions */
#define AGGRESSIVE_OPT __attribute__((optimize("O3", "unroll-loops")))

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
    #define X86_TARGET 1
    #include <immintrin.h>
    #include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
    #define ARM_TARGET 1
    #include <arm_neon.h>
#else
    #define GENERIC_TARGET 1
#endif

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* =========================================== */
/* Function targeting 10-operand RTL pattern   */
/* =========================================== */
AGGRESSIVE_OPT
void test_10_operand(void) {
    global_counter++;
    
#if X86_TARGET && defined(__AVX512F__)
    /* Complex AVX-512 masked permute with multiple immediates - often expands to 10 operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xF0;  /* 4 bits set */
    
    /* This intrinsic typically expands to RTL with: dest, mask, idx, src1, src2 + various immediates */
    __m512i result = _mm512_mask2_permutex2var_epi64(src1, idx, mask, src2);
    
    /* Use result to prevent optimization */
    long long* res_ptr = (long long*)&result;
    global_counter += res_ptr[0] & 1;
    
#elif ARM_TARGET
    /* ARM NEON complex table lookup with multiple registers */
    uint8x16x4_t table;
    table.val[0] = vdupq_n_u8(1);
    table.val[1] = vdupq_n_u8(2);
    table.val[2] = vdupq_n_u8(3);
    table.val[3] = vdupq_n_u8(4);
    
    uint8x16_t indices = vcombine_u8(
        vcreate_u8(0x0706050403020100ULL),
        vcreate_u8(0x0F0E0D0C0B0A0908ULL)
    );
    
    /* Complex table lookup with 4 table registers + indices */
    uint8x16_t result = vqtbl4q_u8(table, indices);
    
    /* Use result */
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    long long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long long result1, result2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* 10-operand pattern */\n\t"
        "mov %[r1], %[a1]\n\t"
        "add %[r1], %[a2]\n\t"
        "add %[r1], %[a3]\n\t"
        "mov %[r2], %[a4]\n\t"
        "add %[r2], %[a5]\n\t"
        "imul %[r1], %[a6]\n\t"
        "add %[r1], %[a7]\n\t"
        "sub %[r1], %[a8]\n\t"
        "xor %[r1], %[a9]\n\t"
        "or %[r1], %[a10]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc"
    );
    
    global_counter += result1 + result2;
#endif
    
    printf("10-operand test executed, counter = %d\n", global_counter);
}

/* =========================================== */
/* Function targeting 11-operand RTL pattern   */
/* =========================================== */
AGGRESSIVE_OPT
void test_11_operand(void) {
    global_counter += 2;
    
#if X86_TARGET && defined(__AVX512F__) && defined(__AVX512VBMI__)
    /* AVX-512 VBMI permute with multiple sources and mask - can expand to 11 operands */
    __m512i src1 = _mm512_set1_epi8('A');
    __m512i src2 = _mm512_set1_epi8('B');
    __m512i src3 = _mm512_set1_epi8('C');
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    /* Complex permutation pattern with multiple sources */
    __m512i idx = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    /* Multi-source permute that may expand to 11 operands */
    __m512i temp = _mm512_permutex2var_epi8(src1, idx, src2);
    __m512i result = _mm512_mask_blend_epi8(mask, temp, src3);
    
    /* Use result */
    char* res_ptr = (char*)&result;
    global_counter += res_ptr[0] & 1;
    
#elif ARM_TARGET
    /* Complex ARM NEON operation chain designed to merge into multi-operand pattern */
    float32x4_t a = vdupq_n_f32(1.0f);
    float32x4_t b = vdupq_n_f32(2.0f);
    float32x4_t c = vdupq_n_f32(3.0f);
    float32x4_t d = vdupq_n_f32(4.0f);
    
    /* Complex FMA chain that might be recognized as single pattern */
    float32x4_t r1 = vfmaq_f32(a, b, c);
    float32x4_t r2 = vfmaq_f32(r1, d, a);
    float32x4_t r3 = vmulq_f32(r2, b);
    float32x4_t result = vaddq_f32(r3, c);
    
    /* Use result */
    global_counter += (int)vgetq_lane_f32(result, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    long long a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    long long a6 = 6, a7 = 7, a8 = 8, a9 = 9, a10 = 10, a11 = 11;
    long long r1, r2, r3;
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand pattern */\n\t"
        "mov %[out1], %[in1]\n\t"
        "add %[out1], %[in2]\n\t"
        "mov %[out2], %[in3]\n\t"
        "imul %[out2], %[in4]\n\t"
        "mov %[out3], %[in5]\n\t"
        "add %[out3], %[in6]\n\t"
        "sub %[out1], %[in7]\n\t"
        "xor %[out2], %[in8]\n\t"
        "or %[out3], %[in9]\n\t"
        "and %[out1], %[in10]\n\t"
        "add %[out2], %[in11]"
        : [out1] "=&r" (r1), [out2] "=&r" (r2), [out3] "=&r" (r3)
        : [in1] "r" (a1), [in2] "r" (a2), [in3] "r" (a3),
          [in4] "r" (a4), [in5] "r" (a5), [in6] "r" (a6),
          [in7] "r" (a7), [in8] "r" (a8), [in9] "r" (a9),
          [in10] "r" (a10), [in11] "r" (a11)
        : "cc"
    );
    
    global_counter += r1 + r2 + r3;
#endif
    
    printf("11-operand test executed, counter = %d\n", global_counter);
}

/* =========================================== */
/* Complex vector reduction chain              */
/* =========================================== */
AGGRESSIVE_OPT
void complex_vector_chain(void) {
#if X86_TARGET && defined(__AVX512F__)
    /* This complex chain may be optimized into multi-operand patterns */
    __m512i v1 = _mm512_set1_epi32(global_counter);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    /* Complex sequence that might be matched as single RTL pattern */
    __m512i r1 = _mm512_add_epi32(v1, v2);
    __m512i r2 = _mm512_mullo_epi32(r1, v3);
    __m512i r3 = _mm512_slli_epi32(r2, 2);
    __m512i r4 = _mm512_sub_epi32(r3, v4);
    __m512i result = _mm512_and_si512(r4, _mm512_set1_epi32(0xFF));
    
    /* Force use of result */
    int* res_ptr = (int*)&result;
    global_counter = res_ptr[0];
#endif
}

/* =========================================== */
/* Main driver                                 */
/* =========================================== */
int main(void) {
    printf("Starting multi-operand RTL pattern test...\n");
    
    /* Execute both test functions */
    test_10_operand();
    test_11_operand();
    
    /* Additional complex pattern that might trigger during optimization */
    complex_vector_chain();
    
    printf("Final counter value: %d\n", global_counter);
    printf("Test completed successfully.\n");
    
    return global_counter == 0 ? 0 : 1;
}
