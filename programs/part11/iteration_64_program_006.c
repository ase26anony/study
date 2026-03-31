/* test_optabs_10_11.c - Cover GCC optabs.cc case 10/11 operand expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== PATTERN 1: Vector Shuffle with Many Elements ==================== */
__attribute__((noipa, noinline))
static void test_vector_shuffle(void) {
    /* Use GCC vector extensions with large types */
    typedef int v16si __attribute__((vector_size(64)));
    typedef long long v8di __attribute__((vector_size(64)));
    
    volatile v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    volatile v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    v16si c;
    
    /* Shuffle with 16 indices = 18 operands total during expansion */
    c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7, 
        16, 17, 18, 19, 20, 21, 22, 23);
    
    use(&c);
}

/* ==================== PATTERN 2: x86 Gather Intrinsics (if available) ==================== */
#ifdef __x86_64__
#include <x86intrin.h>
__attribute__((noipa, noinline, target("avx512f")))
static void test_gather_intrinsic(void) {
    /* AVX-512 gather can have many operands */
    volatile __m512d src = _mm512_set1_pd(1.0);
    volatile __m512i index = _mm512_set1_epi64(0);
    volatile __m512d base = _mm512_set1_pd(2.0);
    volatile __mmask8 mask = 0xFF;
    
    __m512d result = _mm512_mask_i64gather_pd(src, mask, index, 
                                              (const void*)&base, 8);
    use(&result);
}
#endif

/* ==================== PATTERN 3: Atomic Compare Exchange with Many Parameters ==================== */
__attribute__((noipa, noinline))
static void test_atomic_ops(void) {
    volatile intptr_t atomic_var = 0;
    intptr_t expected = 0;
    intptr_t desired = 42;
    int success;
    
    /* __atomic_compare_exchange has 6 explicit args + implicit ones */
    success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                        0, /* weak */
                                        __ATOMIC_SEQ_CST,
                                        __ATOMIC_ACQUIRE);
    
    /* Another atomic with many params */
    intptr_t val = __atomic_fetch_add(&atomic_var, 1, __ATOMIC_RELAXED);
    use(&success);
    use(&val);
}

/* ==================== PATTERN 4: OpenMP SIMD with Multiple Clauses ==================== */
__attribute__((noipa, noinline))
static void test_omp_simd(void) {
    #define N 1024
    static volatile double a[N], b[N], c[N];
    int i;
    
    /* OpenMP SIMD with many clauses - can expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) \
                simdlen(8) safelen(16) \
                reduction(+:a[0]) if(control)
    for (i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + i;
    }
    
    use(a);
}

/* ==================== PATTERN 5: Inline Assembly with Many Operands ==================== */
__attribute__((noipa, noinline))
static void test_multi_operand_asm(void) {
    volatile int64_t out1, out2;
    volatile int64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int64_t f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand template %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */\n"
        : "=r"(out1), "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "memory", "cc"
    );
    
    /* 11-operand asm statement */
    volatile int64_t k = 11;
    asm volatile (
        "/* 11-operand template */\n"
        : "+r"(out1), "+r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "memory"
    );
    
    use(&out1);
    use(&out2);
}

/* ==================== PATTERN 6: Complex Built-in with Many Args ==================== */
#ifdef __aarch64__
#include <arm_neon.h>
__attribute__((noipa, noinline))
static void test_aarch64_multi_reg(void) {
    /* AArch64 multi-register operations */
    volatile int32x4x2_t data2;
    volatile int32x4x4_t data4;
    int32x4_t lanes[4];
    
    /* Load multiple registers - expands to many operands */
    data2 = vld2q_s32((const int32_t*)&control);
    data4 = vld4q_s32((const int32_t*)&control);
    
    /* Table lookup with many operands */
    int32x4_t result = vqtbl4q_s8(data4, vreinterpretq_s8_s32(data2.val[0]));
    
    use(&result);
}
#endif

/* ==================== MAIN DRIVER ==================== */
int main(int argc, char *argv[]) {
    /* Use argv[0] to create a "random" seed */
    unsigned seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    control = seed;
    
    printf("Testing multi-operand expansion patterns (seed=%u)\n", seed);
    
    /* Execute different patterns based on seed */
    switch (seed % 6) {
        case 0:
            test_vector_shuffle();
            printf("Vector shuffle test\n");
            break;
        case 1:
#ifdef __x86_64__
            test_gather_intrinsic();
            printf("Gather intrinsic test\n");
#endif
            break;
        case 2:
            test_atomic_ops();
            printf("Atomic operations test\n");
            break;
        case 3:
            test_omp_simd();
            printf("OpenMP SIMD test\n");
            break;
        case 4:
            test_multi_operand_asm();
            printf("Multi-operand asm test\n");
            break;
        case 5:
#ifdef __aarch64__
            test_aarch64_multi_reg();
            printf("AArch64 multi-reg test\n");
#endif
            break;
    }
    
    /* Force all patterns to be considered by compiler */
    if (control == 0xdeadbeef) {
        test_vector_shuffle();
#ifdef __x86_64__
        test_gather_intrinsic();
#endif
        test_atomic_ops();
        test_omp_simd();
        test_multi_operand_asm();
#ifdef __aarch64__
        test_aarch64_multi_reg();
#endif
    }
    
    return 0;
}

/* Dummy implementation of use() to prevent optimization */
void use(void *p) {
    asm volatile ("" : : "r"(p) : "memory");
}
