/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to trigger 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 complex permute with multiple operands */
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic typically expands to many operands:
       dest, mask, idx, src1, src2 + various immediates */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
    
#elif defined(__aarch64__)
    /* ARM NEON complex operations */
    #include <arm_neon.h>
    
    /* Use multiple vector operations that might combine */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex sequence that might generate multi-operand pattern */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    
    /* Multiple table lookups - can generate complex patterns */
    uint8x8x4_t tbl = {vget_low_u8(a), vget_high_u8(a),
                       vget_low_u8(b), vget_high_u8(b)};
    uint8x8_t indices = vcreate_u8(0x0706050403020100);
    uint8x8_t tbl_result = vtbl4_u8(tbl, indices);
    
    global_counter += vget_lane_u8(tbl_result, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        /* Complex multi-operand pattern */
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* AVX-512 masked gather with complex addressing */
    #include <immintrin.h>
    
    int64_t base[64] __attribute__((aligned(64)));
    for (int i = 0; i < 64; i++) base[i] = i;
    
    __m512i vindex = _mm512_set_epi64(56, 48, 40, 32, 24, 16, 8, 0);
    __mmask8 mask = 0xAA;  /* 10101010 */
    int scale = 8;
    
    /* This gather operation has many operands:
       dest, mask, base, vindex, scale, plus various control operands */
    __m512i gathered = _mm512_mask_i64gather_epi64(
        _mm512_setzero_si512(), mask, vindex, base, scale);
    
    global_counter += _mm512_reduce_add_epi64(gathered);
    
#elif defined(__aarch64__)
    /* ARM SVE-like pattern with multiple operands */
    #include <arm_neon.h>
    
    /* Complex interleaving pattern */
    uint8x16_t v0 = vdupq_n_u8(0);
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    
    /* Multiple operations that might combine */
    uint8x16_t t1 = vaddq_u8(v0, v1);
    uint8x16_t t2 = vaddq_u8(v2, v3);
    uint8x16_t t3 = vaddq_u8(t1, t2);
    uint8x16_t t4 = vaddq_u8(t3, v4);
    
    /* Complex permute with table lookup */
    uint8x8x4_t tbl_data = {
        vget_low_u8(v0), vget_high_u8(v0),
        vget_low_u8(v1), vget_high_u8(v1)
    };
    uint8x8_t idx1 = vcreate_u8(0x0F0E0D0C0B0A0908);
    uint8x8_t idx2 = vcreate_u8(0x0706050403020100);
    
    uint8x8_t r1 = vtbl4_u8(tbl_data, idx1);
    uint8x8_t r2 = vtbl4_u8(tbl_data, idx2);
    
    global_counter += vget_lane_u8(r1, 0) + vget_lane_u8(r2, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    asm volatile (
        /* 11-operand computation */
        "mov %[res], %[a]\n\t"
        "add %[res], %[res], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test with OpenMP SIMD pragmas for pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __x86_64__
    #include <immintrin.h>
    
    /* Chain of operations that might be combined */
    __m512i v1 = _mm512_set1_epi32(global_counter);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    #pragma omp simd
    for (int i = 0; i < 16; i++) {
        /* Complex expression that might generate multi-operand pattern */
        __m512i temp = _mm512_add_epi32(v1, v2);
        temp = _mm512_mullo_epi32(temp, v3);
        temp = _mm512_sub_epi32(temp, v4);
        
        /* Use masked operation */
        __mmask16 mask = _mm512_cmpgt_epi32_mask(temp, _mm512_setzero_si512());
        temp = _mm512_mask_add_epi32(temp, mask, temp, _mm512_set1_epi32(1));
        
        v1 = temp;  /* Create dependency chain */
    }
    
    global_counter += _mm512_reduce_add_epi32(v1);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", global_counter);
    return 0;
}
