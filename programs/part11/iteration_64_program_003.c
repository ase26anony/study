/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's optabs expansion for operations
 * requiring 10 or 11 operands, covering the uncovered lines in optabs.cc
 * 
 * Compilation suggestions:
 * x86_64: gcc -O2 -mavx512f -mavx512vl -mavx512bw -ftree-vectorize -fno-tree-slp-vectorize test.c -o test
 * AArch64: gcc -O3 -march=armv8.2-a+simd -fopenmp -fno-omit-frame-pointer test.c -o test
 * Generic: gcc -O1 -fno-optimize-sibling-calls test.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== Pattern 1: Vector Shuffle with Many Elements ==================== */

__attribute__((noipa, noinline))
static void test_vector_shuffle(void) {
    /* Large vector type - 16 ints = 64 bytes */
    typedef int v16si __attribute__((vector_size(64)));
    
    volatile v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    volatile v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    v16si c;
    
    /* Shuffle with explicit indices - creates many operands during expansion */
    c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7,    /* First 8 from a */
        16, 17, 18, 19, 20, 21, 22, 23  /* First 8 from b (16-23) */
    );
    
    use(&c);
}

/* ==================== Pattern 2: x86 AVX-512 Gather Intrinsic ==================== */

#ifdef __AVX512F__
__attribute__((noipa, noinline))
static void test_avx512_gather(void) {
    /* AVX-512 gather intrinsic with many parameters */
    typedef double v8df __attribute__((vector_size(64)));
    typedef long long v8di __attribute__((vector_size(64)));
    typedef long long v8dm __attribute__((vector_size(8)));
    
    volatile double base[128];
    volatile v8di index = {0, 8, 16, 24, 32, 40, 48, 56};
    volatile v8dm mask = {-1, -1, -1, -1, -1, -1, -1, -1};
    v8df result;
    
    /* Initialize base array */
    for (int i = 0; i < 128; i++) {
        base[i] = i * 1.5;
    }
    
    /* __builtin_ia32_gathersiv8df takes approximately 10 operands:
     * 1. result
     * 2. base pointer
     * 3. index
     * 4. scale
     * 5. mask
     * 6. result (again for some variants)
     * 7-10: additional mask/control operands
     */
    asm volatile (
        "vmovapd %1, %%zmm0\n\t"
        "vmovdqa64 %2, %%zmm1\n\t"
        "vmovdqa64 %3, %%zmm2\n\t"
        "kxnorw %%k0, %%k0, %%k1\n\t"
        "vgatherqpd %4(,%%zmm1,8), %%zmm0 {%%k1}\n\t"
        "vmovapd %%zmm0, %0"
        : "=m"(result)
        : "m"(base), "m"(index), "m"(mask), "r"(base)
        : "zmm0", "zmm1", "zmm2", "k1", "memory"
    );
    
    use(&result);
}
#endif

/* ==================== Pattern 3: Atomic Compare Exchange with Many Parameters ==================== */

__attribute__((noipa, noinline))
static void test_atomic_compare_exchange(void) {
    volatile intptr_t ptr_val = 0x1000;
    volatile intptr_t* ptr = (intptr_t*)ptr_val;
    volatile intptr_t expected = 42;
    volatile intptr_t desired = 43;
    intptr_t expected_local = expected;
    int success;
    
    /* __atomic_compare_exchange_n with 6 explicit parameters + implicit ones
     * expands to many operands during RTL generation */
    success = __atomic_compare_exchange_n(
        (intptr_t*)ptr, 
        &expected_local,
        desired,
        0,  /* weak */
        __ATOMIC_SEQ_CST,
        __ATOMIC_RELAXED
    );
    
    use(&success);
    use(&expected_local);
}

/* ==================== Pattern 4: OpenMP SIMD with Multiple Clauses ==================== */

__attribute__((noipa, noinline))
static void test_openmp_simd(void) {
    #define N 1024
    volatile double a[N], b[N], c[N];
    volatile int i;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i * 1.0;
        b[i] = i * 2.0;
    }
    
    /* OpenMP SIMD with many clauses - each clause adds operands during expansion */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(32) \
                     reduction(+:c[0]) if(simd: control)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * b[i] + i;
    }
    
    use(a);
    use(b);
    use(c);
}

