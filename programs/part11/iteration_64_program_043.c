/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger the 10 and 11 operand cases in optabs.cc
 * by using various GCC features that generate multi-operand instructions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile variables to prevent dead code elimination */
static volatile int volatile_seed = 0;
static volatile int volatile_result = 0;

/* ========== Strategy 1: Vector Extensions with Many Operands ========== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 types for maximum operand count */
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle_10_operands(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with many indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23);
    
    /* Another complex shuffle pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 11, 10, 9, 8, 31, 30, 29, 28, 27, 26, 25, 24);
    
    /* Use AVX-512 specific built-ins that take many arguments */
    __m512i v1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i v2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i v3 = _mm512_add_epi32(v1, v2);
    
    /* AVX-512 gather instruction - often expands to many operands */
    int base[64] = {0};
    __m512i indices = _mm512_set_epi32(0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60);
    __mmask16 mask = 0xFFFF;
    __m512i gathered = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), 
                                                  mask, indices, base, 4);
    
    volatile_result += c[0] + d[0] + _mm512_extract_epi32(v3, 0) + 
                      _mm512_extract_epi32(gathered, 0);
    use(&c);
}

/* AVX-512 blend with many operands */
__attribute__((noipa, noinline))
void test_avx512_blend(void) {
    __m512d a = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_set_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    __mmask8 mask = 0xAA; /* 10101010 binary */
    
    /* Blend operation with mask - may use many operands */
    __m512d c = _mm512_mask_blend_pd(mask, a, b);
    
    /* Complex permute operation */
    __m512d d = _mm512_permutexvar_pd(_mm512_set_epi64(7,6,5,4,3,2,1,0), c);
    
    volatile_result += (int)(_mm512_reduce_add_pd(d) * 100);
    use(&c);
}
#endif

/* ========== Strategy 2: Target-Specific Built-ins ========== */

#ifdef __x86_64__
/* x86 gather built-ins - known to have many operands */
__attribute__((noipa, noinline))
void test_x86_gather(void) {
    /* Using double type gather which often requires many operands */
    double base[64];
    for (int i = 0; i < 64; i++) base[i] = i * 1.5;
    
    /* Complex gather pattern - may trigger 10+ operand expansion */
    __m256i idx = _mm256_set_epi32(7,6,5,4,3,2,1,0);
    __m256d src = _mm256_set1_pd(0.0);
    __mmask8 mask = 0xFF;
    
    /* This built-in often expands to many operands */
    __m256d result = _mm256_mask_i32gather_pd(src, base, idx, 
                                             _mm256_castsi256_pd(_mm256_set1_epi64x(0xFFFFFFFFFFFFFFFF)), 8);
    
    volatile_result += (int)(result[0] * 100);
    use(&result);
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>

/* AArch64 multi-register load/store */
__attribute__((noipa, noinline))
void test_aarch64_multi_reg(void) {
    int32_t data[32];
    for (int i = 0; i < 32; i++) data[i] = i;
    
    /* Load multiple registers - may expand to many operands */
    int32x4x4_t v = vld1q_s32_x4(data);
    int32x4x2_t w = vld1q_s32_x2(data + 16);
    
    /* Complex operations on multiple registers */
    int32x4_t sum1 = vaddq_s32(v.val[0], v.val[1]);
    int32x4_t sum2 = vaddq_s32(v.val[2], v.val[3]);
    int32x4_t sum3 = vaddq_s32(sum1, sum2);
    int32x4_t sum4 = vaddq_s32(sum3, w.val[0]);
    int32x4_t final_sum = vaddq_s32(sum4, w.val[1]);
    
    volatile_result += vgetq_lane_s32(final_sum, 0);
    use(&v);
}
#endif

/* ========== Strategy 3: Atomic Operations ========== */

__attribute__((noipa, noinline))
void test_atomic_operations(void) {
    intptr_t atomic_var = 0;
    intptr_t expected = 0;
    intptr_t desired = 42;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                           0, /* weak */
                                           __ATOMIC_SEQ_CST,
                                           __ATOMIC_RELAXED);
    
    /* Another atomic with many operands */
    intptr_t fetch_add_result = __atomic_fetch_add(&atomic_var, 10, __ATOMIC_SEQ_CST);
    
    /* Atomic exchange with memory order */
    intptr_t exchange_result = __atomic_exchange_n(&atomic_var, 100, __ATOMIC_SEQ_CST);
    
    volatile_result += success + fetch_add_result + exchange_result;
    use(&atomic_var);
}

/* ========== Strategy 4: OpenMP SIMD with Many Clauses ========== */

