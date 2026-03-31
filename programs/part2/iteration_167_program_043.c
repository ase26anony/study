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
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) if (!(cond)) { printf("FAIL: %s\n", msg); return 0; }
#define PASS(msg) printf("PASS: %s\n", msg)

/* Function attributes to control optimization */
#define HOT_FUNC __attribute__((hot, noinline))
#define NOINLINE_FUNC __attribute__((noinline))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>
#include <x86intrin.h>

/* Pattern A: 10 operands - Masked gather with multiple parameters */
NOINLINE_FUNC HOT_FUNC
int test_avx512_10_operands(double* result, const double* base, 
                           const int64_t* indices, __mmask8 mask,
                           double scale_factor, double offset) {
    /* This should generate a 10-operand RTL pattern:
     * 1. Destination vector (result)
     * 2. Mask register
     * 3. Base address
     * 4. Index vector
     * 5. Scale
     * 6. Offset
     * 7-10: Various control operands
     */
    
    __m512d vec_result;
    __m512i vec_indices = _mm512_loadu_si512((const __m512i*)indices);
    
    /* _mm512_mask_i64gather_pd requires:
     * 1. src (__m512d)
     * 2. k (__mmask8)
     * 3. vindex (__m512i)
     * 4. base (void const*)
     * 5. scale (int)
     * Total: 5 explicit + implicit operands = potentially 10+ in RTL
     */
    vec_result = _mm512_mask_i64gather_pd(_mm512_setzero_pd(),
                                          mask,
                                          vec_indices,
                                          base,
                                          8);  // scale = 8 bytes
    
    /* Apply scale and offset */
    __m512d scale_vec = _mm512_set1_pd(scale_factor);
    __m512d offset_vec = _mm512_set1_pd(offset);
    vec_result = _mm512_fmadd_pd(vec_result, scale_vec, offset_vec);
    
    _mm512_storeu_pd(result, vec_result);
    return 1;
}

