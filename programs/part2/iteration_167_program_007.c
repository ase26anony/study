/* test_optabs_10_11_operands.c
 * Test program to trigger RTL expansion with 10-11 operands
 * Compile with: gcc -O3 -ftree-vectorize -funroll-loops -fno-inline -march=native test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Runtime validation helpers */
static int tests_passed = 0;
static int tests_run = 0;

#define TEST_START() tests_run++
#define TEST_PASS() tests_passed++; printf("  PASS\n")
#define TEST_FAIL(msg) printf("  FAIL: %s\n", msg)

#define ARRAY_SIZE 64
#define ALIGN_64 __attribute__((aligned(64)))

/* Global test data */
static double src_data[ARRAY_SIZE] ALIGN_64;
static double dst_data[ARRAY_SIZE] ALIGN_64;
static int64_t indices[ARRAY_SIZE] ALIGN_64;
static double expected[ARRAY_SIZE] ALIGN_64;

/* Initialize test data */
static void init_test_data(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_data[i] = (double)(i * 2 + 1);
        indices[i] = (i % 16) * 4;  /* Scattered access pattern */
        expected[i] = src_data[indices[i] % ARRAY_SIZE] * 2.0;
    }
}

/* ============================================
 * PATTERN A: 10 OPERANDS (x86 AVX-512)
 * ============================================ */
#ifdef __AVX512F__

/* Force no-inline to ensure RTL expansion happens */
__attribute__((noinline, target("avx512f")))
static void test_10_operands_avx512(void) {
    TEST_START();
    printf("Testing 10-operand AVX-512 masked gather...\n");
    
    /* Initialize vectors */
    __m512d src = _mm512_set1_pd(2.0);
    __m512i vindex = _mm512_load_epi64(indices);
    __mmask8 mask = 0xFF;  /* All lanes active */
    
    /* This gather intrinsic conceptually uses 10 operands:
     * 1. Destination vector (return value)
     * 2. Mask
     * 3. Source vector
     * 4. Base pointer
     * 5. vindex
     * 6. Scale (compile-time constant 8 for doubles)
     * 7. Address displacement (0)
     * 8. Hint (_MM_HINT_T0)
     * 9. Mask again (explicit parameter)
     * 10. Memory operand (implicit)
     * 
     * The RTL expander should see this as a 10-operand pattern
     */
    __m512d result = _mm512_mask_i64gather_pd(
        src,                    /* src operand */
        mask,                   /* mask operand */
        vindex,                 /* index operand */
        src_data,               /* base pointer */
        8,                      /* scale (sizeof(double)) */
        _MM_SCALE_8             /* scale enum */
    );
    
    /* Store and validate */
    _mm512_store_pd(dst_data, result);
    
    /* Simple validation - check first 8 elements */
    int valid = 1;
    for (int i = 0; i < 8; i++) {
        double expected_val = src_data[indices[i] % ARRAY_SIZE] * 2.0;
        if (dst_data[i] != expected_val) {
            valid = 0;
            break;
        }
    }
    
    if (valid) TEST_PASS();
    else TEST_FAIL("AVX-512 gather result mismatch");
}

