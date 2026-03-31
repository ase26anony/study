/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Technique 1: Vector shuffle with many elements */
#ifdef __GNUC__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));
#endif

__attribute__((noipa, noinline))
static void test_vector_shuffle(volatile int* result) {
#ifdef __GNUC__
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices = 18 operands total (2 vectors + 16 indices) */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Store results */
    memcpy((void*)result, &c, sizeof(c));
    memcpy((void*)(result + 16), &d, sizeof(d));
    
    use(&c);
    use(&d);
#endif
}

/* Technique 2: AVX-512 gather intrinsics (if available) */
#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noipa, noinline))
static void test_gather_intrinsic(volatile int* result) {
    /* __m512i _mm512_i32gather_epi32(__m512i vindex, void const* base_addr, int scale)
     * This expands to multiple operands including mask, scale, etc. */
    int base[64] __attribute__((aligned(64)));
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Another gather with mask */
    __mmask16 mask = 0xAAAA;
    __m512i gathered_masked = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(), mask, index, base, 4);
    
    _mm512_store_epi32((void*)result, gathered);
    _mm512_store_epi32((void*)(result + 16), gathered_masked);
    
    use(&gathered);
    use(&gathered_masked);
}
#endif

/* Technique 3: Atomic operations with many parameters */
__attribute__((noipa, noinline))
static void test_atomic_operations(volatile int* result) {
    volatile int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0; /* Use strong by default */
    
    /* __atomic_compare_exchange with 6 explicit args + implicit ones */
    int success = __atomic_compare_exchange_n(
        &atomic_var, &expected, desired,
        weak, /* weak */
        __ATOMIC_SEQ_CST, /* success_memorder */
        __ATOMIC_RELAXED  /* failure_memorder */
    );
    
    /* Another atomic with fetch_add */
    int old = __atomic_fetch_add(&atomic_var, 23, __ATOMIC_SEQ_CST);
    
    result[0] = success;
    result[1] = atomic_var;
    result[2] = old;
    result[3] = expected;
    
    use(&atomic_var);
}

/* Technique 4: OpenMP SIMD with many clauses */
#ifdef _OPENMP
__attribute__((noipa, noinline))
static void test_openmp_simd(volatile int* result) {
    #define N 128
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    int d[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* OpenMP SIMD with multiple clauses - can generate many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                simdlen(8) safelen(16) reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i] * c[i];
        result[0] += d[i];
    }
    
    /* Another SIMD loop with collapse */
    #pragma omp simd collapse(2) simdlen(4)
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            a[idx] = b[idx] - c[idx];
        }
    }
    
    memcpy((void*)result, d, sizeof(int) * 16);
    use(a); use(b); use(c); use(d);
}
#endif

/* Technique 5: Inline assembly with many operands */
__attribute__((noipa, noinline))
static void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
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
    asm volatile (
        "/* 11-operand asm %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 */\n\t"
        "imul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "sub %0, %0, %4\n\t"
        "and %0, %0, %5\n\t"
        "or %0, %0, %6\n\t"
        "xor %0, %0, %7\n\t"
        "shl %0, %0, %8\n\t"
        "shr %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use(&out1);
    use(&out2);
}

/* Technique 6: Complex built-in with many arguments */
#ifdef __GNUC__
__attribute__((noipa, noinline))
static void test_complex_builtin(volatile int* result) {
    /* __builtin_constant_p with many arguments in expression */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    /* Complex expression that might expand to many operands */
    int complex_expr = __builtin_constant_p(
        (a + b) * (c - d) / (e | f) & (g ^ h) + (i << 2) - (j >> 1) + ~k
    ) ? 1 : 0;
    
    /* Another complex builtin usage */
    long long llresult = __builtin_add_overflow_p(
        a, b * c - d + e * f - g / h + i - j * k,
        0LL
    );
    
    result[0] = complex_expr;
    result[1] = (int)llresult;
    
    use(&complex_expr);
    use(&llresult);
}
#endif

/* Technique 7: Vector reduction with many elements */
#ifdef __GNUC__
__attribute__((noipa, noinline))
static void test_vector_reduction(volatile int* result) {
    v16si v1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si v2 = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si v3 = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    
    /* Complex vector operation that might use many operands */
    v16si r1 = v1 + v2 * v3 - v1 / v3 + v2 % (v3 + v16si){1};
    v16si r2 = v1 & v2 | v3 ^ ~v1;
    
    /* Horizontal reduction */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += r1[i] + r2[i];
    }
    
    memcpy((void*)result, &r1, sizeof(r1));
    memcpy((void*)(result + 16), &r2, sizeof(r2));
    result[32] = sum;
    
    use(&r1);
    use(&r2);
}
#endif

/* Main function with volatile control flow */
int main(int argc, char *argv[]) {
    /* Create volatile seed from program name */
    volatile unsigned seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    
    /* Allocate result buffer */
    volatile int* results = (volatile int*)malloc(256 * sizeof(int));
    if (!results) return 1;
    
    /* Initialize results */
    for (int i = 0; i < 256; i++) {
        results[i] = i;
    }
    
    /* Execute different tests based on seed */
    int test_choice = seed % 7;
    
    switch (test_choice) {
        case 0:
            test_vector_shuffle(results);
            break;
        case 1:
#ifdef __AVX512F__
            test_gather_intrinsic(results);
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
#ifdef __GNUC__
            test_complex_builtin(results);
#endif
            break;
        case 6:
#ifdef __GNUC__
            test_vector_reduction(results);
#endif
            break;
        default:
            /* Fallback: simple arithmetic */
            results[0] = results[1] + results[2] * results[3];
            break;
    }
    
    /* Compute checksum to ensure code executed */
    volatile int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Prevent dead code elimination */
    use((void*)results);
    use((void*)&checksum);
    
    free((void*)results);
    return 0;
}

/* Dummy implementation of use() to satisfy linker */
void use(void* ptr) {
    /* Empty but prevents optimization */
    asm volatile("" : : "r"(ptr) : "memory");
}
