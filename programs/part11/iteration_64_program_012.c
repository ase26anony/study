/* test_optabs_10_11.c - Cover 10/11 operand expansion cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* ==================== Pattern 1: Vector Shuffle with Many Elements ==================== */

/* Large vector types for shuffle operations */
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with explicit control mask - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Store results */
    memcpy((void*)result, &c, sizeof(c));
    memcpy((void*)(result + 16), &d, sizeof(d));
    
    use((void*)result);
}

/* ==================== Pattern 2: AVX-512 Gather Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather operations can have many operands */
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xAAAA;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    /* __mm512_i32gather_epi32 has many parameters during expansion */
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        index,                   // index
        (void const*)base,       // base
        4                        // scale
    );
    
    _mm512_store_epi32((void*)result, gathered);
    use((void*)result);
}
#endif

/* ==================== Pattern 3: Atomic Operations ==================== */

__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    volatile int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(
        &atomic_var,        // ptr
        &expected,          // expected
        &desired,           // desired
        weak,               // weak
        __ATOMIC_SEQ_CST,   // success_memorder
        __ATOMIC_ACQUIRE    // failure_memorder
    );
    
    /* Another atomic with multiple args */
    int old = __atomic_fetch_add(&atomic_var, 23, __ATOMIC_RELAXED);
    
    result[0] = success;
    result[1] = old;
    result[2] = atomic_var;
    
    use((void*)result);
}

/* ==================== Pattern 4: OpenMP SIMD with Many Clauses ==================== */

__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    #define N 128
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32) \
                    reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + i;
        result[0] += a[i];
    }
    
    /* Store some results */
    for (int i = 0; i < 8; i++) {
        result[i+1] = a[i*16];
    }
    
    use((void*)result);
}

/* ==================== Pattern 5: Multi-Operand Inline Assembly ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand assembly (9 inputs + 1 output) */
    asm volatile (
        "/* Multi-operand test %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
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
    
    /* 11-operand assembly (10 inputs + 1 output) */
    asm volatile (
        "/* 11-operand test %0 = sum of 10 values */\n\t"
        "mov %0, #0\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2\n\t"
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
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use((void*)result);
}

/* ==================== Pattern 6: Complex Vector Operations ==================== */

__attribute__((noipa, noinline))
void test_complex_vector_ops(volatile int* result) {
    /* Using GCC vector extensions with complex operations */
    typedef float v8sf __attribute__((vector_size(32)));
    typedef int v8si __attribute__((vector_size(32)));
    
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    v8sf v3 = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    
    /* Complex expression that may expand to many operands */
    v8sf r1 = v1 * v2 + v3;
    v8sf r2 = __builtin_shufflevector(v1, v2, 0,9,2,11,4,13,6,15);
    v8si mask = r1 > r2;
    v8sf r3 = __builtin_shufflevector(r1, r2, 7,6,5,4,3,2,1,0);
    
    /* Conditional select - another multi-operand operation */
    v8sf final = __builtin_shufflevector(
        (v8sf)__builtin_shufflevector((v8si)r1, (v8si)r3, 0,1,2,3,4,5,6,7),
        (v8sf)mask,
        0,9,2,11,4,13,6,15
    );
    
    memcpy((void*)result, &final, sizeof(final));
    use((void*)result);
}

/* ==================== Main Function ==================== */

int main(int argc, char *argv[]) {
    /* Create volatile result arrays */
    volatile int results[256] = {0};
    
    /* Simple hash from argv[0] for branch selection */
    unsigned seed = 0;
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Execute different patterns based on seed */
    switch (seed % 6) {
        case 0:
            test_vector_shuffle(results);
            break;
        case 1:
            #ifdef __AVX512F__
            test_avx512_gather(results);
            #else
            test_vector_shuffle(results);
            #endif
            break;
        case 2:
            test_atomic_ops(results);
            break;
        case 3:
            test_openmp_simd(results);
            break;
        case 4:
            test_multi_operand_asm(results);
            break;
        case 5:
            test_complex_vector_ops(results);
            break;
    }
    
    /* Compute checksum to ensure execution */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