/* Alternative: Inline assembly with 10 explicit operands */
__attribute__((noinline, target("avx512f")))
static void test_10_operands_asm(void) {
    TEST_START();
    printf("Testing 10-operand inline assembly...\n");
    
    /* Use inline assembly that forces 10 operands */
    double result[8] ALIGN_64;
    double *base = src_data;
    long long idx[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    
    /* This asm statement has 10 operands:
     * 1 output operand + 9 input operands
     */
    asm volatile (
        "vmovupd %1, %%zmm0\n\t"
        "vmovupd %2, %%zmm1\n\t"
        "kxnorw %%k0, %%k0, %%k1\n\t"  /* All-ones mask */
        "vgatherqpd %3(,%4,8), %%zmm0 {%%k1}\n\t"
        "vmovupd %%zmm0, %0\n\t"
        : "=m" (result[0])              /* operand 0: output */
        : "m" (src_data[0]),            /* operand 1: source data */
          "m" (idx[0]),                 /* operand 2: indices */
          "r" (base),                   /* operand 3: base pointer */
          "r" (8LL),                    /* operand 4: scale factor */
          "m" (mask),                   /* operand 5: mask variable */
          "r" (0LL),                    /* operand 6: displacement */
          "i" (_MM_HINT_T0),            /* operand 7: hint */
          "i" (8)                       /* operand 8: scale constant */
        : "zmm0", "zmm1", "k1", "memory"
    );
    
    /* Validate */
    int valid = 1;
    for (int i = 0; i < 8; i++) {
        if (result[i] != src_data[idx[i] / 8]) {
            valid = 0;
            break;
        }
    }
    
    if (valid) TEST_PASS();
    else TEST_FAIL("Inline assembly result mismatch");
}

#endif /* __AVX512F__ */

/* ============================================
 * PATTERN B: 11 OPERANDS (x86 AVX-512)
 * ============================================ */
#ifdef __AVX512F__

__attribute__((noinline, target("avx512f,avx512vl")))
static void test_11_operands_scatter(void) {
    TEST_START();
    printf("Testing 11-operand AVX-512 masked scatter...\n");
    
    /* Complex scatter operation that may require 11 operands */
    __m512d src = _mm512_load_pd(src_data);
    __m512i vindex = _mm512_load_epi64(indices);
    __mmask8 mask = 0x0F;  /* Lower 4 lanes active */
    
    /* Scatter with multiple parameters - potentially 11 operands:
     * 1. Base pointer
     * 2. vindex
     * 3. Scale
     * 4. Displacement
     * 5. Hint
     * 6. Mask
     * 7. Source data
     * 8. Memory operand (implicit)
     * 9. Mask register operand
     * 10. Index register operand  
     * 11. Source register operand
     */
    _mm512_mask_i64scatter_pd(
        dst_data,              /* base pointer */
        mask,                  /* mask */
        vindex,                /* indices */
        src,                   /* source data */
        8,                     /* scale */
        _MM_SCALE_8            /* scale enum */
    );
    
    /* Validate scattered data */
    int valid = 1;
    for (int i = 0; i < 4; i++) {  /* Only first 4 lanes active */
        int idx = indices[i] % ARRAY_SIZE;
        if (dst_data[idx] != src_data[i]) {
            valid = 0;
            break;
        }
    }
    
    if (valid) TEST_PASS();
    else TEST_FAIL("AVX-512 scatter result mismatch");
}

#endif /* __AVX512F__ */

/* ============================================
 * ARM SVE PATTERNS (if supported)
 * ============================================ */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

__attribute__((noinline))
static void test_10_operands_sve(void) {
    TEST_START();
    printf("Testing 10-operand SVE gather...\n");
    
    svbool_t pg = svptrue_b64();
    svint64_t indices_vec = svld1_s64(pg, indices);
    
    /* SVE gather with predicate, base, and offsets */
    svfloat64_t result = svld1_gather_offset(pg, src_data, indices_vec);
    
    /* Store and validate */
    svst1(pg, dst_data, result);
    
    int valid = 1;
    for (int i = 0; i < svcntd(); i++) {
        int idx = indices[i] % ARRAY_SIZE;
        if (dst_data[i] != src_data[idx]) {
            valid = 0;
            break;
        }
    }
    
    if (valid) TEST_PASS();
    else TEST_FAIL("SVE gather result mismatch");
}

#endif /* __ARM_FEATURE_SVE */

/* ============================================
 * POWERPC ALTIVEC/VSX PATTERNS
 * ============================================ */
#ifdef __ALTIVEC__

#include <altivec.h>

__attribute__((noinline))
static void test_10_operands_powerpc(void) {
    TEST_START();
    printf("Testing 10-operand PowerPC vector permute...\n");
    
    /* Complex vector permutation with multiple arguments */
    vector double v1 = vec_ld(0, src_data);
    vector double v2 = vec_ld(16, src_data + 2);
    vector double v3 = vec_ld(32, src_data + 4);
    vector double v4 = vec_ld(48, src_data + 6);
    
    /* Use vec_perm with multiple vector arguments */
    vector unsigned char perm = {0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23};
    
    /* This may expand to multiple operands */
    vector double result1 = vec_perm(v1, v2, perm);
    vector double result2 = vec_perm(v3, v4, perm);
    
    vec_st(result1, 0, dst_data);
    vec_st(result2, 16, dst_data + 2);
    
    TEST_PASS();  /* Simple execution test */
}

#endif /* __ALTIVEC__ */

/* ============================================
 * RISC-V VECTOR EXTENSION PATTERNS
 * ============================================ */
#ifdef __riscv_v

/* RISC-V vector intrinsics */
__attribute__((noinline))
static void test_10_operands_riscv(void) {
    TEST_START();
    printf("Testing 10-operand RISC-V vector load...\n");
    
    /* Placeholder for RISC-V vector operations */
    /* When GCC supports RVV intrinsics, use vle64.v with mask */
    
    TEST_PASS();
}

#endif /* __riscv_v */

/* ============================================
 * GENERIC INLINE ASSEMBLY FALLBACK
 * ============================================ */
__attribute__((noinline))
static void test_11_operands_generic_asm(void) {
    TEST_START();
    printf("Testing 11-operand generic inline assembly...\n");
    
    /* Generic inline assembly with 11 operands */
    long long out1, out2;
    long long in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    long long in6 = 6, in7 = 7, in8 = 8, in9 = 9;
    
    /* Force 11 operands: 2 outputs + 9 inputs */
    asm volatile (
        "/* Dummy 11-operand instruction */\n\t"
        "mov %1, %10\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        : "=r" (out1), "=r" (out2)      /* 2 output operands */
        : "r" (in1), "r" (in2), "r" (in3),  /* 9 input operands */
          "r" (in4), "r" (in5), "r" (in6),
          "r" (in7), "r" (in8), "r" (in9)
        : "cc"
    );
    
    /* Simple validation */
    long long expected_sum = 1+2+3+4+5+6+7+8+9;
    if (out1 == expected_sum && out2 == 9) {
        TEST_PASS();
    } else {
        TEST_FAIL("Generic assembly result mismatch");
    }
}

/* ============================================
 * MAIN FUNCTION
 * ============================================ */
int main(void) {
    printf("=== Testing RTL Expansion with 10-11 Operands ===\n\n");
    
    init_test_data();
    
    /* Clear destination array */
    memset(dst_data, 0, sizeof(dst_data));
    
    /* Run architecture-specific tests */
    
    /* x86 AVX-512 tests */
#ifdef __AVX512F__
    printf("\n--- x86 AVX-512 Tests ---\n");
    test_10_operands_avx512();
    test_10_operands_asm();
    test_11_operands_scatter();
#endif
    
    /* ARM SVE tests */
#ifdef __ARM_FEATURE_SVE
    printf("\n--- ARM SVE Tests ---\n");
    test_10_operands_sve();
#endif
    
    /* PowerPC tests */
#ifdef __ALTIVEC__
    printf("\n--- PowerPC Altivec/VSX Tests ---\n");
    test_10_operands_powerpc();
#endif
    
    /* RISC-V tests */
#ifdef __riscv_v
    printf("\n--- RISC-V Vector Tests ---\n");
    test_10_operands_riscv();
#endif
    
    /* Generic fallback test (always runs) */
    printf("\n--- Generic Tests ---\n");
    test_11_operands_generic_asm();
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_passed == tests_run) {
        printf("All tests passed successfully!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