#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd_complex(void) {
    #define N 1024
    alignas(64) double a[N], b[N], c[N], d[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0;
        b[i] = i * 2.0;
        c[i] = i * 0.5;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                simdlen(8) safelen(16) reduction(+:volatile_result)
    for (int i = 0; i < N; i++) {
        d[i] = a[i] * b[i] + c[i];
        volatile_result += (int)d[i];
    }
    
    /* Another complex SIMD loop */
    #pragma omp simd collapse(2) simdlen(4)
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            a[i*32 + j] = b[i*32 + j] * c[i*32 + j];
        }
    }
    
    use(a); use(b); use(c); use(d);
}
#endif

/* ========== Strategy 5: Inline Assembly with Many Operands ========== */

__attribute__((noipa, noinline))
void test_many_operand_asm(void) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm template %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (out1)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), 
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand asm template */\n\t"
        "mov %0, %1\n\t"
        "imul %0, %2\n\t"
        "add %0, %3\n\t"
        "sub %0, %4\n\t"
        "and %0, %5\n\t"
        "or %0, %6\n\t"
        "xor %0, %7\n\t"
        "shl %0, %8\n\t"
        "shr %0, %9\n\t"
        "add %0, %10"
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    volatile_result += out1 + out2;
    use(&out1);
}

/* ========== Strategy 6: Complex Vector Operations ========== */

__attribute__((noipa, noinline))
void test_complex_vector_ops(void) {
    /* Use vector extensions without specific intrinsics */
    typedef int v8si __attribute__((vector_size(32)));
    typedef float v8sf __attribute__((vector_size(32)));
    
    v8si va = {1,2,3,4,5,6,7,8};
    v8si vb = {8,7,6,5,4,3,2,1};
    v8si vc = {2,2,2,2,2,2,2,2};
    
    /* Complex sequence of vector operations */
    v8si v1 = va + vb;
    v8si v2 = v1 * vc;
    v8si v3 = v2 >> 1;
    v8si v4 = v3 & vb;
    v8si v5 = v4 | va;
    
    /* Conditional select operation - may expand to many operands */
    v8si mask = va > vb;
    v8si v6 = mask ? v5 : v2;
    
    /* Vector shuffle with variable indices */
    v8si indices = {7,6,5,4,3,2,1,0};
    v8si v7 = __builtin_shuffle(v6, indices);
    
    volatile_result += v7[0] + v7[7];
    use(&v7);
}

/* ========== Main Test Driver ========== */

int main(int argc, char *argv[]) {
    /* Use argv[0] to create a pseudo-random seed */
    volatile_seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        volatile_seed = volatile_seed * 31 + argv[0][i];
    }
    
    printf("Testing optabs 10/11 operand cases with seed: %d\n", volatile_seed);
    
    /* Execute different test cases based on seed */
    int test_case = abs(volatile_seed) % 7;
    
    switch (test_case) {
        case 0:
            printf("Running vector shuffle test\n");
            #ifdef __AVX512F__
            test_vector_shuffle_10_operands();
            #endif
            break;
            
        case 1:
            printf("Running AVX-512 blend test\n");
            #ifdef __AVX512F__
            test_avx512_blend();
            #endif
            break;
            
        case 2:
            printf("Running x86 gather test\n");
            #ifdef __x86_64__
            test_x86_gather();
            #endif
            break;
            
        case 3:
            printf("Running atomic operations test\n");
            test_atomic_operations();
            break;
            
        case 4:
            printf("Running OpenMP SIMD test\n");
            #ifdef _OPENMP
            test_openmp_simd_complex();
            #endif
            break;
            
        case 5:
            printf("Running many-operand asm test\n");
            test_many_operand_asm();
            break;
            
        case 6:
            printf("Running complex vector ops test\n");
            test_complex_vector_ops();
            break;
            
        #ifdef __aarch64__
        case 7:
            printf("Running AArch64 multi-register test\n");
            test_aarch64_multi_reg();
            break;
        #endif
    }
    
    printf("Result: %d\n", volatile_result);
    
    /* Force use of all test functions to prevent dead code elimination */
    void (*funcs[])(void) = {
        #ifdef __AVX512F__
        test_vector_shuffle_10_operands,
        test_avx512_blend,
        #endif
        #ifdef __x86_64__
        test_x86_gather,
        #endif
        test_atomic_operations,
        #ifdef _OPENMP
        test_openmp_simd_complex,
        #endif
        test_many_operand_asm,
        test_complex_vector_ops,
        #ifdef __aarch64__
        test_aarch64_multi_reg,
        #endif
    };
    
    /* Reference all functions */
    for (size_t i = 0; i < sizeof(funcs)/sizeof(funcs[0]); i++) {
        if (funcs[i]) {
            /* Create a pointer that the compiler can't optimize away */
            volatile void *ptr = (void*)funcs[i];
            use(ptr);
        }
    }
    
    return volatile_result != 0 ? 0 : 1;
}

/* Dummy implementation of use() to prevent optimization */
void use(void *ptr) {
    /* Empty but marked as used */
    asm volatile ("" : : "r"(ptr) : "memory");
}
