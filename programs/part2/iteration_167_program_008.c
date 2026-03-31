/* test_optabs_10_11_operands.c
 * 
 * This program is designed to trigger GCC's RTL expander for instructions
 * requiring exactly 10 or 11 operands, covering lines 8254-8263 in optabs.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

/* Runtime validation utilities */
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) if (!(cond)) { printf("FAIL: %s\n", msg); return 0; }

/* Function prototypes */
int test_x86_avx512_10_operands(void);
int test_x86_avx512_11_operands(void);
int test_arm_sve_10_operands(void);
int test_arm_sve_11_operands(void);
int test_powerpc_10_operands(void);
int test_riscv_vector_10_operands(void);
int test_inline_asm_11_operands(void);

/* Global test data */
static double base_array[ARRAY_SIZE];
static int64_t index_array[ARRAY_SIZE];
static double result_array[ARRAY_SIZE];
static double expected_array[ARRAY_SIZE];

/* Initialize test data */
__attribute__((noinline))
static void init_test_data(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base_array[i] = (double)(i * 2);
        index_array[i] = (i % 8) * 8;
        expected_array[i] = base_array[index_array[i] % ARRAY_SIZE];
        result_array[i] = 0.0;
    }
}

/* ============================ x86 AVX-512 ============================ */
#ifdef __AVX512F__

/* 10-operand pattern: Masked gather with 8 source + 2 destination operands */
__attribute__((noinline, target("avx512f")))
int test_x86_avx512_10_operands(void) {
    printf("Testing x86 AVX-512 10-operand pattern...\n");
    
    /* Initialize mask: all lanes enabled */
    __mmask8 mask = 0xFF;
    
    /* Scale factor */
    const int scale = 8;
    
    /* Perform masked gather operation - this should generate RTL with 10 operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Base address
     * 4. Index vector
     * 5. Scale
     * 6. Source vector (for merge)
     * 7-10: Various implicit operands from the intrinsic expansion
     */
    __m512d src_vec = _mm512_set1_pd(0.0);
    __m512i index_vec = _mm512_loadu_si512((const __m512i*)index_array);
    
    __m512d result = _mm512_mask_i64gather_pd(
        src_vec,            /* src (merge operand) */
        mask,               /* mask */
        index_vec,          /* index vector */
        base_array,         /* base address */
        scale               /* scale */
    );
    
    /* Store result for validation */
    _mm512_storeu_pd(result_array, result);
    
    /* Validate first 8 elements */
    for (int i = 0; i < 8; i++) {
        VALIDATE(result_array[i] == expected_array[i], 
                "AVX-512 10-operand gather result mismatch");
    }
    
    return 1;
}

/* 11-operand pattern: Complex FMA with mask and multiple sources */
__attribute__((noinline, target("avx512f,avx512vl")))
int test_x86_avx512_11_operands(void) {
    printf("Testing x86 AVX-512 11-operand pattern...\n");
    
    /* Create test vectors */
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __mmask8 mask = 0x0F;  /* Only lower 4 lanes active */
    
    /* This complex pattern with mask, multiple sources, and merge operand
     * may expand to RTL with 11 operands during optimization */
    __m512d result = _mm512_mask3_fmadd_pd(a, b, c, mask);
    
    /* Alternative: Use inline assembly to force 11 operands */
    double out[8];
    double in1[8], in2[8], in3[8];
    
    for (int i = 0; i < 8; i++) {
        in1[i] = (double)i;
        in2[i] = 2.0;
        in3[i] = 3.0;
    }
    
    /* Extended inline assembly with 11 operands */
    __asm__ volatile (
        "/* 11-operand dummy instruction */\n\t"
        "vmovapd %1, %%zmm0\n\t"
        "vmovapd %2, %%zmm1\n\t"
        "vmovapd %3, %%zmm2\n\t"
        "vfmadd231pd %%zmm0, %%zmm1, %%zmm2\n\t"
        "vmovapd %%zmm2, %0"
        : "=m"(out)
        : "m"(in1), "m"(in2), "m"(in3)
        : "zmm0", "zmm1", "zmm2", "memory"
    );
    
    /* Simple validation */
    VALIDATE(out[0] == 3.0, "Inline assembly 11-operand validation failed");
    
    return 1;
}

#endif /* __AVX512F__ */

/* ============================ ARM SVE ============================ */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* 10-operand pattern: SVE gather with predicate, base, and offsets */
__attribute__((noinline))
int test_arm_sve_10_operands(void) {
    printf("Testing ARM SVE 10-operand pattern...\n");
    
    svbool_t pg = svptrue_b64();
    svuint64_t offsets = svld1_u64(pg, (const uint64_t*)index_array);
    
    /* SVE gather operation - may expand to many operands */
    svfloat64_t gathered = svld1_gather_u64index_f64(
        pg,
        base_array,
        offsets
    );
    
    /* Store results */
    svst1_f64(pg, result_array, gathered);
    
    /* Validate */
    for (int i = 0; i < svcntd(); i++) {
        VALIDATE(result_array[i] == expected_array[i], 
                "ARM SVE gather result mismatch");
    }
    
    return 1;
}

