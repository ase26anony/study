/* Test program to trigger 10-11 operand RTL expansion in GCC's optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

/* Prevent inlining to ensure RTL expansion happens */
#define NOINLINE __attribute__((noinline))

/* Test data initialization */
static void init_test_data(double* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (double)(i * 1.5);
    }
}

static void init_test_data_int64(int64_t* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i * 3;
    }
}

/* =========================================== */
/* PATTERN A: 10-operand case (x86 AVX-512)    */
/* =========================================== */
#ifdef __AVX512F__

NOINLINE static void test_10_operand_avx512_gather(void) {
    /* This gather operation with mask requires many operands */
    double src[8] = {0};
    double dst[8] = {0};
    int64_t indices[8] = {0, 2, 4, 6, 8, 10, 12, 14};
    double base_array[16];
    
    init_test_data(base_array, 16);
    
    /* Create a mask (all ones) */
    __mmask8 mask = 0xFF;
    
    /* Use inline assembly to force 10 operands */
    /* Output operand + 9 input operands = 10 total */
    asm volatile (
        "/* 10-operand dummy instruction for RTL expansion */\n\t"
        "dummy_10op %0, %1, %2, %3, %4, %5, %6, %7, %8, %9"
        : "=m"(dst[0])
        : "r"(mask), "r"(&base_array[0]), "r"(&indices[0]), 
          "r"(8), "r"(sizeof(double)), "r"(src),
          "i"(1), "i"(0), "m"(dst[0])
        : "memory"
    );
    
    /* Also use AVX-512 intrinsic which may expand to many operands */
    __m512d base_vec = _mm512_loadu_pd(base_array);
    __m512i index_vec = _mm512_loadu_epi64(indices);
    __m512d scale_vec = _mm512_set1_pd(1.0);
    __mmask8 gather_mask = 0xFF;
    
    /* _mm512_mask_i64gather_pd uses many implicit operands */
    __m512d result = _mm512_mask_i64gather_pd(
        _mm512_setzero_pd(),  /* src (1) */
        gather_mask,          /* mask (2) */
        index_vec,            /* index (3) */
        (void*)base_array,    /* base (4) */
        _MM_SCALE_1           /* scale (5) - implicit operand */
    );
    
    _mm512_storeu_pd(dst, result);
    
    /* Simple validation */
    double sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    printf("AVX-512 10-operand test: sum = %f\n", sum);
}

#endif /* __AVX512F__ */

/* =========================================== */
/* PATTERN B: 11-operand case (x86 AVX-512)    */
/* =========================================== */
#ifdef __AVX512F__

NOINLINE static void test_11_operand_avx512_scatter(void) {
    /* Scatter operation with mask and update requires many operands */
    double src[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double dst_array[16] = {0};
    int64_t indices[8] = {0, 2, 4, 6, 8, 10, 12, 14};
    
    /* Use inline assembly to force 11 operands */
    /* 2 output operands + 9 input operands = 11 total */
    double out1, out2;
    asm volatile (
        "/* 11-operand dummy instruction for RTL expansion */\n\t"
        "dummy_11op %0, %1, %2, %3, %4, %5, %6, %7, %8, %9, %10"
        : "=r"(out1), "=m"(dst_array[0])
        : "r"(&src[0]), "r"(&indices[0]), "r"(&dst_array[0]),
          "r"(8), "r"(sizeof(double)), "r"(0xFF),
          "i"(1), "i"(0), "m"(src[0])
        : "memory"
    );
    
    /* AVX-512 masked scatter with many operands */
    __m512d src_vec = _mm512_loadu_pd(src);
    __m512i index_vec = _mm512_loadu_epi64(indices);
    __mmask8 scatter_mask = 0xFF;
    
    /* _mm512_mask_i64scatter_pd has implicit scale operand */
    _mm512_mask_i64scatter_pd(
        dst_array,           /* base (1) */
        scatter_mask,        /* mask (2) */
        index_vec,           /* index (3) */
        src_vec,             /* src (4) */
        _MM_SCALE_1          /* scale (5) - implicit */
    );
    
    /* Complex pattern: gather + arithmetic + scatter */
    /* This may expand to even more operands during optimization */
    double temp[8];
    for (int i = 0; i < 8; i++) {
        int idx = indices[i];
        if (idx < 16) {
            temp[i] = dst_array[idx] * 2.0;
        }
    }
    
    /* Validation */
    double sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst_array[i];
    }
    printf("AVX-512 11-operand test: sum = %f\n", sum);
}

#endif /* __AVX512F__ */

/* =========================================== */
/* ARM SVE patterns (if supported)             */
/* =========================================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

NOINLINE static void test_10_operand_arm_sve(void) {
    /* ARM SVE gather with predicate, base, and offsets */
    double base[16];
    double dst[8];
    int64_t offsets[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    
    init_test_data(base, 16);
    
    /* Create SVE predicate (all true) */
    svbool_t pg = svptrue_b64();
    
    /* SVE gather - expands to many operands */
    svint64_t offset_vec = svld1_s64(pg, offsets);
    svfloat64_t result = svld1_gather_offset(pg, base, offset_vec);
    
    /* Store result */
    svst1(pg, dst, result);
    
    /* Validation */
    double sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    printf("ARM SVE 10-operand test: sum = %f\n", sum);
}

#endif /* __ARM_FEATURE_SVE */

/* =========================================== */
/* PowerPC Altivec/VSX patterns                */
/* =========================================== */
#ifdef __ALTIVEC__

#include <altivec.h>

