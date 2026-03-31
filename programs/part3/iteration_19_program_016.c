/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for instructions
 * with 10 or 11 operands, covering uncovered lines in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Check the generated .expand file for multi-operand RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure local expansion */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE static uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, 
                                                  uint64_t c, uint64_t d,
                                                  uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h,
                                                  uint64_t i, uint64_t j) {
    /* Complex expression that GCC might break into many operations */
    uint64_t result = 0;
    
    /* Multi-step calculation that could use many temporaries */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Chain of operations that might combine into multi-operand pattern */
    result = t1 + t2;
    result = result * t3;
    result = result + t4;
    result = result - t5;
    
    /* Additional operations to increase operand count */
    result = (result << 5) | (result >> 59);  /* rotate */
    result = result ^ (a + b + c + d + e);
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                                          v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si t1 = a + b;
    v4si t2 = c * d;
    v4si t3 = e & f;
    v4si t4 = g | a;
    v4si t5 = t1 << 2;
    
    /* Multi-step vector operation */
    v4si result = (t1 * t2) + (t3 & t4) - t5;
    
    /* Element-wise operations */
    result[0] = result[0] * b[0] + c[0];
    result[1] = result[1] * b[1] + c[1];
    result[2] = result[2] * b[2] + c[2];
    result[3] = result[3] * b[3] + c[3];
    
    return result;
}

#ifdef __x86_64__
/* x86-specific inline assembly with many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h,
                                               uint64_t i, uint64_t j) {
    uint64_t result1, result2;
    
    /* Extended inline assembly with 10 operands */
    asm volatile (
        /* Complex multi-operand operation simulation */
        "mov %[a], %%rax\n\t"
        "imul %[b], %%rax\n\t"
        "add %[c], %%rax\n\t"
        "sub %[d], %%rax\n\t"
        "xor %[e], %%rax\n\t"
        "or %[f], %%rax\n\t"
        "and %[g], %%rax\n\t"
        "shl $3, %%rax\n\t"
        "add %[h], %%rax\n\t"
        "sub %[i], %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        /* Another operation chain */
        "mov %[j], %%rbx\n\t"
        "lea (%%rbx, %%rbx, 2), %%rbx\n\t"
        "add %%rax, %%rbx\n\t"
        "mov %%rbx, %[out2]"
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rbx", "cc"
    );
    
    return result1 + result2;
}

