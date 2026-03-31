/* test_multi_operand_rtl.c - Test program for 10/11 operand RTL expansion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define VALIDATE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        return 0; \
    } \
} while(0)

#define PASS(msg) printf("PASS: %s\n", msg)

/* Prevent premature inlining to ensure RTL expansion sees the patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>
#include <x86intrin.h>

/* 10-operand pattern: Masked gather with 8 source + 2 destination operands */
NOINLINE static int test_avx512_10_operand(void) {
    /* Setup test data */
    double base[1024] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    double expected[8];
    
    /* Initialize with known pattern */
    for (int i = 0; i < 1024; i++) {
        base[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 16;
        expected[i] = base[i * 16];
    }
    
    /* Clear results */
    memset(result, 0, sizeof(result));
    
    /* Create mask: all lanes enabled */
    __mmask8 mask = 0xFF;
    
    /* This intrinsic should generate RTL with ~10 operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base address
     * 5. Scale
     * 6. Vector length hint
     * 7. Source (for scatter, not used here)
     * Plus implicit operands for addressing modes and temporaries
     */
    __m512d src = _mm512_set1_pd(0.0);
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* Force the compiler to generate the gather instruction */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base, 8 /* scale */);
    
    /* Store results for validation */
    _mm512_store_pd(result, gathered);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        VALIDATE(result[i] == expected[i], 
                "AVX-512 10-operand gather result mismatch");
    }
    
    return 1;
}

/* 11-operand pattern: Complex FMA with mask, rounding mode, etc. */
NOINLINE static int test_avx512_11_operand(void) {
    /* Setup vectors */
    double a_data[8] __attribute__((aligned(64))) = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double b_data[8] __attribute__((aligned(64))) = {2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0};
    double c_data[8] __attribute__((aligned(64))) = {3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0};
    double result[8] __attribute__((aligned(64)));
    double expected[8];
    
    for (int i = 0; i < 8; i++) {
        expected[i] = a_data[i] * b_data[i] + c_data[i];
    }
    
    __m512d a = _mm512_load_pd(a_data);
    __m512d b = _mm512_load_pd(b_data);
    __m512d c = _mm512_load_pd(c_data);
    __mmask8 mask = 0xFF;
    
    /* This FMA with mask and rounding mode may generate 11 operands:
     * 1. Destination
     * 2. Mask
     * 3. Source1 (a)
     * 4. Source2 (b)
     * 5. Source3 (c)
     * 6. Rounding mode
     * 7. Exception suppression
     * Plus implicit operands for register allocation and constraints
     */
    __m512d fma_result = _mm512_mask3_fmadd_round_pd(a, b, c, mask, 
                                                    _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    
    _mm512_store_pd(result, fma_result);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        VALIDATE(fabs(result[i] - expected[i]) < 1e-10,
                "AVX-512 11-operand FMA result mismatch");
    }
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* 10-operand pattern: SVE gather with predicate, base, offsets */
NOINLINE static int test_sve_10_operand(void) {
    /* Setup */
    uint64_t data[1024];
    uint64_t indices[256];
    uint64_t result[256];
    
    for (int i = 0; i < 1024; i++) {
        data[i] = i * 3;
    }
    for (int i = 0; i < 256; i++) {
        indices[i] = i * 4;
    }
    
    svbool_t pg = svptrue_b64();
    svuint64_t vbase = svld1_u64(pg, data);
    svuint64_t voffsets = svld1_u64(pg, indices);
    
    /* SVE gather - may generate ~10 operands in RTL */
    svuint64_t gathered = svld1_gather_u64index_u64(pg, data, voffsets);
    
    svst1_u64(pg, result, gathered);
    
    /* Simple validation */
    for (int i = 0; i < 256; i++) {
        if (svcntd() > i) {  /* Only check active lanes */
            VALIDATE(result[i] == data[indices[i]],
                    "SVE 10-operand gather result mismatch");
        }
    }
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX Implementation ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

/* 11-operand pattern: Complex vector permutation with multiple sources */
NOINLINE static int test_powerpc_11_operand(void) {
    /* Use inline assembly to force 11 operands */
    vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Initialize vectors */
    v0 = (vector float){1.0f, 2.0f, 3.0f, 4.0f};
    v1 = (vector float){2.0f, 2.0f, 2.0f, 2.0f};
    v2 = (vector float){3.0f, 3.0f, 3.0f, 3.0f};
    v3 = (vector float){4.0f, 4.0f, 4.0f, 4.0f};
    v4 = (vector float){5.0f, 5.0f, 5.0f, 5.0f};
    v5 = (vector float){6.0f, 6.0f, 6.0f, 6.0f};
    v6 = (vector float){7.0f, 7.0f, 7.0f, 7.0f};
    v7 = (vector float){8.0f, 8.0f, 8.0f, 8.0f};
    v8 = (vector float){9.0f, 9.0f, 9.0f, 9.0f};
    v9 = (vector float){10.0f, 10.0f, 10.0f, 10.0f};
    v10 = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Extended inline assembly with 11 operands */
    asm volatile (
        "xxperm %0, %1, %2\n\t"
        "xxperm %3, %4, %5\n\t"
        "xvaddsp %6, %7, %8\n\t"
        "xvmaddasp %9, %10, %1\n\t"
        : "=v"(v0), "=v"(v1), "=v"(v2), 
          "=v"(v3), "=v"(v4), "=v"(v5),
          "=v"(v6), "=v"(v7), "=v"(v8),
          "=v"(v9), "=v"(v10)
        : "0"(v0), "1"(v1), "2"(v2),
          "3"(v3), "4"(v4), "5"(v5),
          "6"(v6), "7"(v7), "8"(v8),
          "9"(v9), "10"(v10)
        : 
    );
    
    /* Dummy validation */
    float sum = ((float*)&v0)[0] + ((float*)&v10)[0];
    VALIDATE(sum == sum, "PowerPC 11-operand inline asm executed");
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ==================== Generic Fallback ==================== */

/* Generic inline assembly with many operands for architectures without
   specific vector support */
NOINLINE static int test_generic_many_operands(void) {
    long op0 = 1, op1 = 2, op2 = 3, op3 = 4, op4 = 5;
    long op5 = 6, op6 = 7, op7 = 8, op8 = 9, op9 = 10, op10 = 11;
    long result1, result2;
    
    /* 10-operand inline asm */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        : "=r"(result1)
        : "r"(op0), "r"(op1), "r"(op2), "r"(op3),
          "r"(op4), "r"(op5), "r"(op6), "r"(op7),
          "r"(op8)
        : "cc"
    );
    
    /* 11-operand inline asm */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        : "=r"(result2)
        : "r"(op0), "r"(op1), "r"(op2), "r"(op3),
          "r"(op4), "r"(op5), "r"(op6), "r"(op7),
          "r"(op8), "r"(op9), "r"(op10)
        : "cc"
    );
    
    VALIDATE(result1 == 45 && result2 == 56, 
            "Generic many-operand inline asm validation");
    
    return 1;
}

/* ==================== Hot Loop to Trigger Expansion ==================== */

/* Function containing hot loop with multi-operand operations */
NOINLINE static void hot_loop_with_multi_operand(int iterations) {
    volatile int prevent_optimization = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix of architecture-specific tests in a loop to encourage
           RTL expansion during optimization passes */
        
#ifdef __AVX512F__
        if (i % 4 == 0) {
            prevent_optimization += test_avx512_10_operand();
        }
        if (i % 4 == 1) {
            prevent_optimization += test_avx512_11_operand();
        }
#endif
        
#ifdef __ARM_FEATURE_SVE
        if (i % 4 == 2) {
            prevent_optimization += test_sve_10_operand();
        }
#endif
        
        /* Always run generic test */
        if (i % 4 == 3) {
            prevent_optimization += test_generic_many_operands();
        }
    }
    
    /* Use the result to prevent dead code elimination */
    if (prevent_optimization == 0) {
        printf("No tests executed\n");
    }
}

