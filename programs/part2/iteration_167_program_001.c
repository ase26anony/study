/* Test program to trigger 10-11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation helpers */
static int tests_passed = 0;
static int tests_run = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        tests_run++; \
        if (!(cond)) { \
            printf("FAIL: %s\n", msg); \
            return 0; \
        } else { \
            tests_passed++; \
        } \
    } while (0)

/* Prevent inlining to ensure RTL expansion happens */
#define NOINLINE __attribute__((noinline, optimize("O3")))

/* ==================== x86 AVX-512 Paths ==================== */
#ifdef __AVX512F__

#include <immintrin.h>
#include <x86intrin.h>

/* Pattern A: 10 operands - masked gather with multiple sources */
NOINLINE static int test_avx512_gather_10_operands(void) {
    printf("Testing AVX-512 10-operand gather...\n");
    
    /* Initialize test data */
    double base[64] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        base[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 8;
    }
    
    /* This gather intrinsic typically expands to many operands:
       - Destination vector
       - Mask
       - Base pointer
       - Index vector
       - Scale
       - Displacement
       - Source (for scatter, not used here)
       - Mask register
       - Various control operands
    */
    __m512d src = _mm512_set1_pd(0.0);
    __mmask8 mask = 0xFF;
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* Force multiple operands through complex addressing */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base, _MM_SCALE_8, 
                                               _MM_HINT_NONE);
    
    _mm512_store_pd(result, gathered);
    
    /* Validate results */
    for (int i = 0; i < 8; i++) {
        double expected = base[indices[i]];
        TEST_ASSERT(result[i] == expected, 
                   "AVX-512 gather result mismatch");
    }
    
    return 1;
}

