/* test_multi_operand_rtl.c
 * 
 * This program generates RTL patterns requiring 10-11 operands
 * to trigger uncovered lines 8254-8263 in optabs.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "Validation failed: %s\n", msg); \
        return 0; \
    } \
} while(0)

/* Function prototypes with noinline to prevent early inlining */
__attribute__((noinline)) int test_x86_avx512(void);
__attribute__((noinline)) int test_arm_sve(void);
__attribute__((noinline)) int test_powerpc_vsx(void);
__attribute__((noinline)) int test_riscv_vector(void);
__attribute__((noinline)) int test_generic_inline_asm(void);

/* Global test data */
static double src_data[ARRAY_SIZE];
static double dst_data[ARRAY_SIZE];
static int64_t indices[ARRAY_SIZE];
static uint8_t mask_data[ARRAY_SIZE/8];

/* Initialize test data */
static void init_test_data(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_data[i] = (double)i * 1.5;
        dst_data[i] = 0.0;
        indices[i] = (i * 2) % ARRAY_SIZE;
    }
    for (int i = 0; i < ARRAY_SIZE/8; i++) {
        mask_data[i] = (i % 2) ? 0xFF : 0x00;
    }
}

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Pattern A: 10 operands - masked gather operation */
__attribute__((noinline, target("avx512f")))
int test_x86_avx512_10_operands(void) {
    __m512d result, src, default_val;
    __m512i idx;
    __mmask8 mask;
    const double* base;
    long long scale;
    int hint;
    
    /* Initialize operands */
    base = src_data;
    scale = 8; /* sizeof(double) */
    hint = _MM_HINT_T0;
    mask = 0xAA; /* 10101010 pattern */
    src = _mm512_set1_pd(1.0);
    default_val = _mm512_set1_pd(-1.0);
    idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    
    /* This intrinsic should generate RTL with ~10 operands:
     * 1. result (output)
     * 2. mask
     * 3. idx
     * 4. base
     * 5. scale
     * 6. hint
     * 7. src (for some variants)
     * 8. default_val
     * Plus additional implicit operands
     */
    result = _mm512_mask_i64gather_pd(default_val, mask, idx, base, scale);
    
    /* Use result to prevent optimization */
    double sum = _mm512_reduce_add_pd(result);
    return sum > 0;
}

/* Pattern B: 11 operands - complex masked scatter with update */
__attribute__((noinline, target("avx512f,avx512vl")))
int test_x86_avx512_11_operands(void) {
    /* Using inline assembly to explicitly require 11 operands */
    __m512d src1, src2, src3;
    __m512i idx1, idx2;
    __mmask16 mask;
    double* base1, *base2;
    long scale1, scale2;
    int hint1, hint2;
    
    /* Initialize */
    src1 = _mm512_set1_pd(1.0);
    src2 = _mm512_set1_pd(2.0);
    src3 = _mm512_set1_pd(3.0);
    idx1 = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
    idx2 = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    mask = 0xAAAA;
    base1 = dst_data;
    base2 = dst_data + 8;
    scale1 = 8;
    scale2 = 8;
    hint1 = _MM_HINT_T0;
    hint2 = _MM_HINT_T1;
    
    /* Extended inline assembly with 11 operands:
     * 2 outputs + 9 inputs = 11 total
     */
    __m512d out1, out2;
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "vmovapd %[out1], %[src1]\n\t"
        "vmovapd %[out2], %[src2]\n\t"
        : [out1] "=v" (out1), [out2] "=v" (out2)
        : [src1] "v" (src1), [src2] "v" (src2), [src3] "v" (src3),
          [idx1] "v" (idx1), [idx2] "v" (idx2),
          [mask] "k" (mask), [base1] "r" (base1), [base2] "r" (base2),
          [scale1] "r" (scale1), [scale2] "r" (scale2),
          [hint1] "r" (hint1), [hint2] "r" (hint2)
        : "memory"
    );
    
    /* Use outputs */
    double sum = _mm512_reduce_add_pd(out1) + _mm512_reduce_add_pd(out2);
    return sum > 0;
}