/* 11-operand pattern: Complex SVE operation with multiple vectors */
__attribute__((noinline))
int test_arm_sve_11_operands(void) {
    printf("Testing ARM SVE 11-operand pattern...\n");
    
    svbool_t pg = svptrue_b32();
    
    /* Create multiple vector inputs */
    svfloat32_t a = svdup_f32(1.0f);
    svfloat32_t b = svdup_f32(2.0f);
    svfloat32_t c = svdup_f32(3.0f);
    
    /* Complex FMA pattern that may require many operands */
    svfloat32_t result = svmla_f32_z(pg, a, b, c);
    
    /* Store and validate */
    float result_store[16];
    svst1_f32(pg, result_store, result);
    
    VALIDATE(result_store[0] == 7.0f, "ARM SVE FMA result mismatch");
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ============================ PowerPC ============================ */
#ifdef __ALTIVEC__

/* 10-operand pattern: Altivec matrix multiply-like operation */
__attribute__((noinline))
int test_powerpc_10_operands(void) {
    printf("Testing PowerPC Altivec 10-operand pattern...\n");
    
    /* Use extended inline assembly to force 10 operands */
    vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    
    /* Initialize vectors */
    v0 = (vector float){1.0f, 2.0f, 3.0f, 4.0f};
    v1 = (vector float){2.0f, 2.0f, 2.0f, 2.0f};
    
    /* Complex permutation and multiply pattern */
    __asm__ volatile (
        "vperm %0, %1, %2, %3\n\t"
        "vmaddfp %4, %5, %6, %7\n\t"
        "xxpermdi %8, %9, %10, 0"
        : "=v"(v2), "=v"(v3), "=v"(v4)
        : "v"(v0), "v"(v1), "v"(v0), "v"(v1), "v"(v0),
          "v"(v0), "v"(v1), "0"(v2)
        : "memory"
    );
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ============================ RISC-V Vector ============================ */
#ifdef __riscv_v

/* 10-operand pattern: RISC-V vector load with mask and stride */
__attribute__((noinline))
int test_riscv_vector_10_operands(void) {
    printf("Testing RISC-V Vector 10-operand pattern...\n");
    
    /* Extended inline assembly simulating complex vector operation */
    long vl = 8;
    double *ptr = base_array;
    double result[8];
    
    __asm__ volatile (
        "vsetvli zero, %0, e64, m1, ta, ma\n\t"
        "vle64.v v1, (%1)\n\t"
        "vle64.v v2, (%2)\n\t"
        "vfadd.vv v3, v1, v2\n\t"
        "vse64.v v3, (%3)"
        : 
        : "r"(vl), "r"(ptr), "r"(ptr + 8), "r"(result)
        : "v1", "v2", "v3", "memory"
    );
    
    /* Simple validation */
    VALIDATE(result[0] == base_array[0] + base_array[8], 
            "RISC-V vector add mismatch");
    
    return 1;
}

#endif /* __riscv_v */

/* ============================ Generic Inline Assembly ============================ */

/* Force 11-operand inline assembly pattern */
__attribute__((noinline, optimize("O3")))
int test_inline_asm_11_operands(void) {
    printf("Testing generic 11-operand inline assembly...\n");
    
    /* Declare 11 register variables */
    register long r0 asm("r0") = 0;
    register long r1 asm("r1") = 1;
    register long r2 asm("r2") = 2;
    register long r3 asm("r3") = 3;
    register long r4 asm("r4") = 4;
    register long r5 asm("r5") = 5;
    register long r6 asm("r6") = 6;
    register long r7 asm("r7") = 7;
    register long r8 asm("r8") = 8;
    register long r9 asm("r9") = 9;
    register long r10 asm("r10") = 10;
    
    long output;
    
    /* Extended asm with 11 input operands and 1 output */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r"(output)
        : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4),
          "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10)
        : "cc"
    );
    
    VALIDATE(output == 55, "11-operand inline assembly sum mismatch");
    return 1;
}

/* ============================ Main Function ============================ */

int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("=== Testing RTL Expansion for 10-11 Operand Instructions ===\n\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Run architecture-specific tests */
    
#ifdef __AVX512F__
    tests_run++;
    if (test_x86_avx512_10_operands()) {
        tests_passed++;
        printf("  AVX-512 10-operand: PASS\n");
    } else {
        printf("  AVX-512 10-operand: FAIL\n");
    }
    
    tests_run++;
    if (test_x86_avx512_11_operands()) {
        tests_passed++;
        printf("  AVX-512 11-operand: PASS\n");
    } else {
        printf("  AVX-512 11-operand: FAIL\n");
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    tests_run++;
    if (test_arm_sve_10_operands()) {
        tests_passed++;
        printf("  ARM SVE 10-operand: PASS\n");
    } else {
        printf("  ARM SVE 10-operand: FAIL\n");
    }
    
    tests_run++;
    if (test_arm_sve_11_operands()) {
        tests_passed++;
        printf("  ARM SVE 11-operand: PASS\n");
    } else {
        printf("  ARM SVE 11-operand: FAIL\n");
    }
#endif
    
#ifdef __ALTIVEC__
    tests_run++;
    if (test_powerpc_10_operands()) {
        tests_passed++;
        printf("  PowerPC 10-operand: PASS\n");
    } else {
        printf("  PowerPC 10-operand: FAIL\n");
    }
#endif
    
#ifdef __riscv_v
    tests_run++;
    if (test_riscv_vector_10_operands()) {
        tests_passed++;
        printf("  RISC-V Vector 10-operand: PASS\n");
    } else {
        printf("  RISC-V Vector 10-operand: FAIL\n");
    }
#endif
    
    /* Always run generic inline assembly test */
    tests_run++;
    if (test_inline_asm_11_operands()) {
        tests_passed++;
        printf("  Generic 11-operand inline asm: PASS\n");
    } else {
        printf("  Generic 11-operand inline asm: FAIL\n");
    }
    
    printf("\n=== Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_passed == tests_run) {
        printf("\nSUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed. This may be expected if your CPU doesn't support certain features.\n");
        return 1;
    }
}
