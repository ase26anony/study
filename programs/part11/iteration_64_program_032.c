/* test_optabs_coverage.c - Target GCC's 10/11-operand expansion paths */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== PATTERN 1: Vector shuffles with many elements ========== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline, target("avx512f")))
void test_vector_shuffle_10(volatile int* out) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* 16-element shuffle = 18 total operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Use volatile store to force expansion */
    v16si* ptr = (v16si*)out;
    *ptr = c;
}

__attribute__((noipa, noinline, target("avx512f")))
void test_vector_shuffle_11(volatile long long* out) {
    v8di a = {0,1,2,3,4,5,6,7};
    v8di b = {8,9,10,11,12,13,14,15};
    
    /* 8-element shuffle with mask = 10+ operands */
    v8di c = __builtin_shufflevector(a, b,
        0,8,1,9,2,10,3,11);
    
    v8di* ptr = (v8di*)out;
    *ptr = c;
}
#endif

/* ========== PATTERN 2: x86 gather intrinsics (many operands) ========== */
#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline, target("avx512f,avx512vl")))
void test_gather_intrinsic_10(volatile double* out) {
    __m512d src = _mm512_set1_pd(2.0);
    __m512i index = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __mmask8 mask = 0xFF;
    
    /* __builtin_ia32_gathersiv8df expands to many operands */
    double base[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    __m512d result = _mm512_i64gather_pd(index, base, 8);
    
    _mm512_storeu_pd((double*)out, result);
}

__attribute__((noipa, noinline, target("avx512f")))
void test_gather_intrinsic_11(volatile float* out) {
    __m512 src = _mm512_set1_ps(1.0f);
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xFFFF;
    
    float base[16];
    for (int i = 0; i < 16; i++) base[i] = (float)i;
    
    /* Complex gather with scale, mask, etc. */
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    _mm512_storeu_ps((float*)out, result);
}
#endif

/* ========== PATTERN 3: Atomic operations with many parameters ========== */
__attribute__((noipa, noinline))
void test_atomic_compare_exchange_10(volatile int* out) {
    int expected = 42;
    int desired = 100;
    int* ptr = (int*)out;
    
    /* __atomic_compare_exchange_n expands to many operands */
    __atomic_compare_exchange_n(ptr, &expected, desired, 
                                0, /* weak */
                                __ATOMIC_SEQ_CST, 
                                __ATOMIC_RELAXED);
    
    /* Force use of result */
    if (expected != 42) {
        *out = expected;
    }
}

__attribute__((noipa, noinline))
void test_atomic_compare_exchange_11(volatile long long* out) {
    long long expected = 123456789LL;
    long long desired = 987654321LL;
    long long* ptr = (long long*)out;
    
    /* Full __atomic_compare_exchange with all parameters */
    __atomic_compare_exchange(ptr, &expected, &desired,
                              0, /* weak */
                              __ATOMIC_ACQ_REL,
                              __ATOMIC_ACQUIRE);
    
    *out = expected;
}

/* ========== PATTERN 4: OpenMP SIMD with many clauses ========== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd_10(volatile double* a, volatile double* b, 
                         volatile double* c, int n) {
    #pragma omp simd linear(i:1) aligned(a,b,c:64) \
                simdlen(8) safelen(16) \
                reduction(+:sum) private(tmp)
    for (int i = 0; i < n; i++) {
        double tmp = b[i] * c[i];
        a[i] = tmp + i;
    }
}

__attribute__((noipa, noinline))
void test_openmp_simd_11(volatile float* a, volatile float* b,
                         volatile float* c, volatile float* d,
                         int n) {
    #pragma omp simd linear(i:1) aligned(a,b,c,d:32) \
                simdlen(16) safelen(32) collapse(1) \
                reduction(max:max_val) private(x,y)
    for (int i = 0; i < n; i++) {
        float x = b[i] + c[i];
        float y = x * d[i];
        a[i] = y;
    }
}
#endif

/* ========== PATTERN 5: Inline assembly with many operands ========== */
__attribute__((noipa, noinline))
void test_asm_10_operands(volatile int* out) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* 10-operand template */\n\t"
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "add %9, %10\n\t"
        "mov %%eax, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "eax", "cc"
    );
    
    *out = result;
}

__attribute__((noipa, noinline))
void test_asm_11_operands(volatile long* out) {
    long a = 1, b = 2, c = 3, d = 4, e = 5;
    long f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result;
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand template */\n\t"
        "imul %1, %2\n\t"
        "imul %3, %4\n\t"
        "imul %5, %6\n\t"
        "imul %7, %8\n\t"
        "imul %9, %10\n\t"
        "mov %%rax, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j), "r" (k)
        : "rax", "rdx", "cc"
    );
    
    *out = result;
}

