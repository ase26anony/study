/* test_optabs_10_11_operands.c
 * Test program to cover 10 and 11 operand cases in optabs.cc
 * Compile with: gcc -O2 -mavx512f -mavx512vl -fopenmp -ftree-vectorize test.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noipa, noinline, optimize(0)))

/* External function to keep values live */
extern void use(void*);

/* Volatile seed to prevent constant propagation */
static volatile int seed = 0;

/* Large vector types for shuffle operations */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

/* Test 1: Vector shuffle with many elements */
NOOPT void test_vector_shuffle(v16si* out, v16si a, v16si b) {
    /* Shuffle that requires many operands during expansion */
    *out = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7, 
        16, 17, 18, 19, 20, 21, 22, 23);
}

/* Test 2: AVX-512 gather intrinsic (many parameters) */
#ifdef __AVX512F__
NOOPT void test_gather_intrinsic(double* out, const double* base, 
                                 const int* index, __mmask8 mask) {
    /* __builtin_ia32_gathersiv8df expands to many operands */
    v8df result = __builtin_ia32_gathersiv8df(
        (v8df){0},            /* src */
        (const void*)base,    /* base */
        *(__m256i*)index,     /* vindex */
        mask,                 /* mask */
        1                     /* scale */
    );
    *(v8df*)out = result;
}
#endif

/* Test 3: Atomic compare exchange with many parameters */
NOOPT int test_atomic_compare_exchange(int* ptr, int expected, int desired) {
    int old = expected;
    /* This expands to complex RTL with many operands */
    __atomic_compare_exchange(ptr, &old, &desired, 0,
                             __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    return old;
}

/* Test 4: OpenMP SIMD with many clauses */
NOOPT void test_omp_simd(float* a, float* b, float* c, int n) {
    int i;
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32)
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + (float)i;
    }
}

/* Test 5: Inline assembly with 10 operands */
NOOPT int test_asm_10_operands(int a, int b, int c, int d, int e,
                               int f, int g, int h, int i, int j) {
    int result;
    /* 10 operand asm statement */
    asm volatile (
        "imul %[a], %[b]\n\t"
        "add %[c], %[b]\n\t"
        "imul %[d], %[b]\n\t"
        "add %[e], %[b]\n\t"
        "imul %[f], %[b]\n\t"
        "add %[g], %[b]\n\t"
        "imul %[h], %[b]\n\t"
        "add %[i], %[b]\n\t"
        "add %[j], %[b]\n\t"
        "mov %[b], %[res]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    return result;
}

/* Test 6: Inline assembly with 11 operands */
NOOPT int test_asm_11_operands(int a, int b, int c, int d, int e,
                               int f, int g, int h, int i, int j, int k) {
    int result;
    /* 11 operand asm statement */
    asm volatile (
        "imul %[a], %[b]\n\t"
        "add %[c], %[b]\n\t"
        "imul %[d], %[b]\n\t"
        "add %[e], %[b]\n\t"
        "imul %[f], %[b]\n\t"
        "add %[g], %[b]\n\t"
        "imul %[h], %[b]\n\t"
        "add %[i], %[b]\n\t"
        "imul %[j], %[b]\n\t"
        "add %[k], %[b]\n\t"
        "mov %[b], %[res]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    return result;
}

/* Test 7: Complex vector operation with blending */
NOOPT void test_vector_blend(v16sf* out, v16sf a, v16sf b, v16sf c) {
    /* Complex operation that may expand to many operands */
    v16sf t1 = a * b + c;
    v16sf t2 = b * c - a;
    
    /* Blend based on comparison - may use many operands */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    *out = _mm512_mask_blend_ps(mask, t1, t2);
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use argv[0] to create a non-constant seed */
    for (int i = 0; argv[0][i]; i++) {
        seed ^= argv[0][i] * (i + 1);
    }
    
    int test_case = seed % 7;
    int result = 0;
    
    /* Initialize test data */
    v16si vec_a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si vec_b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    v16si vec_out;
    
    float arr_a[64], arr_b[64], arr_c[64];
    for (int i = 0; i < 64; i++) {
        arr_a[i] = (float)i;
        arr_b[i] = (float)(i * 2);
        arr_c[i] = (float)(i * 3);
    }
    
    int atomic_var = 42;
    double gather_out[8];
    int gather_idx[8] = {0,2,4,6,8,10,12,14};
    
    switch (test_case) {
        case 0:
            test_vector_shuffle(&vec_out, vec_a, vec_b);
            result = vec_out[0] + vec_out[15];
            break;
            
        case 1:
#ifdef __AVX512F__
            test_gather_intrinsic(gather_out, (double*)arr_a, gather_idx, 0xFF);
            result = (int)gather_out[0] + (int)gather_out[7];
#else
            result = 999;
#endif
            break;
            
        case 2:
            result = test_atomic_compare_exchange(&atomic_var, 42, 100);
            break;
            
        case 3:
            test_omp_simd(arr_a, arr_b, arr_c, 64);
            result = (int)arr_a[0] + (int)arr_a[63];
            break;
            
        case 4:
            result = test_asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
            
        case 5:
            result = test_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
            break;
            
        case 6:
#ifdef __AVX512F__
            v16sf vf_a = {0};
            v16sf vf_b = {0};
            v16sf vf_c = {0};
            v16sf vf_out;
            for (int i = 0; i < 16; i++) {
                vf_a[i] = (float)i;
                vf_b[i] = (float)(i * 2);
                vf_c[i] = (float)(i * 3);
            }
            test_vector_blend(&vf_out, vf_a, vf_b, vf_c);
            result = (int)vf_out[0] + (int)vf_out[15];
#else
            result = 888;
#endif
            break;
    }
    
    /* Use results to prevent dead code elimination */
    use(&vec_out);
    use(&result);
    use(arr_a);
    use(arr_b);
    use(arr_c);
    use(&atomic_var);
    use(gather_out);
    
    printf("Test result: %d (seed: %d, case: %d)\n", result, seed, test_case);
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void* ptr) {
    /* Empty - just to keep values live */
    (void)ptr;
}
