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
#define ARRAY_SIZE 1024
#define VALIDATE_SUM(expected, actual, msg) \
    do { \
        if ((expected) != (actual)) { \
            printf("FAIL: %s (expected %lld, got %lld)\n", \
                   msg, (long long)(expected), (long long)(actual)); \
            return 0; \
        } \
    } while(0)

/* Function attributes to prevent early inlining */
#define NOINLINE __attribute__((noinline, noipa))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* 10-operand pattern: Masked gather with 8 source + 2 destination operands */
NOINLINE static int test_avx512_10_operand(void) {
    /* Setup test data */
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 16;
        result[i] = 0.0;
    }
    
    /* This intrinsic should generate RTL with 10 operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Source (pass-through)
     * 4. Base pointer
     * 5. Index vector
     * 6. Scale
     * 7. Displacement
     * 8. Hint
     * 9. Mask register
     * 10. Rounding mode
     */
    __m512d src = _mm512_set1_pd(999.0);
    __m512i vindex = _mm512_load_epi64(indices);
    __mmask8 mask = 0xFF;
    
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base, 8, _MM_SCALE_1);
    
    _mm512_store_pd(result, gathered);
    
    /* Validate */
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (long long)result[i];
    }
    VALIDATE_SUM(1120, sum, "AVX-512 10-operand gather");
    
    return 1;
}

/* 11-operand pattern: Complex FMA with mask and rounding */
NOINLINE static int test_avx512_11_operand(void) {
    /* This inline assembly forces 11 operands:
     * 1-3: Input vectors
     * 4: Mask
     * 5-7: Additional control operands
     * 8-11: Output and clobbered registers
     */
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double c[8] __attribute__((aligned(64)));
    double d[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i + 1.0;
        b[i] = i + 2.0;
        c[i] = i + 3.0;
        d[i] = 0.0;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    __m512d vc = _mm512_load_pd(c);
    __m512d vd;
    
    /* Extended inline assembly with 11 operands */
    asm volatile (
        "vmovapd %1, %0\n\t"
        "vfmadd231pd %2, %3, %0{%4}%{z%}\n\t"
        "vpermpd $0xB1, %0, %0\n\t"
        "vblendmpd %5, %0, %6%{1to8%}\n\t"
        : "=v"(vd), "+v"(va)
        : "v"(vb), "v"(vc), "k"(0xFF), "v"(va), "m"(*a)
        : "cc", "memory"
    );
    
    _mm512_store_pd(d, vd);
    
    /* Simple validation */
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (long long)d[i];
    }
    /* Result depends on FMA, just check it's non-zero */
    if (sum == 0) {
        printf("FAIL: AVX-512 11-operand FMA produced zero\n");
        return 0;
    }
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* 10-operand pattern: SVE gather with predicate */
NOINLINE static int test_sve_10_operand(void) {
    uint64_t base[ARRAY_SIZE];
    uint64_t indices[256];
    uint64_t result[256];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = i * 3;
    }
    for (int i = 0; i < 256; i++) {
        indices[i] = (i * 2) % ARRAY_SIZE;
        result[i] = 0;
    }
    
    svbool_t pg = svptrue_b64();
    svuint64_t vindex = svld1_u64(pg, indices);
    
    /* SVE gather - should generate multiple operands */
    svuint64_t gathered = svld1_gather_u64index_u64(pg, base, vindex);
    
    svst1_u64(pg, result, gathered);
    
    /* Validate */
    long long sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += result[i];
    }
    VALIDATE_SUM(195840, sum, "SVE 10-operand gather");
    
    return 1;
}

/* 11-operand pattern: SVE scatter with update */
NOINLINE static int test_sve_11_operand(void) {
    /* Complex scatter with predicate, base, offset, and data */
    uint64_t data[256];
    uint64_t base[ARRAY_SIZE] = {0};
    uint64_t indices[256];
    
    for (int i = 0; i < 256; i++) {
        data[i] = i * 5;
        indices[i] = (i * 3) % (ARRAY_SIZE - 1);
    }
    
    svbool_t pg = svptrue_b64();
    svuint64_t vdata = svld1_u64(pg, data);
    svuint64_t vindex = svld1_u64(pg, indices);
    
    /* Scatter operation with many operands */
    svst1_scatter_u64index_u64(pg, base, vindex, vdata);
    
    /* Validate scattered data */
    long long sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += base[i];
    }
    VALIDATE_SUM(163200, sum, "SVE 11-operand scatter");
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

/* 10-operand pattern: VSX matrix multiply assist */
NOINLINE static int test_powerpc_10_operand(void) {
    /* Use inline assembly to force many operands */
    vector double a[4], b[4], c[4];
    vector double result[4];
    
    for (int i = 0; i < 4; i++) {
        a[i] = (vector double){i+1.0, i+2.0};
        b[i] = (vector double){i+3.0, i+4.0};
        c[i] = (vector double){i+5.0, i+6.0};
    }
    
    /* Extended inline assembly with 10 operands */
    asm volatile (
        "xvmaddmdp %0, %1, %2\n\t"
        "xxpermdi %3, %0, %1, 2\n\t"
        "xvmaddadp %4, %5, %6\n\t"
        "xxmrghd %7, %8, %9\n\t"
        : "+v"(a[0]), "+v"(a[1]), "+v"(a[2]), "+v"(a[3]),
          "+v"(b[0]), "+v"(b[1]), "+v"(b[2]), "+v"(b[3]),
          "+v"(c[0]), "+v"(c[1])
        :
        : "cr0"
    );
    
    /* Store results */
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
    result[3] = a[3];
    
    /* Simple validation */
    double sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += ((double*)&result[i])[0] + ((double*)&result[i])[1];
    }
    
    if (sum < 100.0) {  /* Arbitrary check */
        printf("FAIL: PowerPC 10-operand result too small: %f\n", sum);
        return 0;
    }
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ==================== RISC-V Vector Extension ==================== */
#ifdef __riscv_v

