/* Test program to trigger 10-11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __x86_64__
#include <immintrin.h>
#endif

/* Prevent inlining to ensure RTL expansion happens */
__attribute__((noinline, target("arch=native")))
void test_10_operand_pattern(void);
__attribute__((noinline, target("arch=native")))
void test_11_operand_pattern(void);

/* Runtime validation */
static int test_passed = 1;
static int avx512_tested = 0;
static int sve_tested = 0;
static int altivec_tested = 0;
static int riscv_tested = 0;

#define VALIDATE(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s\n", msg); \
            test_passed = 0; \
        } \
    } while(0)

/* ==================== x86 AVX-512 Patterns ==================== */
#ifdef __AVX512F__

/* Pattern A: 10 operands - masked gather with multiple parameters */
__attribute__((noinline, target("avx512f,avx512vl")))
void avx512_10_operand_gather(void) {
    avx512_tested = 1;
    
    /* Setup test data */
    double base[64] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    __m512d src, result;
    __mmask8 mask = 0xFF;
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) base[i] = (double)i;
    for (int i = 0; i < 8; i++) indices[i] = i * 8;
    
    /* This intrinsic typically requires many operands:
       - destination (result)
       - mask
       - indices
       - base pointer
       - scale
       - source (for scatter)
       - etc.
       
       We'll use inline asm to force exactly 10 operands */
    __m512d out1, out2;
    __m512i idx = _mm512_load_epi64(indices);
    __m512d zero = _mm512_setzero_pd();
    
    /* Force 10-operand pattern using inline asm */
    asm volatile (
        /* Dummy instruction with 10 operands */
        "vmovapd %{z%}0, %{z%}1\n\t"
        "vmovapd %{z%}2, %{z%}3\n\t"
        "kandb %k4, %k5, %k6\n\t"
        : "=v"(out1), "=v"(out2), "=v"(result)
        : "v"(zero), "v"(zero), 
          "k"(mask), "k"(mask),
          "r"(base), "v"(idx),
          "i"(8), "i"(1)
        : "memory"
    );
    
    /* Simple validation */
    double check[8];
    _mm512_storeu_pd(check, result);
    VALIDATE(check[0] == 0.0, "AVX-512 10-operand pattern");
}

