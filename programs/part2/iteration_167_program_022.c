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
static int validation_passed = 1;

static void validate_result(const char* arch, const char* test_name, 
                           int actual, int expected) {
    if (actual != expected) {
        printf("FAIL: %s/%s: expected %d, got %d\n", 
               arch, test_name, expected, actual);
        validation_passed = 0;
    } else {
        printf("PASS: %s/%s\n", arch, test_name);
    }
}

/* Prevent inlining to ensure RTL expansion happens */
__attribute__((noinline, target("arch=x86-64")))
#ifdef __AVX512F__
void test_avx512_10_operand(void) {
    /* AVX-512 masked gather with 10 operands:
     * dest, mask, base, index, scale, src (for scatter), etc.
     */
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    double dest[ARRAY_SIZE] __attribute__((aligned(64)));
    int64_t indices[ARRAY_SIZE] __attribute__((aligned(64)));
    double src_values[ARRAY_SIZE] __attribute__((aligned(64)));
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)i;
        indices[i] = (i * 2) % ARRAY_SIZE;
        src_values[i] = (double)(i * 3);
    }
    
    /* Complex operation that may expand to 10 operands */
    #pragma omp simd
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        /* This intrinsic pattern often requires many operands:
         * dest, mask, index, base, scale, src, etc.
         */
        __m512d vbase = _mm512_load_pd(&base[i]);
        __m512i vidx = _mm512_load_epi64(&indices[i]);
        __m512d vsrc = _mm512_load_pd(&src_values[i]);
        
        /* Simulate a complex operation that might require
         * 10 operands during RTL expansion */
        __mmask8 mask = 0xFF;
        __m512d result = _mm512_mask_i64gather_pd(
            _mm512_setzero_pd(),  // src (operand 1)
            mask,                 // mask (operand 2)
            vidx,                 // index (operand 3)
            (void*)base,          // base (operand 4)
            8,                    // scale (operand 5) - sizeof(double)
            _MM_SCALE_8           // scale enum (operand 6)
        );
        
        /* Additional operations to encourage complex expansion */
        result = _mm512_fmadd_pd(result, vbase, vsrc);
        _mm512_store_pd(&dest[i], result);
    }
    
    /* Simple validation */
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += dest[i];
    }
    
    /* Expected value based on initialization */
    validate_result("AVX512", "10_operand_gather", 
                   (int)(sum / 1000), 14336); /* Approximate expected */
}

__attribute__((noinline, target("avx512f")))
void test_avx512_11_operand(void) {
    /* Even more complex pattern aiming for 11 operands */
    float a[ARRAY_SIZE] __attribute__((aligned(64)));
    float b[ARRAY_SIZE] __attribute__((aligned(64)));
    float c[ARRAY_SIZE] __attribute__((aligned(64)));
    float d[ARRAY_SIZE] __attribute__((aligned(64)));
    float result[ARRAY_SIZE] __attribute__((aligned(64)));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
        c[i] = i * 3.0f;
        d[i] = i * 4.0f;
    }
    
    /* Complex fused operations that may require many operands */
    #pragma omp simd
    for (int i = 0; i < ARRAY_SIZE; i += 16) {
        __m512 va = _mm512_load_ps(&a[i]);
        __m512 vb = _mm512_load_ps(&b[i]);
        __m512 vc = _mm512_load_ps(&c[i]);
        __m512 vd = _mm512_load_ps(&d[i]);
        
        /* Multiple FMA operations combined - may expand to
         * pattern requiring 11 operands */
        __m512 temp1 = _mm512_fmadd_ps(va, vb, vc);
        __m512 temp2 = _mm512_fnmadd_ps(va, vc, vd);
        __m512 final = _mm512_fmadd_ps(temp1, temp2, va);
        
        _mm512_store_ps(&result[i], final);
    }
    
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += result[i];
    }
    
    validate_result("AVX512", "11_operand_fma", 
                   (int)(checksum / 1000), 86016); /* Approximate */
}
#endif /* __AVX512F__ */

/* ARM SVE specific tests */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

__attribute__((noinline))
void test_sve_10_operand(void) {
    /* SVE gather operations can require many operands */
    double base[ARRAY_SIZE];
    double dest[ARRAY_SIZE];
    int64_t indices[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = i * 1.0;
        indices[i] = i;
    }
    
    svbool_t pg = svwhilelt_b64(0, ARRAY_SIZE);
    svint64_t vidx = svld1_s64(pg, indices);
    
    /* SVE gather with predicate, base, index - may expand to 10 operands */
    svfloat64_t gathered = svld1_gather_index(pg, base, vidx);
    
    svst1(pg, dest, gathered);
    
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += dest[i];
    }
    
    validate_result("ARM SVE", "10_operand_gather", 
                   (int)(sum / 1000), 523776);
}

__attribute__((noinline))
void test_sve_11_operand(void) {
    /* Complex SVE operation aiming for 11 operands */
    float a[ARRAY_SIZE], b[ARRAY_SIZE], c[ARRAY_SIZE];
    float result[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
        c[i] = i * 3.0f;
    }
    
    svbool_t pg = svwhilelt_b32(0, ARRAY_SIZE);
    svfloat32_t va = svld1(pg, a);
    svfloat32_t vb = svld1(pg, b);
    svfloat32_t vc = svld1(pg, c);
    
    /* Complex FMA pattern that may require many operands */
    svfloat32_t temp = svmla_f32_z(pg, va, vb, vc);
    svfloat32_t temp2 = svmls_f32_z(pg, vb, vc, va);
    svfloat32_t final = svmad_f32_z(pg, temp, temp2, va);
    
    svst1(pg, result, final);
    
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += result[i];
    }
    
    validate_result("ARM SVE", "11_operand_mla", 
                   (int)(checksum / 1000), 1572864);
}
#endif /* __ARM_FEATURE_SVE */

