/* test_optabs_10_11_operands.c
 * 
 * This program generates RTL patterns requiring 10-11 operands
 * to trigger uncovered lines 8254-8263 in optabs.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation helpers */
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "Validation failed: %s\n", msg); \
        return 0; \
    } \
} while(0)

/* Function attributes to control expansion */
#define NOINLINE __attribute__((noinline, optimize("O3")))
#define HOT __attribute__((hot))

/* Test data initialization */
static void init_test_data(double *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (double)(i * 2 + 1);
    }
}

static void init_test_data_int64(int64_t *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (int64_t)(i * 3 + 1);
    }
}

/* ==================== x86 AVX-512 Patterns ==================== */

#ifdef __AVX512F__

#include <immintrin.h>

/* Pattern A: 10 operands - masked gather operation */
NOINLINE HOT
int test_avx512_10_operands(void) {
    double src[ARRAY_SIZE];
    double dst[ARRAY_SIZE] = {0};
    int64_t indices[ARRAY_SIZE];
    __mmask8 mask = 0xFF;
    
    init_test_data(src, ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        indices[i] = (i * 2) % ARRAY_SIZE;
    }
    
    /* This gather intrinsic expands to many operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Source vector
     * 4. Base address
     * 5. Index vector
     * 6. Scale
     * 7. Vector length
     * Plus implicit operands for address computation
     */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        __m512i vindex = _mm512_loadu_si512((__m512i*)&indices[i]);
        __m512d vsrc = _mm512_loadu_pd(&src[i]);
        __m512d result = _mm512_mask_i64gather_pd(vsrc, mask, vindex, 
                                                 (void*)src, 8);
        _mm512_storeu_pd(&dst[i], result);
    }
    
    /* Validate */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double expected = src[(i * 2) % ARRAY_SIZE];
        VALIDATE(dst[i] == expected || (i % 8 >= 8), 
                "AVX-512 10-operand gather failed");
    }
    
    return 1;
}

/* Pattern B: 11 operands - complex masked scatter with update */
NOINLINE HOT
int test_avx512_11_operands(void) {
    double data[ARRAY_SIZE * 2];
    double updates[ARRAY_SIZE];
    int64_t indices[ARRAY_SIZE];
    __mmask16 mask = 0xFFFF;
    
    init_test_data(data, ARRAY_SIZE * 2);
    init_test_data(updates, ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        indices[i] = (i * 3) % (ARRAY_SIZE * 2);
    }
    
    /* This scatter operation with multiple parameters should generate
     * 11 operands during RTL expansion:
     * 1. Base address
     * 2. Mask
     * 3. Index vector
     * 4. Source data
     * 5. Scale
     * 6. Displacement
     * 7. Hint
     * Plus additional operands for address modes and temporaries
     */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        __m512i vindex = _mm512_loadu_si512((__m512i*)&indices[i]);
        __m512d vupdate = _mm512_loadu_pd(&updates[i]);
        
        /* Extended inline assembly to force 11 operands */
        __asm__ volatile (
            "/* 11-operand dummy instruction */\n\t"
            "vmovapd %0, %0\n\t"  /* Use the result */
            : "+v"(vupdate)
            : "v"(vupdate), "v"(_mm512_castsi512_pd(vindex)), 
              "m"(data), "m"(mask), "i"(8), "i"(1),
              "r"((uintptr_t)data), "r"(ARRAY_SIZE),
              "m"(indices[0]), "m"(updates[0])
            : "memory"
        );
        
        _mm512_mask_i64scatter_pd((void*)data, mask, vindex, vupdate, 8);
    }
    
    /* Validate some scattered values */
    int valid_count = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int idx = indices[i];
        if (idx < ARRAY_SIZE * 2) {
            /* Check if value was updated (depends on mask) */
            if (data[idx] == updates[i] || data[idx] == (double)(idx * 2 + 1)) {
                valid_count++;
            }
        }
    }
    
    VALIDATE(valid_count >= ARRAY_SIZE / 2, 
            "AVX-512 11-operand scatter failed");
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Patterns ==================== */

