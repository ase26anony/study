/* test_optabs_coverage.c - Cover 10/11-operand expansion cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== Pattern 1: Vector shuffles with many operands ========== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle requiring many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 31, 30, 29, 28, 11, 10, 9, 8, 27, 26, 25, 24);
    
    /* Use AVX-512 specific built-in that may need many operands */
    v16sf fa = {0.0f};
    v16sf fb = {1.0f};
    v16sf fc = __builtin_ia32_blendmps512_mask(fa, fb, 
        (__mmask16)0xAAAA, (v16sf)_mm512_setzero_si512());
    
    memcpy((void*)result, &c, sizeof(c));
    memcpy((void*)(result + 16), &d, sizeof(d));
    memcpy((void*)(result + 32), &fc, sizeof(fc));
}
#endif

/* ========== Pattern 2: Gather instructions (x86-specific) ========== */
#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noipa, noinline))
void test_gather_instructions(volatile int* result) {
    double base[64] = {0};
    int indices[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    __m512d src = _mm512_set1_pd(1.0);
    __mmask8 mask = 0xFF;
    
    /* __builtin_ia32_gathersiv8df takes many arguments:
       src, base, indices, scale, mask, src2 */
    __m512d gathered = _mm512_i32gather_pd(
        _mm256_loadu_si256((__m256i*)indices),
        base,
        8
    );
    
    /* Another gather variant */
    __m512d gathered2 = _mm512_i32gather_pd(
        _mm256_set_epi32(7,6,5,4,3,2,1,0),
        base + 32,
        8
    );
    
    _mm512_storeu_pd((double*)result, gathered);
    _mm512_storeu_pd((double*)(result + 8), gathered2);
}
#endif

/* ========== Pattern 3: Atomic operations with many parameters ========== */
__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    volatile int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0; /* strong */
    
    /* __atomic_compare_exchange expands with many operands */
    int success = __atomic_compare_exchange(
        &atomic_var,
        &expected,
        &desired,
        weak,  /* weak */
        __ATOMIC_SEQ_CST,
        __ATOMIC_RELAXED
    );
    
    /* Another atomic with different ordering */
    int val = 50;
    __atomic_exchange(&atomic_var, &val, &val, __ATOMIC_ACQ_REL);
    
    result[0] = success;
    result[1] = atomic_var;
    result[2] = expected;
}

/* ========== Pattern 4: OpenMP SIMD with many clauses ========== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    #define N 128
    int a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) \
                simdlen(16) safelen(32) \
                reduction(+:c[0:N]) private(i)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] * 3;
    }
    
    /* Another loop with different clauses */
    #pragma omp simd collapse(2) aligned(a:32) \
                linear(k:2) lastprivate(j)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int k = i * 8 + j;
            a[k] = c[k] * 2;
        }
    }
    
    memcpy((void*)result, c, sizeof(c));
    memcpy((void*)(result + N), a, sizeof(a));
}
#endif

/* ========== Pattern 5: Inline assembly with many operands ========== */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Custom 10-operand operation */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* Custom 11-operand operation */\n\t"
        "mul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
}

/* ========== Pattern 6: AArch64-specific multi-register operations ========== */
#ifdef __aarch64__
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_aarch64_multi_reg(volatile int* result) {
    /* Use NEON types that may expand to multi-operand instructions */
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    
    /* Complex sequence that may use many operands */
    int32x4_t r1 = vmlaq_s32(a, b, c);
    int32x4_t r2 = vmlsq_s32(d, a, b);
    
    /* Table lookup with multiple registers */
    uint8x16_t table1 = vdupq_n_u8(1);
    uint8x16_t table2 = vdupq_n_u8(2);
    uint8x16_t table3 = vdupq_n_u8(3);
    uint8x16_t indices = vdupq_n_u8(0);
    
    uint8x16_t tbl_result = vqtbl3q_u8(
        vcombine_u8x3(vget_low_u8(table1), vget_low_u8(table2), vget_low_u8(table3)),
        indices
    );
    
    vst1q_s32((int32_t*)result, r1);
    vst1q_s32((int32_t*)(result + 4), r2);
    vst1q_u8((uint8_t*)(result + 8), tbl_result);
}
#endif

/* ========== Main execution flow ========== */
int main(int argc, char *argv[]) {
    /* Use argv[0] to create a seed for volatile control */
    unsigned seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    control = seed;
    
    /* Allocate result buffer */
    volatile int* results = (volatile int*)malloc(4096 * sizeof(int));
    if (!results) return 1;
    
    /* Initialize results */
    for (int i = 0; i < 4096; i++) {
        results[i] = i;
    }
    
    /* Execute different patterns based on seed */
    int pattern = seed % 6;
    
    switch (pattern) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle(results);
            #endif
            break;
        case 1:
            #ifdef __AVX512F__
            test_gather_instructions(results);
            #endif
            break;
        case 2:
            test_atomic_operations(results);
            break;
        case 3:
            #ifdef _OPENMP
            test_openmp_simd(results);
            #endif
            break;
        case 4:
            test_multi_operand_asm(results);
            break;
        case 5:
            #ifdef __aarch64__
            test_aarch64_multi_reg(results);
            #endif
            break;
    }
    
    /* Compute checksum to ensure execution */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {  /* Check first 256 elements */
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    /* Prevent optimization of results */
    use((void*)results);
    
    printf("Checksum: %08x\n", checksum);
    printf("Control: %d\n", control);
    
    free((void*)results);
    return 0;
}

/* Dummy implementation of external function */
void use(void* ptr) {
    /* Prevent optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}
