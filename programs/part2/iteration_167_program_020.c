/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expander for instructions
 * requiring exactly 10 or 11 operands, covering lines 8254-8263 in optabs.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define ARRAY_SIZE 1024
static int test_passed = 1;

static void validate_result(const char* test_name, int condition) {
    if (!condition) {
        printf("FAIL: %s\n", test_name);
        test_passed = 0;
    }
}

/* Generic portable fallback for validation */
static void portable_gather(double* src, double* dst, int* indices, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = src[indices[i] % n];
    }
}

/* ==================== x86 AVX-512 Paths ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Pattern A: 10 operands - Masked gather with multiple parameters */
__attribute__((noinline, target("avx512f")))
void avx512_10_operand_gather(void) {
    const int N = 16;
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    double dest[N] __attribute__((aligned(64)));
    int64_t indices[N] __attribute__((aligned(64)));
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)i * 1.5;
    }
    for (int i = 0; i < N; i++) {
        indices[i] = (i * 3) % ARRAY_SIZE;
        dest[i] = -1.0;
    }
    
    /* Create mask (all true) */
    __mmask8 mask = 0xFF;
    
    /* This intrinsic expands to an instruction requiring many operands:
     * dest, mask, indices, base, scale, displacement, etc.
     * _mm512_i64gather_pd typically requires: dest, mask, index, base, scale
     * But with all implicit operands, it can reach 10+ during RTL expansion
     */
    __m512d result = _mm512_mask_i64gather_pd(
        _mm512_setzero_pd(),    /* src operand */
        mask,                   /* mask */
        _mm512_load_epi64(indices), /* index vector */
        base,                   /* base pointer */
        8                       /* scale (sizeof(double)) */
    );
    
    _mm512_store_pd(dest, result);
    
    /* Validate */
    double expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = base[indices[i]];
    }
    
    __m512d loaded = _mm512_load_pd(dest);
    __m512d expected_vec = _mm512_load_pd(expected);
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(loaded, expected_vec, _CMP_EQ_OQ);
    
    validate_result("AVX512 10-operand gather", cmp_mask == 0xFF);
}

/* Pattern B: 11 operands - Complex masked scatter with update */
__attribute__((noinline, target("avx512f")))
void avx512_11_operand_scatter(void) {
    const int N = 8;
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    double src[N] __attribute__((aligned(64)));
    int64_t indices[N] __attribute__((aligned(64)));
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = 0.0;
    }
    for (int i = 0; i < N; i++) {
        indices[i] = (i * 5) % ARRAY_SIZE;
        src[i] = (double)(i + 100);
    }
    
    __mmask8 mask = 0xFF;
    
    /* Scatter operation with mask - expands to many operands during RTL */
    _mm512_mask_i64scatter_pd(
        base,                   /* base address */
        mask,                   /* mask */
        _mm512_load_epi64(indices), /* indices */
        _mm512_load_pd(src),    /* source data */
        8                       /* scale */
    );
    
    /* Validate scatter */
    int valid = 1;
    for (int i = 0; i < N; i++) {
        if (base[indices[i]] != src[i]) {
            valid = 0;
            break;
        }
    }
    
    validate_result("AVX512 11-operand scatter", valid);
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Paths ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A for ARM SVE: 10+ operand gather */
__attribute__((noinline))
void arm_sve_10_operand_gather(void) {
    const int N = 256;
    double base[ARRAY_SIZE];
    double dest[N];
    uint64_t indices[N];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)i * 2.0;
    }
    for (int i = 0; i < N; i++) {
        indices[i] = i % ARRAY_SIZE;
    }
    
    svbool_t pg = svptrue_b64();
    
    /* SVE gather with predicate - expands to many operands */
    svfloat64_t gathered = svld1_gather_index(pg, base, svld1sw_u64(pg, indices));
    
    svst1_f64(pg, dest, gathered);
    
    /* Validate */
    int valid = 1;
    for (int i = 0; i < N; i += svcntd()) {
        if (dest[i] != base[indices[i]]) {
            valid = 0;
            break;
        }
    }
    
    validate_result("ARM SVE 10-operand gather", valid);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC VSX/Altivec Paths ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

/* Complex permutation with many vector operands */
__attribute__((noinline))
void powerpc_multi_operand_permute(void) {
    const int N = 16;
    vector float a[N], b[N], c[N], d[N];
    
    /* Initialize vectors */
    for (int i = 0; i < N; i++) {
        a[i] = (vector float){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
        b[i] = (vector float){i*5.0f, i*6.0f, i*7.0f, i*8.0f};
    }
    
    /* Complex operation chain that may expand to many operands */
    for (int i = 0; i < N; i++) {
        /* vec_madd + vec_perm combination */
        vector float madd = vec_madd(a[i], b[i], a[(i+1)%N]);
        vector float perm = vec_perm(madd, b[i], (vector unsigned char){
            0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15
        });
        c[i] = vec_add(perm, a[i]);
        
        /* Another operation to increase operand count */
        d[i] = vec_sub(vec_madd(c[i], b[i], a[i]), 
                      vec_perm(b[i], c[i], (vector unsigned char){
                          15,14,13,12, 11,10,9,8, 7,6,5,4, 3,2,1,0
                      }));
    }
    
    /* Simple validation */
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            sum += ((float*)&d[i])[j];
        }
    }
    
    validate_result("PowerPC multi-operand permute", sum != 0.0f);
}

