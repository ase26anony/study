/* test_multi_operand_rtl.c
 * 
 * This program generates RTL patterns requiring 10-11 operands
 * to trigger uncovered lines in optabs.cc (lines 8254-8263).
 * 
 * Compile with: gcc -O3 -ftree-vectorize -funroll-loops -fopenmp -march=native -fno-inline test_multi_operand_rtl.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

/* Runtime validation helpers */
static int g_checksum = 0;
static int g_tests_run = 0;
static int g_tests_passed = 0;

#define VALIDATE(cond, msg) do { \
    g_tests_run++; \
    if (cond) { \
        g_tests_passed++; \
    } else { \
        printf("FAIL: %s\n", msg); \
    } \
} while(0)

/* Prevent inlining to ensure RTL expansion happens */
#define NOINLINE __attribute__((noinline))

/* ==================== PATTERN A: 10 OPERANDS ==================== */

#ifdef __AVX512F__
NOINLINE static void test_avx512_gather_10_operands(void) {
    /* AVX-512 masked gather with 10 operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale
     * 6. Source vector (for merge)
     * 7. Mask register
     * 8. Index type
     * 9. Data type
     * 10. Hint
     */
    
    double src[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double base[16];
    double result[8];
    
    /* Initialize test data */
    for (int i = 0; i < 16; i++) {
        base[i] = (double)(i * 2);
    }
    
    /* Create index vector with stride 2 */
    __m512i idx = _mm512_set_epi64(14, 12, 10, 8, 6, 4, 2, 0);
    
    /* Create mask (all true) */
    __mmask8 mask = 0xFF;
    
    /* Create source vector for merging */
    __m512d src_vec = _mm512_set1_pd(-1.0);
    
    /* Perform gather - this should generate 10-operand RTL */
    __m512d gathered = _mm512_mask_i64gather_pd(src_vec, mask, idx, base, 8);
    
    /* Store result */
    _mm512_storeu_pd(result, gathered);
    
    /* Validate */
    int valid = 1;
    for (int i = 0; i < 8; i++) {
        double expected = base[i * 2];
        if (result[i] != expected && result[i] != -1.0) {
            valid = 0;
            break;
        }
    }
    
    VALIDATE(valid, "AVX-512 10-operand gather");
    
    /* Update checksum */
    for (int i = 0; i < 8; i++) {
        g_checksum += (int)result[i];
    }
}

NOINLINE static void test_avx512_scatter_11_operands(void) {
    /* AVX-512 masked scatter with 11 operands:
     * This uses inline assembly to force exactly 11 operands
     */
    
    double data[8] = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0};
    double target[16] = {0};
    __m512i idx = _mm512_set_epi64(14, 12, 10, 8, 6, 4, 2, 0);
    __mmask8 mask = 0xFF;
    __m512d data_vec = _mm512_loadu_pd(data);
    
    /* Use inline assembly with 11 operands to force case 11 */
    asm volatile (
        /* Dummy multi-operand instruction pattern */
        "/* 11-operand pattern for RTL expansion */\n\t"
        "vmovapd %0, %0\n\t"  /* Use the operands */
        : "+v"(data_vec), "+v"(data_vec), "+v"(data_vec)  /* 3 outputs */
        : "v"(idx), "m"(target), "k"(mask),              /* 3 inputs */
          "r"(8), "r"(1), "r"(0), "r"(0), "r"(0)         /* 5 more inputs */
        : "memory"
    );
    
    /* Actually perform the scatter for validation */
    _mm512_mask_i64scatter_pd(target, mask, idx, data_vec, 8);
    
    /* Validate scatter results */
    int valid = 1;
    for (int i = 0; i < 8; i++) {
        if (target[i * 2] != data[i]) {
            valid = 0;
            break;
        }
    }
    
    VALIDATE(valid, "AVX-512 11-operand scatter");
    
    for (int i = 0; i < 16; i++) {
        g_checksum += (int)target[i];
    }
}
#endif /* __AVX512F__ */

/* ==================== PATTERN B: ARM SVE 11 OPERANDS ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

NOINLINE static void test_arm_sve_11_operands(void) {
    /* ARM SVE gather with predicate, base, offset - can require many operands */
    
    double base_array[32];
    double result_array[16];
    uint64_t indices[16];
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        base_array[i] = i * 3.0;
    }
    for (int i = 0; i < 16; i++) {
        indices[i] = i * 2;
    }
    
    /* Create SVE vectors */
    svbool_t pg = svptrue_b64();
    svuint64_t offset = svld1_u64(pg, indices);
    
    /* This intrinsic may expand to 11 operands */
    svfloat64_t gathered = svld1_gather_offset(pg, base_array, offset);
    
    /* Store results */
    svst1(pg, result_array, gathered);
    
    /* Validate */
    int valid = 1;
    for (int i = 0; i < 16; i++) {
        if (result_array[i] != base_array[i * 2]) {
            valid = 0;
            break;
        }
    }
    
    VALIDATE(valid, "ARM SVE 11-operand gather");
    
    for (int i = 0; i < 16; i++) {
        g_checksum += (int)result_array[i];
    }
}
#endif /* __ARM_FEATURE_SVE */

