/* test_optabs_10_11_operands.c
 * Test program to cover 10 and 11 operand cases in optabs.cc
 * Compile with: gcc -O2 -mavx512f -mavx512vl -fopenmp -ftree-vectorize -fno-tree-slp-vectorize test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization and inlining */
#define NO_OPT __attribute__((noipa, noinline, optimize(0)))

/* External function to keep values live */
extern void use(void*);

/* Volatile seed to prevent dead code elimination */
static volatile int seed = 0;

/* ========== Pattern 1: Vector shuffle with many elements ========== */
NO_OPT
void test_vector_shuffle() {
    /* Use AVX-512 512-bit vectors (16 elements) */
    typedef int v16si __attribute__((vector_size(64)));
    typedef long long v8di __attribute__((vector_size(64)));
    
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    v16si c;
    
    /* Shuffle with explicit indices - requires many operands during expansion */
    c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23);
    
    /* Use result to prevent elimination */
    volatile v16si* volatile ptr = &c;
    use((void*)ptr);
}

/* ========== Pattern 2: AVX-512 gather intrinsic ========== */
#ifdef __AVX512F__
NO_OPT
void test_gather_intrinsic() {
    /* AVX-512 gather can have many operands: base, scale, index, mask, etc. */
    double src[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double dst[8] = {0};
    int indices[8] = {0,2,4,6,1,3,5,7};
    
    /* Use inline assembly that looks like gather instruction */
    asm volatile (
        "vmovupd %1, %%zmm0\n\t"
        "vmovdqu32 %2, %%zmm1\n\t"
        "kxnorw %%k0, %%k0, %%k1\n\t"
        "vgatherdpd (%%zmm1, %3, 8), %%zmm0 {%%k1}\n\t"
        "vmovupd %%zmm0, %0\n\t"
        : "=m" (dst)
        : "m" (src), "m" (indices), "r" (seed)
        : "zmm0", "zmm1", "k1", "memory"
    );
    
    use((void*)dst);
}
#endif

/* ========== Pattern 3: Atomic compare-exchange with many params ========== */
NO_OPT
void test_atomic_ops() {
    volatile _Atomic int atomic_var = 0;
    int expected = 0;
    int desired = seed + 1;
    int success;
    
    /* __atomic_compare_exchange has many parameters that expand to multiple operands */
    success = __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                          0, /* weak */
                                          __ATOMIC_SEQ_CST, 
                                          __ATOMIC_RELAXED);
    
    /* Also test __atomic_exchange which can have complex expansion */
    int old = __atomic_exchange_n(&atomic_var, desired + 1, __ATOMIC_SEQ_CST);
    
    use((void*)&success);
    use((void*)&old);
}

/* ========== Pattern 4: OpenMP SIMD with many clauses ========== */
NO_OPT
void test_omp_simd() {
    #define N 1024
    static float a[N] __attribute__((aligned(64)));
    static float b[N] __attribute__((aligned(64)));
    static float c[N] __attribute__((aligned(64)));
    static float d[N] __attribute__((aligned(64)));
    
    /* Initialize with volatile to prevent constant propagation */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + seed);
        b[i] = (float)(i * 2 + seed);
        c[i] = (float)(i * 3 + seed);
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                     simdlen(16) safelen(32) \
                     reduction(+:seed)
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i] * c[i];
        seed += (int)d[i];
    }
    
    use((void*)d);
    use((void*)&seed);
}

/* ========== Pattern 5: Inline assembly with 10+ operands ========== */
NO_OPT
void test_multi_operand_asm() {
    int out1, out2;
    int a = seed + 1;
    int b = seed + 2;
    int c = seed + 3;
    int d = seed + 4;
    int e = seed + 5;
    int f = seed + 6;
    int g = seed + 7;
    int h = seed + 8;
    int i = seed + 9;
    int j = seed + 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "imul %[a], %[b]\n\t"
        "add %[c], %[b]\n\t"
        "sub %[d], %[b]\n\t"
        "mov %[b], %[out1]\n\t"
        "lea (%[e], %[f], 2), %[out2]\n\t"
        "add %[g], %[out2]\n\t"
        "sub %[h], %[out2]\n\t"
        "imul %[i], %[out2]\n\t"
        "add %[j], %[out2]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = seed + 11;
    int result;
    
    asm volatile (
        "mov %[a], %[res]\n\t"
        "add %[b], %[res]\n\t"
        "add %[c], %[res]\n\t"
        "add %[d], %[res]\n\t"
        "add %[e], %[res]\n\t"
        "add %[f], %[res]\n\t"
        "add %[g], %[res]\n\t"
        "add %[h], %[res]\n\t"
        "add %[i], %[res]\n\t"
        "add %[j], %[res]\n\t"
        "add %[k], %[res]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    use((void*)&out1);
    use((void*)&out2);
    use((void*)&result);
}

/* ========== Pattern 6: Complex vector operations ========== */
NO_OPT
void test_complex_vector_ops() {
    /* Use GCC vector extensions with complex operations */
    typedef float v8sf __attribute__((vector_size(32)));
    typedef float v16sf __attribute__((vector_size(64)));
    
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    v8sf v3 = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    
    /* Complex expression that may expand to many operands */
    v8sf result = (v1 * v2) + (v1 / v3) - (v2 * v3) + (v1 + v2) * (v1 - v2);
    
    /* Fused multiply-add style operations */
    v8sf fma_result;
    #ifdef __FMA__
    asm volatile (
        "vfmadd231ps %[v1], %[v2], %[v3]"
        : [v3] "+x" (fma_result)
        : [v1] "x" (v1), [v2] "x" (v2)
    );
    #endif
    
    use((void*)&result);
    #ifdef __FMA__
    use((void*)&fma_result);
    #endif
}

/* ========== Main test driver ========== */
int main(int argc, char *argv[]) {
    /* Use argv[0] to create a pseudo-random seed */
    for (char *p = argv[0]; *p; p++) {
        seed = (seed * 31) + *p;
    }
    
    /* Run different tests based on seed to cover multiple paths */
    switch (seed % 6) {
        case 0:
            test_vector_shuffle();
            break;
        case 1:
            #ifdef __AVX512F__
            test_gather_intrinsic();
            #else
            test_atomic_ops();
            #endif
            break;
        case 2:
            test_atomic_ops();
            break;
        case 3:
            test_omp_simd();
            break;
        case 4:
            test_multi_operand_asm();
            break;
        case 5:
            test_complex_vector_ops();
            break;
    }
    
    /* Print something to ensure execution */
    printf("Test completed with seed: %d\n", seed);
    
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void *ptr) {
    /* Empty but referenced to keep values live */
    (void)ptr;
}