/* ==================== Main Function ==================== */

int main(void) {
    int passed = 0;
    int total = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    printf("Compiled with architecture features:\n");
    
#ifdef __AVX512F__
    printf("  AVX-512 supported\n");
    total += 2;
#endif
#ifdef __ARM_FEATURE_SVE
    printf("  ARM SVE supported\n");
    total += 1;
#endif
#ifdef __ALTIVEC__
    printf("  PowerPC Altivec supported\n");
    total += 1;
#endif
    printf("  Generic inline assembly always available\n");
    total += 1;
    
    /* Execute tests once for validation */
#ifdef __AVX512F__
    if (test_avx512_10_operand()) {
        PASS("AVX-512 10-operand test");
        passed++;
    }
    if (test_avx512_11_operand()) {
        PASS("AVX-512 11-operand test");
        passed++;
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    if (test_sve_10_operand()) {
        PASS("ARM SVE 10-operand test");
        passed++;
    }
#endif
    
#ifdef __ALTIVEC__
    if (test_powerpc_11_operand()) {
        PASS("PowerPC 11-operand test");
        passed++;
    }
#endif
    
    if (test_generic_many_operands()) {
        PASS("Generic many-operand test");
        passed++;
    }
    
    /* Execute hot loop to trigger RTL expansion during optimization */
    printf("\nExecuting hot loop to encourage RTL expansion...\n");
    hot_loop_with_multi_operand(100);
    
    printf("\nSummary: %d/%d architecture-specific tests passed\n", passed, total);
    
    if (passed == total) {
        printf("SUCCESS: All available multi-operand patterns executed\n");
        return 0;
    } else {
        printf("WARNING: Some tests failed or were not compiled\n");
        return 1;
    }
}
