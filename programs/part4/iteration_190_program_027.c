/* Test case to cover 10 and 11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t prng_state = 123456789;

static uint64_t simple_prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function to ensure values are used and not optimized away */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Helper to initialize arrays with non-constant values */
static void init_values(uint64_t *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = simple_prng();
    }
}

/* ========== 10 OPERAND FUNCTION ========== */
uint64_t func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                          uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                          uint64_t a8, uint64_t a9) {
    uint64_t result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that can expand to many operands */
    #include <immintrin.h>
    __m512i v1 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i v2 = _mm512_set_epi64(a1, a0, a9, a8, a7, a6, a5, a4);
    __mmask8 mask = (a0 & 0xFF) | ((a1 & 0xFF) << 8);
    
    /* Complex operation with many internal operands */
    __m512i res = _mm512_mask_add_epi64(v1, mask, v1, v2);
    
    /* Extract and combine results */
    uint64_t temp[8];
    _mm512_storeu_si512(temp, res);
    for (int i = 0; i < 8; i++) {
        result += temp[i];
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE intrinsic - may expand to many operands */
    #include <arm_sve.h>
    svuint64_t sv1 = svdup_u64(a0);
    svuint64_t sv2 = svdup_u64(a1);
    svuint64_t sv3 = svdup_u64(a2);
    svuint64_t sv4 = svdup_u64(a3);
    svuint64_t sv5 = svdup_u64(a4);
    
    /* Multiple operations to increase operand count */
    svuint64_t sum1 = svadd_u64_x(svptrue_b64(), sv1, sv2);
    svuint64_t sum2 = svadd_u64_x(svptrue_b64(), sv3, sv4);
    svuint64_t sum3 = svadd_u64_x(svptrue_b64(), sum1, sum2);
    svuint64_t final = svadd_u64_x(svptrue_b64(), sum3, sv5);
    
    result = svaddv_u64(svptrue_b64(), final);
    
#else
    /* Generic inline assembly with 10 operands */
    uint64_t out0, out1, out2, out3;
    
    asm volatile(
        /* Dummy operations using all 10 input operands */
        "mov %[out0], %[in0] \n\t"
        "add %[out0], %[out0], %[in1] \n\t"
        "add %[out0], %[out0], %[in2] \n\t"
        "mov %[out1], %[in3] \n\t"
        "add %[out1], %[out1], %[in4] \n\t"
        "add %[out1], %[out1], %[in5] \n\t"
        "mov %[out2], %[in6] \n\t"
        "add %[out2], %[out2], %[in7] \n\t"
        "add %[out2], %[out2], %[in8] \n\t"
        "mov %[out3], %[in9] \n\t"
        "add %[out0], %[out0], %[out1] \n\t"
        "add %[out2], %[out2], %[out3] \n\t"
        "add %[out0], %[out0], %[out2] \n\t"
        : [out0] "=&r" (out0), [out1] "=&r" (out1),
          [out2] "=&r" (out2), [out3] "=&r" (out3)
        : [in0] "r" (a0), [in1] "r" (a1), [in2] "r" (a2),
          [in3] "r" (a3), [in4] "r" (a4), [in5] "r" (a5),
          [in6] "r" (a6), [in7] "r" (a7), [in8] "r" (a8),
          [in9] "r" (a9)
        : "cc"
    );
    
    result = out0;
#endif
    
    return result + a0 + a9; /* Ensure all inputs affect output */
}

/* ========== 11 OPERAND FUNCTION ========== */
uint64_t func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                          uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                          uint64_t a8, uint64_t a9, uint64_t a10) {
    uint64_t result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512VBMI2__)
    /* AVX-512 VBMI2 has compress/expand instructions with many operands */
    #include <immintrin.h>
    __m512i data = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i src = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
    __mmask8 mask = (a0 & 0xFF) | ((a1 & 0xFF) << 8) | ((a2 & 0xFF) << 16);
    
    /* _mm512_mask_compress_epi64 expands to many operands */
    __m512i compressed = _mm512_mask_compress_epi64(data, mask, src);
    
    uint64_t temp[8];
    _mm512_storeu_si512(temp, compressed);
    for (int i = 0; i < 8; i++) {
        result += temp[i];
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter/store with predicate may use many operands */
    #include <arm_sve.h>
    svuint64_t data = svdup_u64(a0);
    svuint64_t base = svdup_u64(a1);
    svuint64_t offsets = svdup_u64(a2);
    svbool_t pred = svcmpgt_u64(svptrue_b64(), data, svdup_u64(a3));
    
    /* Complex SVE operation chain */
    svuint64_t scaled = svmul_u64_x(pred, data, svdup_u64(a4));
    svuint64_t added = svadd_u64_x(pred, scaled, svdup_u64(a5));
    svuint64_t shifted = svlsl_u64_x(pred, added, svdup_u64(a6));
    
    result = svaddv_u64(pred, shifted);
    
#else
    /* Generic inline assembly with 11 operands */
    uint64_t out0, out1, out2, out3, out4;
    
    asm volatile(
        /* Use all 11 input operands in dummy operations */
        "mov %[out0], %[in0] \n\t"
        "add %[out0], %[out0], %[in1] \n\t"
        "add %[out0], %[out0], %[in2] \n\t"
        "mov %[out1], %[in3] \n\t"
        "add %[out1], %[out1], %[in4] \n\t"
        "add %[out1], %[out1], %[in5] \n\t"
        "mov %[out2], %[in6] \n\t"
        "add %[out2], %[out2], %[in7] \n\t"
        "add %[out2], %[out2], %[in8] \n\t"
        "mov %[out3], %[in9] \n\t"
        "mov %[out4], %[in10] \n\t"
        "add %[out3], %[out3], %[out4] \n\t"
        "add %[out0], %[out0], %[out1] \n\t"
        "add %[out2], %[out2], %[out3] \n\t"
        "add %[out0], %[out0], %[out2] \n\t"
        : [out0] "=&r" (out0), [out1] "=&r" (out1),
          [out2] "=&r" (out2), [out3] "=&r" (out3),
          [out4] "=&r" (out4)
        : [in0] "r" (a0), [in1] "r" (a1), [in2] "r" (a2),
          [in3] "r" (a3), [in4] "r" (a4), [in5] "r" (a5),
          [in6] "r" (a6), [in7] "r" (a7), [in8] "r" (a8),
          [in9] "r" (a9), [in10] "r" (a10)
        : "cc"
    );
    
    result = out0;
#endif
    
    return result + a0 + a10; /* Ensure all inputs affect output */
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    uint64_t values[12];
    uint64_t final_result = 0;
    
    /* Initialize with non-constant values */
    init_values(values, 12);
    
    /* Also use argv to add external input dependency */
    if (argc > 1) {
        values[0] += (uint64_t)argv[0];
    }
    
    /* Call 10-operand function */
    uint64_t res10 = func_10_operands(
        values[0], values[1], values[2], values[3],
        values[4], values[5], values[6], values[7],
        values[8], values[9]
    );
    
    /* Call 11-operand function */
    uint64_t res11 = func_11_operands(
        values[0], values[1], values[2], values[3],
        values[4], values[5], values[6], values[7],
        values[8], values[9], values[10]
    );
    
    /* Combine results to prevent dead code elimination */
    final_result = res10 + res11 + values[11];
    
    /* Print result to ensure side effect */
    printf("Result: %lu\n", (unsigned long)final_result);
    
    /* Use all values to prevent optimization */
    for (int i = 0; i < 12; i++) {
        use(&values[i]);
    }
    
    return (final_result > 1000000) ? 0 : 1;
}