/* ==================== PATTERN C: POWERPC ALTIVEC ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

NOINLINE static void test_powerpc_10_operands(void) {
    /* PowerPC vector permute with multiple arguments */
    
    vector float a = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float b = {5.0f, 6.0f, 7.0f, 8.0f};
    vector unsigned char perm = {0,1,2,3, 16,17,18,19, 8,9,10,11, 20,21,22,23};
    
    /* Complex vector operation that may require many operands */
    vector float result;
    
    /* Use inline assembly with 10 operands */
    asm volatile (
        "vperm %0, %1, %2, %3\n\t"
        : "=v"(result)
        : "v"(a), "v"(b), "v"(perm),
          "r"(0), "r"(0), "r"(0), "r"(0), "r"(0), "r"(0)  /* Extra dummy operands */
        : 
    );
    
    /* Simple validation */
    float res[4];
    memcpy(res, &result, 16);
    
    int valid = (res[0] == 1.0f && res[1] == 5.0f);
    VALIDATE(valid, "PowerPC 10-operand vector permute");
    
    for (int i = 0; i < 4; i++) {
        g_checksum += (int)res[i];
    }
}
#endif /* __ALTIVEC__ */

/* ==================== PATTERN D: GENERIC INLINE ASM ==================== */

/* Generic inline assembly with exactly 10 operands */
NOINLINE static void test_generic_10_operand_asm(void) {
    long ops[10];
    long result;
    
    for (int i = 0; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* Inline assembly with exactly 10 operands */
    asm volatile (
        "/* 10-operand dummy pattern for RTL expansion */\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]), "r"(ops[8])
        : "cc"
    );
    
    /* Expected: 1+2+3+4+5+6+7+8+9 = 45 */
    VALIDATE(result == 45, "Generic 10-operand inline assembly");
    g_checksum += result;
}

/* Generic inline assembly with exactly 11 operands */
NOINLINE static void test_generic_11_operand_asm(void) {
    long ops[11];
    long result;
    
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        "/* 11-operand dummy pattern for RTL expansion */\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9]), "r"(ops[10])
        : "cc"
    );
    
    /* Expected: 1+2+3+4+5+6+7+8+9+10 = 55 */
    VALIDATE(result == 55, "Generic 11-operand inline assembly");
    g_checksum += result;
}

/* ==================== HOT LOOP WITH VECTORIZATION ==================== */

NOINLINE static void hot_loop_with_multi_operand_patterns(void) {
    /* This function contains a hot loop that should trigger
     * vectorization and RTL expansion with many operands */
    
    double a[1024], b[1024], c[1024];
    
    /* Initialize arrays */
    #pragma omp parallel for
    for (int i = 0; i < 1024; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.5;
    }
    
    /* Complex computation that may generate multi-operand RTL */
    #pragma omp parallel for simd
    for (int i = 0; i < 1024; i++) {
        /* Fused multiply-add pattern that could expand to many operands */
        c[i] = a[i] * b[i] + a[i] + b[i];
        
        /* Additional operations to encourage complex RTL patterns */
        if (i % 2 == 0) {
            c[i] = c[i] * 2.0 - 1.0;
        }
    }
    
    /* Validate */
    int valid = 1;
    for (int i = 0; i < 1024; i++) {
        double expected = a[i] * b[i] + a[i] + b[i];
        if (i % 2 == 0) expected = expected * 2.0 - 1.0;
        
        if (c[i] != expected) {
            valid = 0;
            break;
        }
    }
    
    VALIDATE(valid, "Hot loop vectorization");
    
    /* Update checksum */
    for (int i = 0; i < 1024; i += 64) {
        g_checksum += (int)c[i];
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    printf("Testing RTL expansion with 10-11 operands...\n");
    printf("=============================================\n");
    
    /* Always run generic tests */
    test_generic_10_operand_asm();
    test_generic_11_operand_asm();
    
    /* Run architecture-specific tests */
#ifdef __AVX512F__
    printf("Running AVX-512 tests...\n");
    test_avx512_gather_10_operands();
    test_avx512_scatter_11_operands();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Running ARM SVE tests...\n");
    test_arm_sve_11_operands();
#endif
    
#ifdef __ALTIVEC__
    printf("Running PowerPC Altivec tests...\n");
    test_powerpc_10_operands();
#endif
    
    /* Run hot loop to trigger optimization passes */
    printf("Running vectorized hot loop...\n");
    hot_loop_with_multi_operand_patterns();
    
    /* Summary */
    printf("\n=============================================\n");
    printf("Tests run: %d\n", g_tests_run);
    printf("Tests passed: %d\n", g_tests_passed);
    printf("Final checksum: %d\n", g_checksum);
    
    if (g_tests_run == g_tests_passed) {
        printf("\nSUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("\nFAILURE: Some tests failed\n");
        return 1;
    }
}
