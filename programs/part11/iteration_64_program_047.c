/* test_optabs_multioperand.c - Cover 10/11 operand cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== PATTERN 1: Vector Shuffle with Many Elements ==================== */

__attribute__((noipa, noinline))
static void test_vector_shuffle(void) {
    /* Large vector shuffle that may require many operands */
    typedef int v16si __attribute__((vector_size(64)));
    typedef int v32si __attribute__((vector_size(128)));
    
    volatile v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    volatile v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with explicit control mask - many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,    /* First 8 from a */
        16,17,18,19,20,21,22,23  /* First 8 from b */
    );
    
    /* Use result to prevent elimination */
    use(&c);
    
    /* Even larger shuffle attempt */
    v32si d = __builtin_shufflevector(
        (v32si){0}, (v32si){0},
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    use(&d);
}

/* ==================== PATTERN 2: x86 AVX-512 Gather Intrinsics ==================== */

#ifdef __x86_64__
__attribute__((target("avx512f")))
__attribute__((noipa, noinline))
static void test_avx512_gather(void) {
    /* AVX-512 gather instructions have many operands */
    typedef double v8df __attribute__((vector_size(64)));
    typedef int v8si __attribute__((vector_size(32)));
    typedef long long v8di __attribute__((vector_size(64)));
    
    volatile double base[1024];
    volatile v8si index = {0,8,16,24,32,40,48,56};
    volatile v8df src = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    volatile v8di mask = {-1, -1, -1, -1, -1, -1, -1, -1};
    
    /* __builtin_ia32_gathersiv8df has many parameters:
       src, base, index, scale, mask, hint */
    v8df result;
    
    /* Direct intrinsic usage - may expand to many operands */
    asm volatile (
        "vmovapd %[src], %%zmm0\n\t"
        "vmovaps %[index], %%ymm1\n\t"
        "vmovapd %[mask], %%zmm2\n\t"
        "vgatherqpd %%zmm2, (%%rax,%%ymm1,8), %%zmm0\n\t"
        "vmovapd %%zmm0, %[result]\n\t"
        : [result] "=m" (result)
        : [src] "m" (src), [index] "m" (index), [mask] "m" (mask),
          "a" (base)
        : "zmm0", "zmm1", "zmm2", "memory"
    );
    
    use(&result);
}
#endif

/* ==================== PATTERN 3: Atomic Operations with Many Parameters ==================== */

__attribute__((noipa, noinline))
static void test_atomic_operations(void) {
    volatile _Atomic int atomic_var = 42;
    int expected = 42;
    int desired = 43;
    int weak_result;
    
    /* __atomic_compare_exchange with many parameters */
    weak_result = __atomic_compare_exchange_n(
        &atomic_var, &expected, desired,
        0,  /* weak */
        __ATOMIC_SEQ_CST, __ATOMIC_RELAXED
    );
    
    use(&weak_result);
    
    /* Another atomic with many operands */
    long double ld = 3.14159;
    long double old = __atomic_exchange_n(&ld, 2.71828, __ATOMIC_ACQ_REL);
    use(&old);
}

/* ==================== PATTERN 4: OpenMP SIMD with Complex Clauses ==================== */

__attribute__((noipa, noinline))
static void test_openmp_simd(void) {
#define N 1024
    volatile double a[N], b[N], c[N];
    int i;
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        a[i] = i * 1.0;
        b[i] = i * 2.0;
    }
    
    /* OpenMP SIMD with many clauses - may expand to multi-operand operations */
#pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16) \
                private(i) lastprivate(i)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * b[i] + (double)i;
    }
    
    use(c);
#undef N
}

/* ==================== PATTERN 5: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
static void test_many_operand_asm(void) {
    /* 10-operand asm statement */
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    asm volatile (
        "/* 10-operand test */\n\t"
        "addl %[a], %[b]\n\t"
        "addl %[c], %[d]\n\t"
        "addl %[e], %[f]\n\t"
        "addl %[g], %[h]\n\t"
        "addl %[i], %[j]\n\t"
        "movl %[b], %[out1]\n\t"
        "movl %[j], %[out2]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [a] "r" (a), [b] "0" (b), [c] "r" (c), [d] "1" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    use(&out1);
    use(&out2);
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand test */\n\t"
        "imull %[a], %[b]\n\t"
        "addl %[c], %[d]\n\t"
        "subl %[e], %[f]\n\t"
        "andl %[g], %[h]\n\t"
        "orl  %[i], %[j]\n\t"
        "xorl %[k], %[out1]"
        : [out1] "=r" (out1)
        : [a] "r" (a), [b] "0" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    use(&out1);
}

/* ==================== PATTERN 6: AArch64 NEON Multi-register Operations ==================== */

#ifdef __aarch64__
__attribute__((noipa, noinline))
static void test_aarch64_neon(void) {
    /* AArch64 has multi-register load/store instructions */
    typedef int32x4_t v4si;
    typedef int32x4x4_t v4x4si;
    
    volatile int32_t data[16];
    v4x4si vecs;
    
    /* ld4 instruction loads 4 registers - may require many operands */
    asm volatile (
        "ld4 {v0.4s, v1.4s, v2.4s, v3.4s}, [%[data]]\n\t"
        "st4 {v0.4s, v1.4s, v2.4s, v3.4s}, [%[data], #64]"
        : 
        : [data] "r" (data)
        : "v0", "v1", "v2", "v3", "memory"
    );
    
    use(data);
}
#endif

/* ==================== PATTERN 7: Complex Built-in with Many Arguments ==================== */

__attribute__((noipa, noinline))
static void test_complex_builtin(void) {
    /* __builtin_constant_p with many arguments in a complex expression */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Complex expression that might expand to many operands */
    int result = __builtin_constant_p(a) ? a :
                 __builtin_constant_p(b) ? b :
                 __builtin_constant_p(c) ? c :
                 __builtin_constant_p(d) ? d :
                 __builtin_constant_p(e) ? e :
                 __builtin_constant_p(f) ? f :
                 __builtin_constant_p(g) ? g :
                 __builtin_constant_p(h) ? h :
                 __builtin_constant_p(i) ? i :
                 __builtin_constant_p(j) ? j : 0;
    
    use(&result);
    
    /* __sync builtin with many arguments */
    long long llresult = __sync_val_compare_and_swap(
        &a, 1, 100
    );
    use(&llresult);
}

/* ==================== MAIN FUNCTION ==================== */

int main(int argc, char *argv[]) {
    /* Use argv[0] to create a pseudo-random seed */
    unsigned seed = 0;
    for (char *p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    
    /* Execute different patterns based on seed */
    switch (seed % 7) {
        case 0:
            test_vector_shuffle();
            break;
        case 1:
#ifdef __x86_64__
            test_avx512_gather();
#endif
            break;
        case 2:
            test_atomic_operations();
            break;
        case 3:
            test_openmp_simd();
            break;
        case 4:
            test_many_operand_asm();
            break;
        case 5:
#ifdef __aarch64__
            test_aarch64_neon();
#endif
            break;
        case 6:
            test_complex_builtin();
            break;
    }
    
    /* Compute checksum to ensure execution */
    volatile int checksum = seed;
    for (int i = 0; i < 100; i++) {
        checksum ^= i;
    }
    
    printf("Checksum: %u\n", checksum);
    return 0;
}