/* Pattern B: 11 operands - complex masked operation with multiple destinations */
NOINLINE static int test_avx512_fmadd_11_operands(void) {
    printf("Testing AVX-512 11-operand fused multiply-add...\n");
    
    /* Create data that forces many operands */
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __mmask8 mask = 0x0F; /* Only lower 4 elements */
    
    /* Use inline assembly to force exactly 11 operands */
    __m512d result;
    
    /* Extended asm with 11 operands:
       1 output operand + 10 input operands = 11 total */
    asm volatile (
        /* Dummy instruction pattern that uses many registers */
        "vmovapd %[a], %%zmm0\n\t"
        "vmovapd %[b], %%zmm1\n\t"
        "vmovapd %[c], %%zmm2\n\t"
        "kmovw   %[mask], %%k1\n\t"
        "vfmadd132pd %%zmm1, %%zmm2, %%zmm0 %{%%k1%}\n\t"
        "vmovapd %%zmm0, %[result]\n\t"
        : [result] "=v" (result)
        : [a] "v" (a),
          [b] "v" (b),
          [c] "v" (c),
          [mask] "r" (mask),
          "m" (*(const double(*)[8])&a),  /* Force memory operand */
          "m" (*(const double(*)[8])&b),
          "m" (*(const double(*)[8])&c),
          "i" (8),                         /* Constant operand */
          "i" (3),
          "i" (4)                          /* Total: 11 operands */
        : "zmm0", "zmm1", "zmm2", "k1", "memory"
    );
    
    /* Validate */
    double res[8];
    _mm512_store_pd(res, result);
    
    for (int i = 0; i < 4; i++) {
        double expected = (i + 1) * 2.0 + 3.0;
        TEST_ASSERT(res[i] == expected, 
                   "AVX-512 FMA result mismatch (masked elements)");
    }
    
    for (int i = 4; i < 8; i++) {
        double expected = (i + 1); /* Unchanged due to mask */
        TEST_ASSERT(res[i] == expected, 
                   "AVX-512 FMA result mismatch (unmasked elements)");
    }
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Paths ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands - SVE gather with predicate */
NOINLINE static int test_arm_sve_gather_10_operands(void) {
    printf("Testing ARM SVE 10-operand gather...\n");
    
    uint64_t base[64];
    uint64_t indices[16];
    uint64_t result[16];
    
    for (int i = 0; i < 64; i++) {
        base[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        indices[i] = i * 4;
    }
    
    /* SVE gather operations can require many operands:
       - Predicate
       - Base pointer
       - Index vector
       - Scale
       - Various control operands
    */
    svbool_t pg = svptrue_b64();
    svuint64_t vindex = svld1_u64(pg, indices);
    
    /* This should expand to multiple operands */
    svuint64_t gathered = svld1_gather_u64index_u64(pg, base, vindex);
    
    svst1_u64(pg, result, gathered);
    
    /* Validate */
    for (int i = 0; i < 16; i++) {
        uint64_t expected = base[indices[i]];
        TEST_ASSERT(result[i] == expected, 
                   "ARM SVE gather result mismatch");
    }
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX Paths ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

/* Pattern B: 11 operands - Complex vector permutation */
NOINLINE static int test_powerpc_11_operands(void) {
    printf("Testing PowerPC 11-operand vector operations...\n");
    
    /* Use inline assembly to force 11 operands */
    vector float v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    vector float v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    vector float v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    vector float result;
    
    /* Extended asm with 11 operands */
    asm volatile (
        "xxmrghw %x[out], %x[a], %x[b]\n\t"
        "xxmrglw %x[tmp], %x[c], %x[d]\n\t"
        "xxpermdi %x[out], %x[out], %x[tmp], 2\n\t"
        : [out] "=wa" (result),
          [tmp] "=&wa" (v1)  /* Clobber v1 as temp */
        : [a] "wa" (v1),
          [b] "wa" (v2),
          [c] "wa" (v3),
          [d] "wa" (v4),
          "r" (0),           /* Constant operands */
          "r" (1),
          "r" (2),
          "r" (3),
          "r" (4),           /* Total: 11 operands */
          "m" (*(const float(*)[4])&v2)  /* Memory operand */
        : "vs0", "vs1", "vs2", "vs3"
    );
    
    /* Simple validation */
    float res[4];
    memcpy(res, &result, sizeof(res));
    
    TEST_ASSERT(res[0] > 0.0f, "PowerPC vector result validation");
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ==================== RISC-V Vector Paths ==================== */
#ifdef __riscv_v

/* Pattern A: 10 operands - RISC-V vector load with mask */
NOINLINE static int test_riscv_vector_10_operands(void) {
    printf("Testing RISC-V Vector 10-operand load...\n");
    
    /* Use inline assembly to simulate complex vector operation */
    long array[32];
    long result[32];
    
    for (int i = 0; i < 32; i++) {
        array[i] = i * 5;
    }
    
    /* Extended asm with 10 operands */
    asm volatile (
        "vsetvli zero, %[vl], e64, m8, ta, ma\n\t"
        "vle64.v v0, (%[src])\n\t"
        "vse64.v v0, (%[dst])\n\t"
        : 
        : [src] "r" (array),
          [dst] "r" (result),
          [vl] "r" (32),
          "m" (*(const long(*)[32])array),  /* Memory operand */
          "f" (0.0),                         /* Floating constant */
          "f" (1.0),
          "i" (8),                           /* Integer constants */
          "i" (64),
          "i" (1),
          "i" (2)                            /* Total: 10 operands */
        : "v0", "v8", "v16", "memory"
    );
    
    /* Validate */
    for (int i = 0; i < 32; i++) {
        TEST_ASSERT(result[i] == array[i], 
                   "RISC-V vector load/store mismatch");
    }
    
    return 1;
}

#endif /* __riscv_v */

/* ==================== Generic Fallback ==================== */
/* Fallback using pure inline assembly with many operands */
NOINLINE static int test_generic_many_operands(void) {
    printf("Testing generic many-operand inline assembly...\n");
    
    /* Force 11 operands with generic inline assembly */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result;
    
    asm volatile (
        /* Dummy multi-operand pattern */
        "mov %[a], %[tmp]\n\t"
        "add %[b], %[tmp]\n\t"
        "add %[c], %[tmp]\n\t"
        "add %[d], %[tmp]\n\t"
        "add %[e], %[tmp]\n\t"
        "add %[f], %[tmp]\n\t"
        "add %[g], %[tmp]\n\t"
        "add %[h], %[tmp]\n\t"
        "add %[i], %[tmp]\n\t"
        "mov %[tmp], %[out]\n\t"
        : [out] "=r" (result),
          [tmp] "=&r" (a)  /* Use a as temporary */
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f),
          [g] "r" (g),
          [h] "r" (h),
          [i] "r" (i),
          [j] "r" (j)      /* Total: 11 operands */
        : "cc"
    );
    
    TEST_ASSERT(result == 46, "Generic many-operand test failed");
    return 1;
}

/* ==================== Main Function ==================== */
int main(void) {
    printf("=== Testing 10-11 Operand RTL Expansion ===\n\n");
    
    int any_tests_run = 0;
    
    /* Run architecture-specific tests */
#ifdef __AVX512F__
    any_tests_run = 1;
    test_avx512_gather_10_operands();
    test_avx512_fmadd_11_operands();
    printf("AVX-512 paths completed\n\n");
#endif
    
#ifdef __ARM_FEATURE_SVE
    any_tests_run = 1;
    test_arm_sve_gather_10_operands();
    printf("ARM SVE paths completed\n\n");
#endif
    
#ifdef __ALTIVEC__
    any_tests_run = 1;
    test_powerpc_11_operands();
    printf("PowerPC paths completed\n\n");
#endif
    
#ifdef __riscv_v
    any_tests_run = 1;
    test_riscv_vector_10_operands();
    printf("RISC-V Vector paths completed\n\n");
#endif
    
    /* Always run generic test */
    test_generic_many_operands();
    printf("Generic paths completed\n\n");
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_run == tests_passed) {
        printf("\nSUCCESS: All tests passed!\n");
        printf("RTL expander should have encountered 10-11 operand cases.\n");
        return 0;
    } else {
        printf("\nFAILURE: Some tests failed\n");
        return 1;
    }
}