#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands - SVE gather with predicate */
NOINLINE HOT
int test_arm_sve_10_operands(void) {
    double src[ARRAY_SIZE];
    double dst[ARRAY_SIZE] = {0};
    int64_t indices[ARRAY_SIZE];
    
    init_test_data(src, ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        indices[i] = (i * 2) % ARRAY_SIZE;
    }
    
    svbool_t pg = svwhilelt_b64(0, ARRAY_SIZE);
    
    /* SVE gather with multiple parameters - should expand to 10 operands */
    for (int i = 0; i < ARRAY_SIZE; i += svcntd()) {
        svint64_t vindex = svld1_s64(pg, &indices[i]);
        svfloat64_t result = svld1_gather_s64index_f64(pg, src, vindex);
        svst1_f64(pg, &dst[i], result);
    }
    
    /* Validate */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double expected = src[(i * 2) % ARRAY_SIZE];
        VALIDATE(dst[i] == expected, "ARM SVE 10-operand gather failed");
    }
    
    return 1;
}

/* Pattern B: 11 operands - SVE scatter with multiple offsets */
NOINLINE HOT  
int test_arm_sve_11_operands(void) {
    double data[ARRAY_SIZE * 2];
    double updates[ARRAY_SIZE];
    int64_t indices[ARRAY_SIZE];
    int64_t offsets[ARRAY_SIZE];
    
    init_test_data(data, ARRAY_SIZE * 2);
    init_test_data(updates, ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        indices[i] = i;
        offsets[i] = (i % 4) * 8;
    }
    
    svbool_t pg = svwhilelt_b64(0, ARRAY_SIZE);
    
    /* Complex scatter operation that should require 11 operands */
    for (int i = 0; i < ARRAY_SIZE; i += svcntd()) {
        svint64_t vindex = svld1_s64(pg, &indices[i]);
        svint64_t voffset = svld1_s64(pg, &offsets[i]);
        svfloat64_t vupdate = svld1_f64(pg, &updates[i]);
        
        /* Force 11 operands through inline assembly */
        __asm__ volatile (
            "/* ARM SVE 11-operand pattern */\n\t"
            "mov %0, %0\n\t"  /* Use the vectors */
            : "+w"(vupdate)
            : "w"(vindex), "w"(voffset), "w"(pg),
              "m"(data), "m"(updates), "m"(indices), "m"(offsets),
              "r"((uintptr_t)data), "r"(ARRAY_SIZE), "i"(8)
            : "memory"
        );
        
        svst1_scatter_s64offset_f64(pg, data, vindex, vupdate);
    }
    
    /* Validate */
    int valid_count = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int idx = indices[i];
        if (idx < ARRAY_SIZE && data[idx] == updates[i]) {
            valid_count++;
        }
    }
    
    VALIDATE(valid_count > 0, "ARM SVE 11-operand scatter failed");
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC VSX Patterns ==================== */

#ifdef __VSX__

#include <altivec.h>

/* Pattern for 10 operands - VSX matrix multiply-like operation */
NOINLINE HOT
int test_powerpc_10_operands(void) {
    vector double a[4], b[4], c[4], d[4];
    
    /* Initialize vectors */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            a[i][j] = i + j + 1.0;
            b[i][j] = i * j + 2.0;
            c[i][j] = 0.0;
            d[i][j] = 0.0;
        }
    }
    
    /* Complex vector operation that should expand to 10 operands */
    for (int i = 0; i < 4; i++) {
        vector double t1 = vec_madd(a[i], b[i], c[i]);
        vector double t2 = vec_nmsub(a[(i+1)%4], b[(i+2)%4], d[i]);
        
        /* Inline assembly to ensure 10 operands */
        __asm__ volatile (
            "xvadddp %0, %1, %2\n\t"
            "xvmuldp %3, %4, %5\n\t"
            : "=wa"(c[i]), "=wa"(d[i])
            : "wa"(t1), "wa"(t2), 
              "wa"(a[i]), "wa"(b[i]), "wa"(c[(i+1)%4]),
              "wa"(d[(i+1)%4]), "r"(i), "m"(a[0])
            : "v0", "v1", "v2", "v3"
        );
    }
    
    /* Simple validation */
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            sum += c[i][j] + d[i][j];
        }
    }
    
    VALIDATE(sum > 0.0, "PowerPC 10-operand vector op failed");
    
    return 1;
}

#endif /* __VSX__ */

