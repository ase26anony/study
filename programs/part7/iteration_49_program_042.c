/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_volatile_zero = 0;

/* Function attribute to force specific optimization */
#define FORCE_EXPANSION __attribute__((noinline, optimize("O3")))

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test function for 10-operand case using AVX-512 intrinsics */
FORCE_EXPANSION
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 expands to many operands:
       - dest
       - mask
       - idx
       - src1
       - src2
       Plus various immediates and temporaries
    */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    int64_t* res_arr = (int64_t*)&result;
    if (g_volatile_zero) {
        printf("%ld\n", res_arr[0]);
    }
}

/* Test function for 11-operand case using multiple operations */
FORCE_EXPANSION
void test_11_operand_x86(void) {
    /* Complex blend with multiple sources and masks */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0x5555;
    
    /* Chain operations that might combine into complex pattern */
    __m512i temp1 = _mm512_mask_blend_epi32(mask1, a, b);
    __m512i temp2 = _mm512_mask_blend_epi32(mask2, c, d);
    
    /* Complex permute with many operands */
    __m512i idx = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i result = _mm512_permutex2var_epi32(temp1, idx, temp2);
    
    /* Use result */
    int32_t* res_arr = (int32_t*)&result;
    if (g_volatile_zero) {
        printf("%d\n", res_arr[0]);
    }
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>

/* Test function for 10-operand case using ARM NEON */
FORCE_EXPANSION
void test_10_operand_arm(void) {
    /* Complex table lookup with multiple registers */
    uint8x16_t tab1 = vdupq_n_u8(1);
    uint8x16_t tab2 = vdupq_n_u8(2);
    uint8x16_t tab3 = vdupq_n_u8(3);
    uint8x16_t idx = vcombine_u8(
        vcreate_u8(0x0706050403020100),
        vcreate_u8(0x0F0E0D0C0B0A0908)
    );
    
    /* vqtbl3q_u8 uses 3 table registers + index + destination */
    uint8x16x3_t tables = {tab1, tab2, tab3};
    uint8x16_t result = vqtbl3q_u8(tables, idx);
    
    /* Use result */
    uint8_t* res_arr = (uint8_t*)&result;
    if (g_volatile_zero) {
        printf("%d\n", res_arr[0]);
    }
}

/* Test function for 11-operand case using inline assembly */
FORCE_EXPANSION
void test_11_operand_arm(void) {
    uint64_t out1, out2, out3;
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    uint64_t in6 = 6, in7 = 7, in8 = 8;
    
    /* Inline assembly with 11 operands:
       3 outputs + 8 inputs = 11 total operands */
    asm volatile (
        "add %[o1], %[i1], %[i2]\n\t"
        "add %[o2], %[i3], %[i4]\n\t"
        "add %[o3], %[i5], %[i6]\n\t"
        "mul %[o1], %[o1], %[i7]\n\t"
        "mul %[o2], %[o2], %[i8]\n\t"
        : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
          [i7] "r" (in7), [i8] "r" (in8)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%lu %lu %lu\n", out1, out2, out3);
    }
}

#endif /* __aarch64__ */

/* Generic fallback using inline assembly with many operands */
FORCE_EXPANSION
void test_10_operand_generic(void) {
    long out1, out2, out3, out4;
    long in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    long in6 = 6;
    
    /* 10 operands: 4 outputs + 6 inputs */
    asm volatile (
        "add %[o1], %[i1], %[i2]\n\t"
        "add %[o2], %[i3], %[i4]\n\t"
        "add %[o3], %[o1], %[i5]\n\t"
        "add %[o4], %[o2], %[i6]\n\t"
        : [o1] "=&r" (out1), [o2] "=&r" (out2),
          [o3] "=&r" (out3), [o4] "=&r" (out4)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%ld %ld %ld %ld\n", out1, out2, out3, out4);
    }
}

FORCE_EXPANSION
void test_11_operand_generic(void) {
    long out1, out2, out3, out4, out5;
    long in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    long in6 = 6;
    
    /* 11 operands: 5 outputs + 6 inputs */
    asm volatile (
        "add %[o1], %[i1], %[i2]\n\t"
        "add %[o2], %[i3], %[i4]\n\t"
        "add %[o3], %[o1], %[i5]\n\t"
        "add %[o4], %[o2], %[i6]\n\t"
        "mov %[o5], #100\n\t"
        : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3),
          [o4] "=&r" (out4), [o5] "=&r" (out5)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%ld %ld %ld %ld %ld\n", out1, out2, out3, out4, out5);
    }
}

/* Main driver function */
int main(void) {
    /* Call architecture-specific or generic test functions */
    
#ifdef __x86_64__
    test_10_operand_x86();
    test_11_operand_x86();
#elif defined(__aarch64__)
    test_10_operand_arm();
    test_11_operand_arm();
#else
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    /* Also call generic versions to increase coverage chances */
    test_10_operand_generic();
    test_11_operand_generic();
    
    return 0;
}