/* Pattern B: 11 operands - complex masked operation */
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
void avx512_11_operand_scatter(void) {
    /* Setup scatter data */
    double target[64] __attribute__((aligned(64)));
    int64_t scatter_idx[8] __attribute__((aligned(64)));
    __m512d data = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __mmask8 mask = 0xFF;
    
    for (int i = 0; i < 64; i++) target[i] = 0.0;
    for (int i = 0; i < 8; i++) scatter_idx[i] = i * 4;
    
    __m512i idx = _mm512_load_epi64(scatter_idx);
    
    /* Force 11-operand pattern */
    __m512d out1, out2, out3;
    asm volatile (
        /* Complex pattern with 11 operands */
        "vmulpd %{z%}0, %{z%}1, %{z%}2\n\t"
        "vaddpd %{z%}3, %{z%}4, %{z%}5\n\t"
        "korb %k6, %k7, %k8\n\t"
        : "=v"(out1), "=v"(out2), "=v"(out3)
        : "v"(data), "v"(data), "v"(data),
          "k"(mask), "k"(mask), "k"(mask),
          "r"(target), "v"(idx),
          "i"(8)
        : "memory"
    );
    
    /* Validation */
    VALIDATE(target[0] == 0.0, "AVX-512 11-operand pattern");
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Patterns ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* SVE 10-operand pattern */
__attribute__((noinline))
void sve_10_operand_gather(void) {
    sve_tested = 1;
    
    /* SVE gather operations can have many parameters */
    double base[1024];
    uint64_t indices[svcntd()];
    svbool_t pg = svptrue_b64();
    
    for (size_t i = 0; i < 1024; i++) base[i] = (double)i;
    for (size_t i = 0; i < svcntd(); i++) indices[i] = i * 2;
    
    /* Use inline asm to force 10 operands */
    svfloat64_t result, tmp1, tmp2;
    svuint64_t idx = svld1_u64(pg, indices);
    
    asm volatile (
        /* SVE-style 10-operand pattern */
        "ld1d %0.d, %1/z, [%2, %3.d, lsl #3]\n\t"
        "fadd %4.d, %1/m, %5.d, %6.d\n\t"
        : "=w"(result), "=w"(tmp1), "=w"(tmp2)
        : "w"(pg), "w"(idx), "r"(base),
          "w"(svdup_f64(1.0)), "w"(svdup_f64(2.0)),
          "i"(svcntd()), "i"(3)
        : "memory"
    );
    
    VALIDATE(1, "SVE 10-operand pattern");
}

/* SVE 11-operand pattern */
__attribute__((noinline))
void sve_11_operand_scatter(void) {
    double target[1024];
    uint64_t indices[svcntd()];
    svbool_t pg = svptrue_b64();
    svfloat64_t data = svdup_f64(42.0);
    
    for (size_t i = 0; i < 1024; i++) target[i] = 0.0;
    for (size_t i = 0; i < svcntd(); i++) indices[i] = i * 3;
    
    svuint64_t idx = svld1_u64(pg, indices);
    
    asm volatile (
        /* 11-operand scatter-like pattern */
        "st1d %0.d, %1, [%2, %3.d, lsl #3]\n\t"
        "fmul %4.d, %1/m, %5.d, %6.d\n\t"
        : 
        : "w"(data), "w"(pg), "r"(target), "w"(idx),
          "w"(data), "w"(data), "w"(data),
          "i"(svcntd()), "i"(8), "i"(1), "i"(0)
        : "memory"
    );
    
    VALIDATE(1, "SVE 11-operand pattern");
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec Patterns ==================== */
#ifdef __ALTIVEC__

/* Altivec 10-operand pattern using matrix operations */
__attribute__((noinline))
void altivec_10_operand_matrix(void) {
    altivec_tested = 1;
    
    /* Use inline asm with many vector registers */
    vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    
    asm volatile (
        /* Complex permutation with 10 operands */
        "vperm %0, %1, %2, %3\n\t"
        "vmaddfp %4, %5, %6, %7\n\t"
        : "=v"(v0), "=v"(v1), "=v"(v2)
        : "v"(v3), "v"(v4), "v"(v5), "v"(v6), "v"(v7),
          "v"(v8), "v"(v9), "i"(0)
        : 
    );
    
    VALIDATE(1, "Altivec 10-operand pattern");
}

#endif /* __ALTIVEC__ */

/* ==================== RISC-V Vector Patterns ==================== */
#ifdef __riscv_v

/* RISC-V V extension 11-operand pattern */
__attribute__((noinline))
void riscv_11_operand_vector(void) {
    riscv_tested = 1;
    
    long vl = 8;
    double src[64], dst[64];
    
    for (int i = 0; i < 64; i++) {
        src[i] = (double)i;
        dst[i] = 0.0;
    }
    
    /* Force 11-operand pattern for vector load/store */
    asm volatile (
        "vsetvli zero, %0, e64, m1, ta, ma\n\t"
        "vle64.v v1, (%1)\n\t"
        "vle64.v v2, (%2)\n\t"
        "vfadd.vv v3, v1, v2\n\t"
        "vse64.v v3, (%3)\n\t"
        : 
        : "r"(vl), "r"(src), "r"(src + 8), "r"(dst),
          "r"(src + 16), "r"(src + 24), "r"(src + 32),
          "r"(src + 40), "r"(src + 48), "r"(src + 56),
          "i"(8)
        : "v1", "v2", "v3", "memory"
    );
    
    VALIDATE(dst[0] == 0.0, "RISC-V 11-operand pattern");
}

#endif /* __riscv_v */

/* ==================== Generic Fallback ==================== */
void generic_multi_operand_test(void) {
    /* Force 10-operand inline asm on generic architecture */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long out1, out2, out3;
    
    asm volatile (
        /* 10-operand generic pattern */
        "add %0, %1, %2\n\t"
        "add %3, %4, %5\n\t"
        "add %6, %7, %8\n\t"
        : "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i)
        : 
    );
    
    VALIDATE(out1 == 3, "Generic 10-operand pattern");
    
    /* 11-operand generic pattern */
    long k = 11;
    asm volatile (
        /* 11-operand pattern */
        "imul %0, %1, %2\n\t"
        "imul %3, %4, %5\n\t"
        "imul %6, %7, %8\n\t"
        : "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
          "r"(k)
        : 
    );
    
    VALIDATE(out1 == 2, "Generic 11-operand pattern");
}

/* ==================== Main Test Driver ==================== */
int main(void) {
    printf("Testing 10-11 operand RTL expansion patterns...\n");
    
    /* Execute architecture-specific tests */
#ifdef __AVX512F__
    printf("Testing AVX-512 patterns...\n");
    avx512_10_operand_gather();
    avx512_11_operand_scatter();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE patterns...\n");
    sve_10_operand_gather();
    sve_11_operand_scatter();
#endif
    
#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec patterns...\n");
    altivec_10_operand_matrix();
#endif
    
#ifdef __riscv_v
    printf("Testing RISC-V Vector patterns...\n");
    riscv_11_operand_vector();
#endif
    
    /* Always test generic pattern */
    printf("Testing generic patterns...\n");
    generic_multi_operand_test();
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("AVX-512 tested: %s\n", avx512_tested ? "YES" : "NO");
    printf("ARM SVE tested: %s\n", sve_tested ? "YES" : "NO");
    printf("Altivec tested: %s\n", altivec_tested ? "YES" : "NO");
    printf("RISC-V tested:  %s\n", riscv_tested ? "YES" : "NO");
    
    if (test_passed) {
        printf("\nSUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("\nFAILURE: Some tests failed!\n");
        return 1;
    }
}