#include <immintrin.h>
/* AVX-512 operations that might use many operands */
NOINLINE static __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                             __m512i d, __m512i e, __m512i f) {
    /* Complex AVX-512 expression */
    __m512i t1 = _mm512_add_epi64(a, b);
    __m512i t2 = _mm512_mullo_epi64(c, d);
    __m512i t3 = _mm512_and_si512(e, f);
    __m512i t4 = _mm512_or_si512(t1, t2);
    __m512i t5 = _mm512_xor_si512(t3, t4);
    
    /* Blend operation with many operands */
    __mmask8 mask = 0xAA;  /* 10101010 */
    __m512i result = _mm512_mask_blend_epi64(mask, t1, t2);
    result = _mm512_add_epi64(result, t5);
    
    return result;
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>
/* ARM-specific NEON operations */
NOINLINE static uint64x2_t aarch64_multi_operand(uint64x2_t a, uint64x2_t b,
                                                 uint64x2_t c, uint64x2_t d,
                                                 uint64x2_t e, uint64x2_t f,
                                                 uint64x2_t g, uint64x2_t h) {
    /* Complex NEON expression chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vmulq_u64(c, d);
    uint64x2_t t3 = vandq_u64(e, f);
    uint64x2_t t4 = vorrq_u64(g, h);
    uint64x2_t t5 = veorq_u64(t1, t2);
    
    /* Multiple operations that might combine */
    uint64x2_t result = vaddq_u64(t3, t4);
    result = vmulq_u64(result, t5);
    
    /* Lane operations that might use many operands */
    result = vsetq_lane_u64(vgetq_lane_u64(a, 0) + vgetq_lane_u64(b, 1), result, 0);
    result = vsetq_lane_u64(vgetq_lane_u64(c, 1) * vgetq_lane_u64(d, 0), result, 1);
    
    return result;
}
#endif

/* Multi-precision arithmetic that often expands to many RTL operands */
NOINLINE static void multi_precision_calc(uint64_t *result,
                                          const uint64_t *a,
                                          const uint64_t *b,
                                          int size) {
    /* Multi-precision addition */
    uint64_t carry = 0;
    for (int i = 0; i < size; i++) {
        uint64_t sum = a[i] + b[i] + carry;
        carry = (sum < a[i]) || (carry && sum == a[i]);
        result[i] = sum;
    }
    
    /* Additional complex operations */
    for (int i = 0; i < size - 1; i++) {
        uint64_t hi, lo;
        /* Simulate 128-bit multiplication using 64-bit parts */
        __uint128_t product = (__uint128_t)result[i] * (__uint128_t)result[i+1];
        hi = (uint64_t)(product >> 64);
        lo = (uint64_t)product;
        result[i] = lo;
        if (i + 1 < size) {
            result[i+1] = hi;
        }
    }
}

/* Main test function */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test data */
    uint64_t values[10];
    for (int i = 0; i < 10; i++) {
        values[i] = (uint64_t)argc + i * 7;
    }
    
    /* Test different code paths based on arguments */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        result = multi_operand_arithmetic(values[0], values[1], values[2],
                                         values[3], values[4], values[5],
                                         values[6], values[7], values[8],
                                         values[9]);
        
        /* Vector operations */
        v4si vec_a = {1, 2, 3, 4};
        v4si vec_b = {5, 6, 7, 8};
        v4si vec_c = {9, 10, 11, 12};
        v4si vec_d = {13, 14, 15, 16};
        v4si vec_e = {17, 18, 19, 20};
        v4si vec_f = {21, 22, 23, 24};
        v4si vec_g = {25, 26, 27, 28};
        
        v4si vec_result = vector_multi_operand(vec_a, vec_b, vec_c, vec_d,
                                              vec_e, vec_f, vec_g);
        result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    } else {
        /* Path 2: Architecture-specific operations */
#ifdef __x86_64__
        result = x86_multi_operand_asm(values[0], values[1], values[2],
                                      values[3], values[4], values[5],
                                      values[6], values[7], values[8],
                                      values[9]);
#elif defined(__aarch64__)
        uint64x2_t neon_a = {values[0], values[1]};
        uint64x2_t neon_b = {values[2], values[3]};
        uint64x2_t neon_c = {values[4], values[5]};
        uint64x2_t neon_d = {values[6], values[7]};
        uint64x2_t neon_e = {values[8], values[9]};
        uint64x2_t neon_f = {values[0], values[1]};
        uint64x2_t neon_g = {values[2], values[3]};
        uint64x2_t neon_h = {values[4], values[5]};
        
        uint64x2_t neon_result = aarch64_multi_operand(neon_a, neon_b, neon_c,
                                                      neon_d, neon_e, neon_f,
                                                      neon_g, neon_h);
        result = vgetq_lane_u64(neon_result, 0) + vgetq_lane_u64(neon_result, 1);
#endif
    }
    
    /* Multi-precision arithmetic test */
    uint64_t mp_a[4] = {0xFFFFFFFFFFFFFFFF, 0x123456789ABCDEF0, 
                        0xFEDCBA9876543210, 0x1111111111111111};
    uint64_t mp_b[4] = {0x0000000000000001, 0xFEDCBA9876543210,
                        0x123456789ABCDEF0, 0x2222222222222222};
    uint64_t mp_result[4];
    
    multi_precision_calc(mp_result, mp_a, mp_b, 4);
    result += mp_result[0] + mp_result[1] + mp_result[2] + mp_result[3];
    
    /* Loop with varying data to increase coverage */
    for (int i = 0; i < (argc % 10); i++) {
        values[i % 10] += result;
        result = multi_operand_arithmetic(values[0], values[1], values[2],
                                         values[3], values[4], values[5],
                                         values[6], values[7], values[8],
                                         values[9]);
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return 0;
}