/* ========== PATTERN 6: AArch64-specific multi-register ops ========== */
#ifdef __aarch64__
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_aarch64_multi_reg_10(volatile int64_t* out) {
    /* Use 4-register load intrinsic */
    int64x2x4_t data;
    int64_t ptr[8] = {1,2,3,4,5,6,7,8};
    
    /* vld1q_s64_x4 expands to many operands */
    data = vld1q_s64_x4((const int64_t*)ptr);
    
    /* Store results */
    for (int i = 0; i < 4; i++) {
        vst1q_s64((int64_t*)out + i*2, data.val[i]);
    }
}

__attribute__((noipa, noinline))
void test_aarch64_multi_reg_11(volatile float* out) {
    /* Complex SIMD operation with multiple registers */
    float32x4_t a = vdupq_n_f32(1.0f);
    float32x4_t b = vdupq_n_f32(2.0f);
    float32x4_t c = vdupq_n_f32(3.0f);
    float32x4_t d = vdupq_n_f32(4.0f);
    
    /* FMLA (fused multiply-add) chain */
    float32x4_t r1 = vfmaq_f32(a, b, c);
    float32x4_t r2 = vfmaq_f32(r1, d, a);
    float32x4_t r3 = vfmaq_f32(r2, b, d);
    float32x4_t result = vfmaq_f32(r3, c, b);
    
    vst1q_f32(out, result);
}
#endif

/* ========== Main test driver ========== */
int main(int argc, char* argv[]) {
    /* Use argv[0] to create a "random" seed */
    unsigned seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    control = seed;
    
    /* Allocate test buffers with proper alignment */
    volatile double* dbl_buf = (volatile double*)aligned_alloc(64, 1024);
    volatile float* flt_buf = (volatile float*)aligned_alloc(32, 1024);
    volatile int* int_buf = (volatile int*)aligned_alloc(32, 1024);
    volatile long long* ll_buf = (volatile long long*)aligned_alloc(32, 1024);
    
    if (!dbl_buf || !flt_buf || !int_buf || !ll_buf) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize buffers */
    for (int i = 0; i < 256; i++) {
        if (i < 128) dbl_buf[i] = i * 0.5;
        if (i < 256) flt_buf[i] = i * 0.25f;
        if (i < 256) int_buf[i] = i * 3;
        if (i < 128) ll_buf[i] = i * 7LL;
    }
    
    /* Select test based on seed */
    int test_num = seed % 12;
    
    switch (test_num) {
        case 0:
#ifdef __AVX512F__
            test_vector_shuffle_10(int_buf);
#endif
            break;
        case 1:
#ifdef __AVX512F__
            test_vector_shuffle_11(ll_buf);
#endif
            break;
        case 2:
#ifdef __AVX512F__
            test_gather_intrinsic_10(dbl_buf);
#endif
            break;
        case 3:
#ifdef __AVX512F__
            test_gather_intrinsic_11(flt_buf);
#endif
            break;
        case 4:
            test_atomic_compare_exchange_10(int_buf);
            break;
        case 5:
            test_atomic_compare_exchange_11(ll_buf);
            break;
        case 6:
#ifdef _OPENMP
            test_openmp_simd_10(dbl_buf, dbl_buf+64, dbl_buf+128, 64);
#endif
            break;
        case 7:
#ifdef _OPENMP
            test_openmp_simd_11(flt_buf, flt_buf+64, flt_buf+128, flt_buf+192, 64);
#endif
            break;
        case 8:
            test_asm_10_operands(int_buf);
            break;
        case 9:
            test_asm_11_operands(ll_buf);
            break;
        case 10:
#ifdef __aarch64__
            test_aarch64_multi_reg_10(ll_buf);
#endif
            break;
        case 11:
#ifdef __aarch64__
            test_aarch64_multi_reg_11(flt_buf);
#endif
            break;
    }
    
    /* Compute checksum to ensure execution */
    long long checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (long long)int_buf[i];
        checksum += (long long)ll_buf[i];
        checksum += (long long)dbl_buf[i];
        checksum += (long long)flt_buf[i];
    }
    
    /* Prevent optimization of results */
    use((void*)int_buf);
    use((void*)ll_buf);
    use((void*)dbl_buf);
    use((void*)flt_buf);
    
    printf("Checksum: %lld (test %d)\n", checksum, test_num);
    
    /* Cleanup */
    free((void*)dbl_buf);
    free((void*)flt_buf);
    free((void*)int_buf);
    free((void*)ll_buf);
    
    return 0;
}