/* ==================== Pattern 5: Inline Assembly with 10+ Operands ==================== */

__attribute__((noipa, noinline))
static void test_multi_operand_asm(void) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* Inline assembly with exactly 10 operands */
    asm volatile (
        "/* Multi-operand test %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
        "add %1, %2, %0\n\t"
        "add %0, %3, %0\n\t"
        "add %0, %4, %0\n\t"
        "add %0, %5, %0\n\t"
        "add %0, %6, %0\n\t"
        "add %0, %7, %0\n\t"
        "add %0, %8, %0\n\t"
        "add %0, %9, %0"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* Another with 11 operands */
    asm volatile (
        "mov %1, %0\n\t"
        "add %0, %2, %0\n\t"
        "add %0, %3, %0\n\t"
        "add %0, %4, %0\n\t"
        "add %0, %5, %0\n\t"
        "add %0, %6, %0\n\t"
        "add %0, %7, %0\n\t"
        "add %0, %8, %0\n\t"
        "add %0, %9, %0\n\t"
        "add %0, %10, %0"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    use(&out1);
    use(&out2);
}

/* ==================== Pattern 6: Complex Vector Operation ==================== */

__attribute__((noipa, noinline))
static void test_complex_vector_op(void) {
    /* Using GCC complex vector extensions */
    typedef float __attribute__((vector_size(32))) v8sf;
    typedef float __attribute__((vector_size(32), __complex__)) v8cf;
    
    volatile v8cf cv1, cv2, cv3;
    volatile v8sf mask = {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    v8cf result;
    
    /* Initialize complex vectors */
    for (int i = 0; i < 8; i++) {
        ((float*)&cv1)[2*i] = i * 1.0f;
        ((float*)&cv1)[2*i+1] = i * 0.5f;
        ((float*)&cv2)[2*i] = i * 2.0f;
        ((float*)&cv2)[2*i+1] = i * 1.5f;
    }
    
    /* Complex multiply-add operation - expands to many operands */
    result = cv1 * cv2 + cv3;
    
    /* Blend based on mask - another multi-operand operation */
    v8sf real_result = __builtin_shuffle(
        (v8sf)result, 
        (v8sf)cv3, 
        (__attribute__((vector_size(32))) int){0,2,4,6,8,10,12,14}
    );
    
    use(&result);
    use(&real_result);
}

/* ==================== Main Execution ==================== */

int main(int argc, char *argv[]) {
    /* Use argv[0] to create a simple hash for control flow */
    unsigned int seed = 0;
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    control = seed;
    
    /* Execute different patterns based on seed to ensure all code paths
     * are considered by the compiler during expansion */
    switch (seed % 6) {
        case 0:
            test_vector_shuffle();
            break;
        case 1:
#ifdef __AVX512F__
            test_avx512_gather();
#else
            test_atomic_compare_exchange();
#endif
            break;
        case 2:
            test_atomic_compare_exchange();
            break;
        case 3:
            test_openmp_simd();
            break;
        case 4:
            test_multi_operand_asm();
            break;
        case 5:
            test_complex_vector_op();
            break;
    }
    
    /* Force all patterns to be compiled by referencing them */
    void (*funcs[])(void) = {
        test_vector_shuffle,
#ifdef __AVX512F__
        test_avx512_gather,
#endif
        test_atomic_compare_exchange,
        test_openmp_simd,
        test_multi_operand_asm,
        test_complex_vector_op
    };
    
    /* Create a checksum from function addresses to ensure they're not optimized away */
    unsigned long checksum = 0;
    for (size_t i = 0; i < sizeof(funcs)/sizeof(funcs[0]); i++) {
        checksum += (unsigned long)funcs[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    printf("Control: %d\n", control);
    
    return 0;
}

/* Dummy implementation of use() to satisfy linker */
void use(void *ptr) {
    /* Prevent optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}