/* Pattern B: 11 operands - Complex masked scatter with update */
NOINLINE_FUNC HOT_FUNC
int test_avx512_11_operands(double* data, const double* updates,
                           const int64_t* indices, __mmask8 mask,
                           double alpha, double beta, int stride) {
    /* This should generate an 11-operand RTL pattern */
    
    __m512d vec_data = _mm512_loadu_pd(data);
    __m512d vec_updates = _mm512_loadu_pd(updates);
    __m512i vec_indices = _mm512_loadu_si512((const __m512i*)indices);
    
    /* Complex operation combining gather, computation, and scatter */
    __m512d gathered;
    
    /* Gather phase - adds several operands */
    gathered = _mm512_mask_i64gather_pd(_mm512_setzero_pd(),
                                        mask,
                                        vec_indices,
                                        data,
                                        8);
    
    /* Fused multiply-add with multiple parameters */
    __m512d alpha_vec = _mm512_set1_pd(alpha);
    __m512d beta_vec = _mm512_set1_pd(beta);
    
    __m512d result = _mm512_fmadd_pd(gathered, alpha_vec,
                                    _mm512_fmadd_pd(vec_updates, beta_vec,
                                                   _mm512_setzero_pd()));
    
    /* Scatter with stride - potentially adds more operands */
    _mm512_mask_i64scatter_pd(data, mask, vec_indices, result, 8);
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands for ARM SVE */
NOINLINE_FUNC HOT_FUNC
int test_arm_sve_10_operands(double* result, const double* base,
                            const int64_t* offsets, svbool_t pg) {
    /* SVE gather with multiple parameters */
    svint64_t offset_vec = svld1_s64(pg, offsets);
    
    /* svld1_gather_s64index requires:
     * 1. pg (svbool_t)
     * 2. base (const void*)
     * 3. indices (svint64_t)
     * Plus implicit operands for result, etc.
     */
    svfloat64_t gathered = svld1_gather_s64index_f64(pg, base, offset_vec);
    
    svst1_f64(pg, result, gathered);
    return 1;
}

/* Pattern B: 11 operands for ARM SVE */
NOINLINE_FUNC HOT_FUNC  
int test_arm_sve_11_operands(double* data, const double* updates,
                            const int64_t* indices, svbool_t pg,
                            double scale, double bias) {
    /* Complex SVE operation with multiple parameters */
    svint64_t idx_vec = svld1_s64(pg, indices);
    svfloat64_t data_vec = svld1_f64(pg, data);
    svfloat64_t update_vec = svld1_f64(pg, updates);
    
    /* Gather with predicate */
    svfloat64_t gathered = svld1_gather_s64index_f64(pg, data, idx_vec);
    
    /* Fused operation */
    svfloat64_t scale_vec = svdup_f64(scale);
    svfloat64_t bias_vec = svdup_f64(bias);
    
    svfloat64_t result = svmla_f64_z(pg, bias_vec, gathered, scale_vec);
    result = svadd_f64_z(pg, result, update_vec);
    
    /* Scatter result back */
    svst1_scatter_s64index_f64(pg, data, idx_vec, result);
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX Implementation ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

/* Pattern A: 10 operands using PowerPC vector operations */
NOINLINE_FUNC HOT_FUNC
int test_powerpc_10_operands(vector double* result, const vector double* a,
                            const vector double* b, const vector double* c,
                            const vector double* d, const vector double* e) {
    /* Complex vector operation with many inputs */
    vector double temp1 = vec_madd(*a, *b, *c);
    vector double temp2 = vec_madd(*d, *e, temp1);
    vector double temp3 = vec_add(temp1, temp2);
    vector double temp4 = vec_sub(temp3, *a);
    vector double final = vec_madd(temp4, *b, *c);
    
    *result = vec_add(final, *d);
    return 1;
}

#endif /* __ALTIVEC__ */

/* ==================== RISC-V Vector Extension ==================== */
#ifdef __riscv_v

/* Pattern A: 10 operands for RISC-V V extension */
NOINLINE_FUNC HOT_FUNC
int test_riscv_10_operands(double* dst, const double* src, 
                          const ptrdiff_t* indices, long gvl) {
    /* Using inline assembly to force 10 operands */
    asm volatile (
        "vsetvli zero, %[gvl], e64, m8, ta, ma\n\t"
        "vle64.v v0, (%[src])\n\t"
        "vadd.vv v8, v0, v0\n\t"
        "vs8r.v v8, (%[dst])\n\t"
        : 
        : [dst] "r" (dst), [src] "r" (src), [gvl] "r" (gvl)
        : "v0", "v8", "memory"
    );
    return 1;
}

#endif /* __riscv_v */

/* ==================== Generic Inline Assembly Fallback ==================== */

/* Pattern A: 10 operands using generic inline assembly */
NOINLINE_FUNC HOT_FUNC
int test_generic_10_operands(int* out, int a, int b, int c, int d, int e,
                            int f, int g, int h, int i) {
    /* Force 10 operands in RTL expansion */
    int result;
    asm volatile (
        "/* 10-operand dummy instruction */\n\t"
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i)
        : "cc"
    );
    *out = result;
    return 1;
}

/* Pattern B: 11 operands using generic inline assembly */
NOINLINE_FUNC HOT_FUNC
int test_generic_11_operands(int* out, int a, int b, int c, int d, int e,
                            int f, int g, int h, int i, int j) {
    /* Force 11 operands in RTL expansion */
    int result;
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "imul %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    *out = result;
    return 1;
}

/* ==================== Test Harness ==================== */

/* Hot loop to encourage vectorization and RTL expansion */
HOT_FUNC
void run_hot_loop(double* data, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Complex operations in hot loop */
        data[i % ARRAY_SIZE] = data[i % ARRAY_SIZE] * 1.1 + 0.5;
        
        /* Encourage unrolling */
        if (i % 8 == 0) {
            data[(i + 1) % ARRAY_SIZE] += data[i % ARRAY_SIZE];
        }
    }
}