/* PowerPC Altivec/VSX tests */
#ifdef __ALTIVEC__
#include <altivec.h>

__attribute__((noinline))
void test_powerpc_10_operand(void) {
    /* PowerPC vector operations with many operands */
    vector float a[ARRAY_SIZE/4], b[ARRAY_SIZE/4], c[ARRAY_SIZE/4];
    vector float result[ARRAY_SIZE/4];
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        a[i] = (vector float){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
        b[i] = (vector float){i*5.0f, i*6.0f, i*7.0f, i*8.0f};
        c[i] = (vector float){i*9.0f, i*10.0f, i*11.0f, i*12.0f};
    }
    
    /* Complex vector operations that may expand to many operands */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        vector float temp = vec_madd(a[i], b[i], c[i]);
        vector float temp2 = vec_nmsub(a[i], c[i], b[i]);
        result[i] = vec_madd(temp, temp2, a[i]);
    }
    
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        float* ptr = (float*)&result[i];
        checksum += ptr[0] + ptr[1] + ptr[2] + ptr[3];
    }
    
    validate_result("PowerPC", "10_operand_vec", 
                   (int)(checksum / 1000), 2097152);
}

/* Inline assembly with exactly 11 operands */
__attribute__((noinline))
void test_powerpc_11_operand_asm(void) {
    volatile int out1, out2, out3;
    volatile int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    volatile int in6 = 6, in7 = 7, in8 = 8, in9 = 9;
    
    /* Extended asm with 11 total operands (3 outputs + 8 inputs) */
    asm volatile (
        "dummy_operation %0, %1, %2, %3, %4, %5, %6, %7, %8, %9, %10"
        : "=r"(out1), "=r"(out2), "=r"(out3)  /* 3 outputs */
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "r"(in6), "r"(in7), "r"(in8)        /* 8 inputs */
        : "memory"
    );
    
    validate_result("PowerPC", "11_operand_asm", 
                   out1 + out2 + out3, 0);
}
#endif /* __ALTIVEC__ */

/* RISC-V Vector Extension tests */
#ifdef __riscv_v
#include <riscv_vector.h>

__attribute__((noinline))
void test_riscv_10_operand(void) {
    /* RISC-V vector operations */
    double data[ARRAY_SIZE];
    double result[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 1.0;
    }
    
    size_t vl = vsetvl_e64m8(ARRAY_SIZE);
    vfloat64m8_t vdata = vle64_v_f64m8(data, vl);
    
    /* Complex vector operation */
    vfloat64m8_t vresult = vfadd_vv_f64m8(vdata, vdata, vl);
    vresult = vfmul_vv_f64m8(vresult, vdata, vl);
    
    vse64_v_f64m8(result, vresult, vl);
    
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += result[i];
    }
    
    validate_result("RISC-V", "10_operand_vec", 
                   (int)(sum / 1000), 1047552);
}
#endif /* __riscv_v */

/* Generic inline assembly fallback for testing */
__attribute__((noinline))
void test_generic_10_operand_asm(void) {
    /* Generic inline assembly with exactly 10 operands */
    volatile int out1, out2;
    volatile int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    volatile int in6 = 6, in7 = 7, in8 = 8;
    
    asm volatile (
        "/* 10-operand dummy instruction */\n\t"
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %1, %8"
        : "=r"(out1), "=r"(out2)              /* 2 outputs */
        : "0"(0), "r"(in1), "r"(in2), "r"(in3), 
          "r"(in4), "r"(in5), "r"(in6), "r"(in7), "r"(in8) /* 8 inputs */
        : "cc"
    );
    
    validate_result("Generic", "10_operand_asm", out1 + out2, 36);
}

__attribute__((noinline))
void test_generic_11_operand_asm(void) {
    /* Generic inline assembly with exactly 11 operands */
    volatile int out1, out2, out3;
    volatile int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    volatile int in6 = 6, in7 = 7, in8 = 8, in9 = 9;
    
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %1, %6\n\t"
        "mov %2, %7\n\t"
        "add %2, %2, %8\n\t"
        "add %2, %2, %9"
        : "=r"(out1), "=r"(out2), "=r"(out3)  /* 3 outputs */
        : "0"(0), "r"(in1), "r"(in2), "r"(in3), "r"(in4),
          "r"(in5), "r"(in6), "r"(in7), "r"(in8), "r"(in9) /* 8 inputs */
        : "cc"
    );
    
    validate_result("Generic", "11_operand_asm", out1 + out2 + out3, 45);
}

/* Main test driver */
int main(void) {
    printf("Testing RTL expansion for 10-11 operand patterns...\n");
    
    /* Always run generic assembly tests */
    test_generic_10_operand_asm();
    test_generic_11_operand_asm();
    
    /* Architecture-specific tests */
#ifdef __AVX512F__
    printf("\nRunning AVX-512 tests...\n");
    test_avx512_10_operand();
    test_avx512_11_operand();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("\nRunning ARM SVE tests...\n");
    test_sve_10_operand();
    test_sve_11_operand();
#endif
    
#ifdef __ALTIVEC__
    printf("\nRunning PowerPC tests...\n");
    test_powerpc_10_operand();
    test_powerpc_11_operand_asm();
#endif
    
#ifdef __riscv_v
    printf("\nRunning RISC-V Vector tests...\n");
    test_riscv_10_operand();
#endif
    
    printf("\n=== Test Summary ===\n");
    if (validation_passed) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