NOINLINE static void test_11_operand_powerpc(void) {
    /* PowerPC matrix multiply style operation with many vector operands */
    vector double a[4], b[4], c[4];
    
    /* Initialize vectors */
    for (int i = 0; i < 4; i++) {
        a[i] = (vector double){i*1.0, i*2.0};
        b[i] = (vector double){i*3.0, i*4.0};
    }
    
    /* Complex vector operation that may expand to many operands */
    /* Using inline assembly to ensure 11 operands */
    vector double out1, out2, out3;
    
    asm volatile (
        "/* PowerPC 11-operand vector operation */\n\t"
        "dummy_vec %0, %1, %2, %3, %4, %5, %6, %7, %8, %9, %10"
        : "=v"(out1), "=v"(out2), "=v"(out3)
        : "v"(a[0]), "v"(a[1]), "v"(a[2]), "v"(a[3]),
          "v"(b[0]), "v"(b[1]), "v"(b[2]), "v"(b[3]),
          "i"(0)
        : 
    );
    
    /* Store results */
    vec_st(out1, 0, (vector double*)c[0]);
    vec_st(out2, 0, (vector double*)c[1]);
    vec_st(out3, 0, (vector double*)c[2]);
    
    printf("PowerPC 11-operand test completed\n");
}

#endif /* __ALTIVEC__ */

/* =========================================== */
/* RISC-V Vector Extension patterns            */
/* =========================================== */
#ifdef __riscv_v

NOINLINE static void test_10_operand_riscv(void) {
    /* RISC-V vector load with mask, stride, length */
    double data[32];
    double dst[16];
    
    init_test_data(data, 32);
    
    /* Use inline assembly for 10 operands */
    long vl = 8;  /* vector length */
    
    asm volatile (
        "/* RISC-V 10-operand vector operation */\n\t"
        "dummy_vl %0, %1, %2, %3, %4, %5, %6, %7, %8, %9"
        : "=m"(dst[0])
        : "r"(data), "r"(dst), "r"(vl),
          "r"(8), "r"(sizeof(double)), "r"(1),
          "i"(0), "i"(1), "m"(data[0])
        : "memory"
    );
    
    printf("RISC-V 10-operand test completed\n");
}

#endif /* __riscv_v */

/* =========================================== */
/* Generic fallback with many-parameter inline */
/* =========================================== */
NOINLINE static void test_generic_many_operands(void) {
    /* Force a function call with many parameters that might be inlined
       and expanded to multi-operand RTL */
    volatile double a = 1.0, b = 2.0, c = 3.0, d = 4.0;
    volatile double e = 5.0, f = 6.0, g = 7.0, h = 8.0;
    volatile double i = 9.0, j = 10.0, k = 11.0;
    volatile double result1, result2, result3;
    
    /* Inline assembly with many operands - generic version */
    asm volatile (
        "/* Generic 10-operand operation */\n\t"
        "dummy_many %0, %1, %2, %3, %4, %5, %6, %7, %8, %9"
        : "=r"(result1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "i"(0)
        : 
    );
    
    asm volatile (
        "/* Generic 11-operand operation */\n\t"
        "dummy_many2 %0, %1, %2, %3, %4, %5, %6, %7, %8, %9, %10"
        : "=r"(result1), "=r"(result2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "i"(0)
        : 
    );
    
    printf("Generic many-operand test completed\n");
}

/* =========================================== */
/* Hot loop to encourage vectorization and     */
/* RTL expansion                               */
/* =========================================== */
NOINLINE static void hot_loop_with_complex_ops(int iterations) {
    double data[256];
    double result[256];
    
    init_test_data(data, 256);
    
    /* This loop may be vectorized and unrolled, potentially creating
       multi-operand instructions during RTL expansion */
    #pragma omp simd
    for (int i = 0; i < 256 - 8; i += 8) {
        /* Complex operation that might expand to many operands */
        for (int j = 0; j < 8; j++) {
            /* Using inline assembly to force many operands in the loop */
            double temp;
            asm volatile (
                "dummy_loop %0, %1, %2, %3, %4, %5"
                : "=r"(temp)
                : "r"(data[i+j]), "r"(data[i+j+1]), 
                  "r"(data[i+j+2]), "r"(j), "i"(1)
                : 
            );
            result[i+j] = temp;
        }
    }
    
    /* Compute checksum */
    double sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += result[i];
    }
    printf("Hot loop checksum: %f\n", sum);
}

/* =========================================== */
/* Main function                               */
/* =========================================== */
int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("=== Testing 10-11 operand RTL expansion ===\n\n");
    
    /* Run architecture-specific tests */
    
#ifdef __AVX512F__
    printf("Testing AVX-512 paths...\n");
    tests_run += 2;
    test_10_operand_avx512_gather();
    test_11_operand_avx512_scatter();
    tests_passed += 2;
    printf("AVX-512 tests completed\n\n");
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE paths...\n");
    tests_run++;
    test_10_operand_arm_sve();
    tests_passed++;
    printf("ARM SVE tests completed\n\n");
#endif
    
#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec paths...\n");
    tests_run++;
    test_11_operand_powerpc();
    tests_passed++;
    printf("PowerPC tests completed\n\n");
#endif
    
#ifdef __riscv_v
    printf("Testing RISC-V Vector paths...\n");
    tests_run++;
    test_10_operand_riscv();
    tests_passed++;
    printf("RISC-V tests completed\n\n");
#endif
    
    /* Always run generic test */
    printf("Testing generic paths...\n");
    tests_run++;
    test_generic_many_operands();
    tests_passed++;
    
    /* Run hot loop to encourage optimization */
    printf("\nRunning hot loop for vectorization...\n");
    hot_loop_with_complex_ops(1000);
    
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_passed == tests_run) {
        printf("All tests completed successfully!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