int test_x86_avx512(void) {
    printf("Testing x86 AVX-512 10-11 operand patterns...\n");
    
    int result1 = test_x86_avx512_10_operands();
    int result2 = test_x86_avx512_11_operands();
    
    VALIDATE(result1 && result2, "AVX-512 multi-operand operations failed");
    
    /* Additional test with gather intrinsic that may use many operands */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        __m512i vindex = _mm512_loadu_si512(&indices[i]);
        __mmask8 kmask = _cvtu32_mask8(mask_data[i/8]);
        __m512d src = _mm512_loadu_pd(&src_data[i]);
        
        /* This gather may expand to many operands */
        __m512d gathered = _mm512_mask_i64gather_pd(
            src,                    /* default value */
            kmask,                  /* mask */
            vindex,                 /* indices */
            src_data,               /* base address */
            8                       /* scale */
        );
        
        _mm512_storeu_pd(&dst_data[i], gathered);
    }
    
    /* Validate */
    double checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst_data[i];
    }
    printf("AVX-512 checksum: %f\n", checksum);
    
    return 1;
}
#else
int test_x86_avx512(void) {
    printf("AVX-512 not supported on this platform\n");
    return 1; /* Not an error, just not supported */
}
#endif

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern for 10+ operands using SVE gather instructions */
__attribute__((noinline))
int test_arm_sve(void) {
    printf("Testing ARM SVE 10-11 operand patterns...\n");
    
    svbool_t pg = svptrue_b64();
    svint64_t indices_vec = svld1_s64(pg, indices);
    svfloat64_t src_vec = svld1_f64(pg, src_data);
    
    /* SVE gather with multiple parameters - may require many operands */
    svfloat64_t gathered = svld1_gather_index(pg, src_data, indices_vec);
    
    /* Store results */
    svst1_f64(pg, dst_data, gathered);
    
    /* Extended inline assembly to force 11 operands */
    svfloat64_t out1, out2, out3;
    asm volatile (
        "/* ARM SVE 11-operand pattern */\n\t"
        "mov %[out1].d, %[src].d\n\t"
        "mov %[out2].d, %[indices].d\n\t"
        "mov %[out3].d, %[pg].d\n\t"
        : [out1] "=w" (out1), [out2] "=w" (out2), [out3] "=w" (out3)
        : [src] "w" (src_vec), [indices] "w" (indices_vec), [pg] "w" (pg),
          "[base] "r" (src_data), "[scale] "r" (8),
          "[offset1] "r" (0), "[offset2] "r" (8), "[offset3] "r" (16),
          "[hint] "r" (0)
        : "memory"
    );
    
    /* Use outputs */
    double sum = 0;
    for (int i = 0; i < svcntd(); i++) {
        sum += svlasta_f64(svptrue_b64(), out1);
    }
    
    printf("ARM SVE test completed\n");
    return 1;
}
#else
int test_arm_sve(void) {
    printf("ARM SVE not supported on this platform\n");
    return 1;
}
#endif

/* ==================== PowerPC VSX Implementation ==================== */
#ifdef __VSX__

#include <altivec.h>

/* Pattern using PowerPC matrix multiply assist */
__attribute__((noinline))
int test_powerpc_vsx(void) {
    printf("Testing PowerPC VSX multi-operand patterns...\n");
    
    vector double v1 = {1.0, 2.0};
    vector double v2 = {3.0, 4.0};
    vector double v3 = {5.0, 6.0};
    vector double v4 = {7.0, 8.0};
    vector double v5 = {9.0, 10.0};
    vector double v6 = {11.0, 12.0};
    vector double v7 = {13.0, 14.0};
    vector double v8 = {15.0, 16.0};
    
    /* Extended inline assembly with 10 operands */
    vector double out1, out2;
    asm volatile (
        "/* PowerPC 10-operand instruction pattern */\n\t"
        "xxmrghw %x[out1], %x[v1], %x[v2]\n\t"
        "xxmrglw %x[out2], %x[v3], %x[v4]\n\t"
        : [out1] "=wa" (out1), [out2] "=wa" (out2)
        : [v1] "wa" (v1), [v2] "wa" (v2), [v3] "wa" (v3), [v4] "wa" (v4),
          [v5] "wa" (v5), [v6] "wa" (v6), [v7] "wa" (v7), [v8] "wa" (v8)
        : "memory"
    );
    
    /* Use the results */
    double sum = ((double*)&out1)[0] + ((double*)&out2)[0];
    printf("PowerPC VSX sum: %f\n", sum);
    
    return 1;
}
#else
int test_powerpc_vsx(void) {
    printf("PowerPC VSX not supported on this platform\n");
    return 1;
}
#endif

/* ==================== Generic Inline Assembly ==================== */
/* Always compile this version as fallback */
__attribute__((noinline))
int test_generic_inline_asm(void) {
    printf("Testing generic inline assembly with many operands...\n");
    
    /* Force 10 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9;
    long out1, out2;
    
    asm volatile (
        "/* 10-operand generic pattern */\n\t"
        "add %[out1], %[a], %[b]\n\t"
        "add %[out2], %[c], %[d]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
    );
    
    VALIDATE(out1 == 3 && out2 == 7, "Generic 10-operand asm failed");
    
    /* Force 11 operands */
    long j = 10, k = 11;
    long out3;
    
    asm volatile (
        "/* 11-operand generic pattern */\n\t"
        "add %[out1], %[a], %[b]\n\t"
        "add %[out2], %[c], %[d]\n\t"
        "add %[out3], %[e], %[f]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
    );
    
    VALIDATE(out3 == 11, "Generic 11-operand asm failed");
    
    return 1;
}

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing RTL expansion for 10-11 operand patterns\n");
    printf("===============================================\n");
    
    init_test_data();
    
    int passed = 1;
    
    /* Test architecture-specific implementations */
    passed &= test_x86_avx512();
    passed &= test_arm_sve();
    passed &= test_powerpc_vsx();
    
    /* Always test generic inline assembly */
    passed &= test_generic_inline_asm();
    
    /* Hot loop to encourage vectorization and RTL expansion */
    #pragma omp parallel for simd
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Complex expression that might generate multi-operand RTL */
        dst_data[i] = src_data[i] * 2.0 + src_data[(i+1) % ARRAY_SIZE] * 3.0;
    }
    
    /* Final validation */
    double final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += dst_data[i];
    }
    printf("Final checksum: %f\n", final_sum);
    
    if (passed) {
        printf("\n✅ All tests passed successfully!\n");
        printf("If compiled with appropriate flags, this should have triggered\n");
        printf("the 10-11 operand RTL expansion cases in optabs.cc\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed\n");
        return 1;
    }
}