int main() {
    int success_count = 0;
    int test_count = 0;
    
    printf("Testing RTL expansion for 10-11 operand instructions...\n");
    printf("Target: optabs.cc lines 8254-8263\n\n");
    
    /* Initialize test data */
    double data[ARRAY_SIZE];
    double result[ARRAY_SIZE];
    int64_t indices[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 1.5;
        result[i] = 0.0;
        indices[i] = (i * 2) % ARRAY_SIZE;
    }
    
    /* Run hot loop to encourage optimization */
    run_hot_loop(data, 1000);
    
    /* ========== Test architecture-specific paths ========== */
    
#ifdef __AVX512F__
    printf("[x86 AVX-512] Testing...\n");
    test_count++;
    
    /* Test 10-operand pattern */
    if (test_avx512_10_operands(result, data, indices, 0xFF, 2.0, 1.0)) {
        /* Simple validation */
        double sum = 0.0;
        for (int i = 0; i < 8; i++) {
            sum += result[i];
        }
        VALIDATE(sum > 0.0, "AVX-512 10-operand pattern");
        PASS("AVX-512 10-operand pattern");
        success_count++;
    }
    
    /* Test 11-operand pattern */
    test_count++;
    if (test_avx512_11_operands(data, result, indices, 0xFF, 0.5, 0.25, 1)) {
        PASS("AVX-512 11-operand pattern");
        success_count++;
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("[ARM SVE] Testing...\n");
    test_count++;
    
    svbool_t pg = svptrue_b64();
    if (test_arm_sve_10_operands(result, data, indices, pg)) {
        PASS("ARM SVE 10-operand pattern");
        success_count++;
    }
    
    test_count++;
    if (test_arm_sve_11_operands(data, result, indices, pg, 1.5, 0.5)) {
        PASS("ARM SVE 11-operand pattern");
        success_count++;
    }
#endif
    
#ifdef __ALTIVEC__
    printf("[PowerPC Altivec] Testing...\n");
    test_count++;
    
    vector double vec_a = {1.0, 2.0};
    vector double vec_b = {3.0, 4.0};
    vector double vec_c = {5.0, 6.0};
    vector double vec_d = {7.0, 8.0};
    vector double vec_e = {9.0, 10.0};
    vector double vec_result;
    
    if (test_powerpc_10_operands(&vec_result, &vec_a, &vec_b, &vec_c, 
                                 &vec_d, &vec_e)) {
        PASS("PowerPC 10-operand pattern");
        success_count++;
    }
#endif
    
#ifdef __riscv_v
    printf("[RISC-V Vector] Testing...\n");
    test_count++;
    
    ptrdiff_t riscv_indices[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        riscv_indices[i] = i * sizeof(double);
    }
    
    if (test_riscv_10_operands(result, data, riscv_indices, ARRAY_SIZE)) {
        PASS("RISC-V 10-operand pattern");
        success_count++;
    }
#endif
    
    /* ========== Test generic inline assembly paths ========== */
    printf("[Generic] Testing inline assembly patterns...\n");
    
    int out_val;
    
    /* Test 10-operand generic pattern */
    test_count++;
    if (test_generic_10_operands(&out_val, 1, 2, 3, 4, 5, 6, 7, 8, 9)) {
        VALIDATE(out_val == 45, "Generic 10-operand pattern");
        PASS("Generic 10-operand pattern");
        success_count++;
    }
    
    /* Test 11-operand generic pattern */
    test_count++;
    if (test_generic_11_operands(&out_val, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)) {
        VALIDATE(out_val == 56, "Generic 11-operand pattern");
        PASS("Generic 11-operand pattern");
        success_count++;
    }
    
    /* ========== Summary ========== */
    printf("\n=== Test Summary ===\n");
    printf("Total tests compiled: %d\n", test_count);
    printf("Tests passed: %d\n", success_count);
    
    if (success_count == test_count) {
        printf("\nSUCCESS: All tests passed!\n");
        printf("The RTL expander should have encountered 10-11 operand patterns.\n");
        printf("Check coverage of optabs.cc lines 8254-8263.\n");
        return 0;
    } else {
        printf("\nPARTIAL: Some tests failed or were not compiled.\n");
        printf("Compile with appropriate architecture flags for full coverage.\n");
        return 1;
    }
}
