/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* Test 1: Vector shuffle with many elements */
__attribute__((noipa, noinline))
static void test_vector_shuffle(volatile int* result) {
    typedef int v16si __attribute__((vector_size(64)));
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with explicit indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Use AVX-512 style mask blend if available */
    #ifdef __AVX512F__
    __mmask16 mask = 0xAAAA;
    v16si d = _mm512_mask_blend_epi32(mask, a, b);
    c = __builtin_shufflevector(c, d, 0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23);
    #endif
    
    /* Store result */
    memcpy((void*)result, &c, sizeof(c));
    use((void*)result);
}

/* Test 2: Gather intrinsics (x86-specific, many operands) */
__attribute__((noipa, noinline))
static void test_gather_intrinsic(volatile int* result) {
    #ifdef __AVX512F__
    float src[64];
    int indices[16];
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    __mmask16 mask = 0xFFFF;
    
    /* __builtin_ia32_gathersiv16sf has many parameters */
    __m512 gathered = _mm512_mask_i32gather_ps(
        _mm512_setzero_ps(),    // src
        mask,                   // mask
        vindex,                 // vindex
        src,                    // base
        4                       // scale
    );
    
    memcpy((void*)result, &gathered, sizeof(gathered));
    #endif
    use((void*)result);
}

/* Test 3: Atomic compare-exchange with many parameters */
__attribute__((noipa, noinline))
static void test_atomic_ops(volatile int* result) {
    intptr_t atomic_var = 0;
    intptr_t expected = 0;
    intptr_t desired = 42;
    
    /* __atomic_compare_exchange has 6 parameters, expands to many operands */
    int success = __atomic_compare_exchange(
        &atomic_var,            // ptr
        &expected,              // expected
        &desired,               // desired
        0,                      // weak
        __ATOMIC_SEQ_CST,       // success_memorder
        __ATOMIC_RELAXED        // failure_memorder
    );
    
    /* Additional atomic ops */
    __atomic_add_fetch(&atomic_var, 1, __ATOMIC_SEQ_CST);
    __atomic_and_fetch(&atomic_var, 0xFF, __ATOMIC_SEQ_CST);
    
    *result = (int)(atomic_var | (success << 16));
    use((void*)result);
}

/* Test 4: OpenMP SIMD with multiple clauses */
__attribute__((noipa, noinline))
static void test_openmp_simd(volatile int* result) {
    #define N 128
    float a[N], b[N], c[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16) \
                     reduction(+:c[0:N]) if(control > 0)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i] + (float)i;
    }
    
    /* Additional pragma for collapse */
    #pragma omp simd collapse(2) aligned(a:32)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            a[i*8 + j] += c[i*8 + j];
        }
    }
    
    result[0] = (int)c[0];
    use((void*)result);
}

/* Test 5: Inline assembly with many operands */
__attribute__((noipa, noinline))
static void test_multi_operand_asm(volatile int* result) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand test %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */\n\t"
        "add %1, %2\n\t"
        "adc %3, %4\n\t"
        "mov %0, %1\n\t"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc", "memory"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand test */\n\t"
        "imul %1, %2\n\t"
        "add %3, %0\n\t"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    result[0] = out1 + out2;
    use((void*)result);
}

/* Test 6: Complex builtin with many arguments */
__attribute__((noipa, noinline))
static void test_complex_builtin(volatile int* result) {
    #ifdef __AVX512F__
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __mmask16 m1 = 0xAAAA;
    __mmask16 m2 = 0x5555;
    
    /* Complex sequence that may expand to many operands */
    __m512i r1 = _mm512_mask_add_epi32(a, m1, b, c);
    __m512i r2 = _mm512_mask_mullo_epi32(r1, m2, a, b);
    __m512i r3 = _mm512_mask_slli_epi32(r2, m1, r1, 3);
    
    memcpy((void*)result, &r3, sizeof(r3));
    #endif
    use((void*)result);
}

/* Main function with volatile control flow */
int main(int argc, char *argv[]) {
    /* Create volatile seed from argv[0] */
    unsigned seed = 0;
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    control = seed;
    
    /* Allocate volatile result arrays */
    volatile int results[6][64] = {0};
    volatile int checksum = 0;
    
    /* Execute tests based on seed */
    int test_to_run = seed % 6;
    
    switch (test_to_run) {
        case 0:
            test_vector_shuffle(results[0]);
            break;
        case 1:
            test_gather_intrinsic(results[1]);
            break;
        case 2:
            test_atomic_ops(results[2]);
            break;
        case 3:
            test_openmp_simd(results[3]);
            break;
        case 4:
            test_multi_operand_asm(results[4]);
            break;
        case 5:
            test_complex_builtin(results[5]);
            break;
    }
    
    /* Force execution of all tests in some compilers */
    if (control & 1) {
        test_vector_shuffle(results[0]);
    }
    if (control & 2) {
        test_atomic_ops(results[2]);
    }
    if (control & 4) {
        test_multi_operand_asm(results[4]);
    }
    
    /* Compute checksum to ensure code runs */
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 64; j++) {
            checksum ^= results[i][j];
        }
    }
    
    printf("Checksum: %d (seed: %u)\n", checksum, seed);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy implementation of external function */
void __attribute__((weak)) use(void *ptr) {
    /* Prevent optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}