#endif /* __ALTIVEC__ */

/* ==================== RISC-V Vector Extension ==================== */
#ifdef __riscv_v

/* RISC-V vector extension with many operands */
__attribute__((noinline))
void riscv_vector_multi_operand(void) {
    /* Using inline assembly to force multi-operand pattern */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result1, result2;
    
    /* Pattern A: 10 operands */
    asm volatile (
        "dummy10 %0, %1, %2, %3, %4, %5, %6, %7, %8, %9"
        : "=r"(result1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "memory"
    );
    
    /* Pattern B: 11 operands */
    asm volatile (
        "dummy11 %0, %1, %2, %3, %4, %5, %6, %7, %8, %9, %10"
        : "=r"(result2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "memory"
    );
    
    validate_result("RISC-V 10/11 operand asm", (result1 + result2) == 0);
}

#endif /* __riscv_v */

/* ==================== Generic Inline Assembly Fallback ==================== */
/* Always compile this to ensure we have some multi-operand patterns */
__attribute__((noinline, optimize("O3")))
void generic_multi_operand_asm(void) {
    /* Force 10 operands */
    {
        long op0, op1, op2, op3, op4, op5, op6, op7, op8, op9;
        long result;
        
        /* Initialize operands */
        op0 = 0; op1 = 1; op2 = 2; op3 = 3; op4 = 4;
        op5 = 5; op6 = 6; op7 = 7; op8 = 8; op9 = 9;
        
        /* Inline assembly with 10 operands */
        asm volatile (
            "# 10-operand pattern\n\t"
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9"
            : "=r"(result)
            : "r"(op0), "r"(op1), "r"(op2), "r"(op3), 
              "r"(op4), "r"(op5), "r"(op6), "r"(op7), "r"(op8)
            : "cc"
        );
        
        validate_result("Generic 10-operand asm", result == 36);
    }
    
    /* Force 11 operands */
    {
        long op0, op1, op2, op3, op4, op5, op6, op7, op8, op9, op10;
        long result;
        
        /* Initialize operands */
        op0 = 0; op1 = 1; op2 = 2; op3 = 3; op4 = 4;
        op5 = 5; op6 = 6; op7 = 7; op8 = 8; op9 = 9;
        op10 = 10;
        
        /* Inline assembly with 11 operands */
        asm volatile (
            "# 11-operand pattern\n\t"
            "mov %0, %1\n\t"
            "imul %0, %2\n\t"
            "add %0, %3\n\t"
            "sub %0, %4\n\t"
            "and %0, %5\n\t"
            "or %0, %6\n\t"
            "xor %0, %7\n\t"
            "add %0, %8\n\t"
            "sub %0, %9\n\t"
            "add %0, %10"
            : "=r"(result)
            : "r"(op0), "r"(op1), "r"(op2), "r"(op3), 
              "r"(op4), "r"(op5), "r"(op6), "r"(op7), 
              "r"(op8), "r"(op9), "r"(op10)
            : "cc"
        );
        
        validate_result("Generic 11-operand asm", result != 0);
    }
}

/* ==================== Main Function ==================== */
int main(void) {
    int architectures_tested = 0;
    int architectures_passed = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    
    /* Always test generic assembly */
    architectures_tested++;
    printf("Testing generic inline assembly patterns...\n");
    generic_multi_operand_asm();
    if (test_passed) architectures_passed++;
    
    /* Test architecture-specific paths */
    #ifdef __AVX512F__
    architectures_tested++;
    test_passed = 1;
    printf("Testing AVX-512 patterns...\n");
    avx512_10_operand_gather();
    avx512_11_operand_scatter();
    if (test_passed) architectures_passed++;
    #endif
    
    #ifdef __ARM_FEATURE_SVE
    architectures_tested++;
    test_passed = 1;
    printf("Testing ARM SVE patterns...\n");
    arm_sve_10_operand_gather();
    if (test_passed) architectures_passed++;
    #endif
    
    #ifdef __ALTIVEC__
    architectures_tested++;
    test_passed = 1;
    printf("Testing PowerPC Altivec patterns...\n");
    powerpc_multi_operand_permute();
    if (test_passed) architectures_passed++;
    #endif
    
    #ifdef __riscv_v
    architectures_tested++;
    test_passed = 1;
    printf("Testing RISC-V Vector patterns...\n");
    riscv_vector_multi_operand();
    if (test_passed) architectures_passed++;
    #endif
    
    printf("\n=== Test Summary ===\n");
    printf("Architectures tested: %d\n", architectures_tested);
    printf("Architectures passed: %d\n", architectures_passed);
    
    if (architectures_passed == architectures_tested) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