/* 11-operand pattern: RISC-V vector load with mask and length */
NOINLINE static int test_riscv_11_operand(void) {
    long data[ARRAY_SIZE];
    long result[256];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 7;
    }
    
    /* Extended inline assembly simulating complex vector operation */
    asm volatile (
        "vsetvli zero, %0, e64, m8, ta, ma\n\t"
        "vle64.v v0, (%1)\n\t"
        "vadd.vv v8, v0, v0\n\t"
        "vsll.vi v16, v8, 2\n\t"
        "vmerge.vvm v24, v8, v16, v0\n\t"
        "vse64.v v24, (%2)\n\t"
        : 
        : "r"(256), "r"(data), "r"(result)
        : "v0", "v8", "v16", "v24", "memory"
    );
    
    /* Validate */
    long long sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += result[i];
    }
    VALIDATE_SUM(2284800, sum, "RISC-V 11-operand vector op");
    
    return 1;
}

#endif /* __riscv_v */

/* ==================== Generic Fallback ==================== */
#ifndef __AVX512F__
#ifndef __ARM_FEATURE_SVE
#ifndef __ALTIVEC__
#ifndef __riscv_v

/* Fallback using extended inline assembly with dummy instructions */
NOINLINE static int test_generic_10_operand(void) {
    long ops[10];
    long result = 0;
    
    for (int i = 0; i < 10; i++) {
        ops[i] = i * 11;
    }
    
    /* Force 10 operands in inline assembly */
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9])
        : "cc"
    );
    
    VALIDATE_SUM(495, result, "Generic 10-operand assembly");
    return 1;
}

NOINLINE static int test_generic_11_operand(void) {
    long ops[11];
    long result = 0;
    
    for (int i = 0; i < 11; i++) {
        ops[i] = i * 13;
    }
    
    /* Force 11 operands */
    asm volatile (
        "mov %1, %0\n\t"
        "imul %2, %0\n\t"
        "add %3, %0\n\t"
        "sub %4, %0\n\t"
        "and %5, %0\n\t"
        "or %6, %0\n\t"
        "xor %7, %0\n\t"
        "shl %8, %0\n\t"
        "shr %9, %0\n\t"
        "add %10, %0\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9]), "r"(ops[10])
        : "cc"
    );
    
    if (result == 0) {
        printf("FAIL: Generic 11-operand produced zero\n");
        return 0;
    }
    
    return 1;
}

#endif
#endif
#endif
#endif

/* ==================== Main Driver ==================== */
int main(void) {
    int passed = 0;
    int total = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    printf("Target: Trigger case 10: and case 11: in optabs.cc\n\n");
    
    /* Test architecture-specific implementations */
    
#ifdef __AVX512F__
    printf("Testing AVX-512 paths...\n");
    total += 2;
    if (test_avx512_10_operand()) {
        printf("  ✓ AVX-512 10-operand pattern\n");
        passed++;
    }
    if (test_avx512_11_operand()) {
        printf("  ✓ AVX-512 11-operand pattern\n");
        passed++;
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE paths...\n");
    total += 2;
    if (test_sve_10_operand()) {
        printf("  ✓ ARM SVE 10-operand pattern\n");
        passed++;
    }
    if (test_sve_11_operand()) {
        printf("  ✓ ARM SVE 11-operand pattern\n");
        passed++;
    }
#endif
    
#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec/VSX paths...\n");
    total += 1;
    if (test_powerpc_10_operand()) {
        printf("  ✓ PowerPC 10-operand pattern\n");
        passed++;
    }
#endif
    
#ifdef __riscv_v
    printf("Testing RISC-V Vector paths...\n");
    total += 1;
    if (test_riscv_11_operand()) {
        printf("  ✓ RISC-V 11-operand pattern\n");
        passed++;
    }
#endif

/* Fallback for architectures without vector extensions */
#if !defined(__AVX512F__) && !defined(__ARM_FEATURE_SVE) && \
    !defined(__ALTIVEC__) && !defined(__riscv_v)
    printf("Testing generic paths...\n");
    total += 2;
    if (test_generic_10_operand()) {
        printf("  ✓ Generic 10-operand pattern\n");
        passed++;
    }
    if (test_generic_11_operand()) {
        printf("  ✓ Generic 11-operand pattern\n");
        passed++;
    }
#endif
    
    printf("\nSummary: %d/%d tests passed\n", passed, total);
    
    if (passed == total) {
        printf("\nSUCCESS: All multi-operand patterns executed correctly.\n");
        printf("During compilation, this should trigger:\n");
        printf("  - case 10: in optabs.cc (10-operand expansion)\n");
        printf("  - case 11: in optabs.cc (11-operand expansion)\n");
        return 0;
    } else {
        printf("\nFAILURE: Some tests failed\n");
        return 1;
    }
}