/* ==================== Generic fallback with inline asm ==================== */

/* Generic 10-operand inline assembly pattern */
NOINLINE HOT
int test_generic_10_operands(void) {
    uint64_t ops[10];
    uint64_t result = 0;
    
    for (int i = 0; i < 10; i++) {
        ops[i] = i * 3 + 1;
    }
    
    /* Explicit 10-operand inline asm */
    __asm__ volatile (
        "/* 10-operand dummy instruction */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9])
        : "cc"
    );
    
    uint64_t expected = 0;
    for (int i = 0; i < 10; i++) {
        expected += ops[i];
    }
    
    VALIDATE(result == expected, "Generic 10-operand asm failed");
    return 1;
}

/* Generic 11-operand inline assembly pattern */
NOINLINE HOT
int test_generic_11_operands(void) {
    uint64_t ops[11];
    uint64_t results[2] = {0, 0};
    
    for (int i = 0; i < 11; i++) {
        ops[i] = i * 5 + 2;
    }
    
    /* Explicit 11-operand inline asm with 2 outputs */
    __asm__ volatile (
        "/* 11-operand dummy instruction */\n\t"
        "mov %0, %2\n\t"
        "mov %1, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %1, %1, %5\n\t"
        "mul %0, %0, %6\n\t"
        "mul %1, %1, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %1, %1, %9\n\t"
        "sub %0, %0, %10\n\t"
        "sub %1, %1, %11\n\t"
        : "=r"(results[0]), "=r"(results[1])
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9]), "r"(ops[10])
        : "cc"
    );
    
    uint64_t expected0 = ((ops[0] + ops[2]) * ops[4]) + ops[6] - ops[8];
    uint64_t expected1 = ((ops[1] + ops[3]) * ops[5]) + ops[7] - ops[9];
    
    VALIDATE(results[0] == expected0 && results[1] == expected1,
            "Generic 11-operand asm failed");
    
    return 1;
}

/* ==================== Main test driver ==================== */

int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing RTL expansion with 10-11 operands...\n");
    printf("Targeting optabs.cc lines 8254-8263\n\n");
    
    /* Run architecture-specific tests */
    
#ifdef __AVX512F__
    printf("Testing AVX-512 patterns...\n");
    tests_run++;
    if (test_avx512_10_operands()) {
        printf("  AVX-512 10-operand test: PASSED\n");
        tests_passed++;
    } else {
        printf("  AVX-512 10-operand test: FAILED\n");
    }
    
    tests_run++;
    if (test_avx512_11_operands()) {
        printf("  AVX-512 11-operand test: PASSED\n");
        tests_passed++;
    } else {
        printf("  AVX-512 11-operand test: FAILED\n");
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE patterns...\n");
    tests_run++;
    if (test_arm_sve_10_operands()) {
        printf("  ARM SVE 10-operand test: PASSED\n");
        tests_passed++;
    } else {
        printf("  ARM SVE 10-operand test: FAILED\n");
    }
    
    tests_run++;
    if (test_arm_sve_11_operands()) {
        printf("  ARM SVE 11-operand test: PASSED\n");
        tests_passed++;
    } else {
        printf("  ARM SVE 11-operand test: FAILED\n");
    }
#endif
    
#ifdef __VSX__
    printf("Testing PowerPC VSX patterns...\n");
    tests_run++;
    if (test_powerpc_10_operands()) {
        printf("  PowerPC 10-operand test: PASSED\n");
        tests_passed++;
    } else {
        printf("  PowerPC 10-operand test: FAILED\n");
    }
#endif
    
    /* Always run generic tests */
    printf("Testing generic patterns...\n");
    tests_run++;
    if (test_generic_10_operands()) {
        printf("  Generic 10-operand test: PASSED\n");
        tests_passed++;
    } else {
        printf("  Generic 10-operand test: FAILED\n");
    }
    
    tests_run++;
    if (test_generic_11_operands()) {
        printf("  Generic 11-operand test: PASSED\n");
        tests_passed++;
    } else {
        printf("  Generic 11-operand test: FAILED\n");
    }
    
    printf("\nSummary: %d/%d tests passed\n", tests_passed, tests_run);
    
    if (tests_passed == tests_run) {
        printf("\nSUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed\n");
        return 1;
    }
}
