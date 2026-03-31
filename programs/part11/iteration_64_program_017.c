/* test_optabs_coverage.c - Cover 10/11 operand cases in GCC optabs */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noipa, noinline, optimize(0)))
extern void use(void *); /* External function to keep values alive */

/* ==================== Pattern 1: Vector Shuffle with Many Elements ==================== */
NOOPT void test_vector_shuffle(volatile int *result) {
    /* Use large vector types to require many shuffle operands */
    typedef int v16si __attribute__((vector_size(64)));
    typedef long long v8di __attribute__((vector_size(64)));
    
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    v16si c;
    
    /* Shuffle with explicit control mask - many operands during expansion */
    c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,    /* First 8 from a */
        16,17,18,19,20,21,22,23  /* First 8 from b */
    );
    
    /* Complex shuffle pattern requiring many operands */
    v16si d = __builtin_shufflevector(c, a,
        0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23
    );
    
    /* Store result to volatile memory */
    for (int i = 0; i < 8; i++) {
        result[i] = d[i] + d[i+8];
    }
    
    use(&d);
}

/* ==================== Pattern 2: x86 AVX-512 Gather Intrinsics ==================== */
#ifdef __AVX512F__
#include <immintrin.h>
NOOPT void test_avx512_gather(volatile int *result) {
    /* AVX-512 gather instructions have many operands */
    __m512i index = _mm512_set_epi32(0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60);
    __mmask16 mask = 0xAAAA;
    __m512i src = _mm512_set1_epi32(42);
    int scale = 4;
    void *base_ptr = (void*)result;
    
    /* __builtin_ia32_gathersiv16si has many parameters */
    __m512i gathered = _mm512_mask_i32gather_epi32(src, mask, index, base_ptr, scale);
    
    /* Store with scatter - another multi-operand operation */
    __m512i data = _mm512_set1_epi32(99);
    _mm512_mask_i32scatter_epi32(base_ptr, mask, index, data, scale);
    
    /* Use the result */
    result[0] = _mm512_extract_epi32(gathered, 0);
    use(&gathered);
}
#endif

/* ==================== Pattern 3: Atomic Operations with Many Parameters ==================== */
NOOPT void test_atomic_operations(volatile int *result) {
    volatile int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0; /* Strong compare-exchange */
    
    /* __atomic_compare_exchange has many parameters that expand to multiple operands */
    int success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                           weak, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Complex atomic operation with multiple memory orders */
    int old = __atomic_fetch_add(&atomic_var, 23, __ATOMIC_ACQ_REL);
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    result[0] = success + old + atomic_var;
    use(&atomic_var);
}

/* ==================== Pattern 4: OpenMP SIMD with Complex Clauses ==================== */
NOOPT void test_openmp_simd(volatile int *result) {
    #define N 1024
    static int a[N] __attribute__((aligned(64)));
    static int b[N] __attribute__((aligned(64)));
    static int c[N] __attribute__((aligned(64)));
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with many clauses - expands to multi-operand operations */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32) \
                    reduction(+:result[0]) private(c)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i] + i;
        result[0] += c[i] & 0xFF;
    }
    
    /* Nested SIMD pragmas */
    #pragma omp simd collapse(2) simdlen(8)
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            a[i*32 + j] = b[i*32 + j] + c[i*32 + j];
        }
    }
    
    use(a); use(b); use(c);
}

/* ==================== Pattern 5: Multi-Operand Inline Assembly ==================== */
NOOPT void test_multi_operand_asm(volatile int *result) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm for coverage */\n\t"
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
        "/* 11-operand asm */\n\t"
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
    
    result[0] = out1 + out2;
    use(&out1); use(&out2);
}

/* ==================== Pattern 6: AArch64 NEON Multi-register Operations ==================== */
#ifdef __aarch64__
#include <arm_neon.h>
NOOPT void test_aarch64_neon(volatile int *result) {
    /* Multi-register load/store operations */
    int32x4x4_t quad_vec;  /* 4 registers of 4 ints each */
    int32_t data[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    
    /* Load 4 registers - expands to multi-operand operation */
    quad_vec = vld1q_s32_x4(data);
    
    /* Complex SIMD operations */
    int32x4_t a = quad_vec.val[0];
    int32x4_t b = quad_vec.val[1];
    int32x4_t c = quad_vec.val[2];
    int32x4_t d = quad_vec.val[3];
    
    /* FMA (fused multiply-add) chain */
    int32x4_t r1 = vmlaq_s32(a, b, c);
    int32x4_t r2 = vmlaq_s32(r1, d, a);
    int32x4_t r3 = vmlaq_s32(r2, b, d);
    int32x4_t r4 = vmlaq_s32(r3, c, a);
    
    /* Store results */
    vst1q_s32((int32_t*)result, r4);
    
    use(&quad_vec);
}
#endif

/* ==================== Main Execution Flow ==================== */
int main(int argc, char *argv[]) {
    volatile int results[64] = {0};
    volatile int seed = 0;
    
    /* Create seed from program name */
    for (char *p = argv[0]; *p; p++) {
        seed = (seed * 31 + *p) & 0xFF;
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
            test_vector_shuffle(results);  /* Fallback */
            #endif
            break;
        case 2:
            test_atomic_operations(results);
            break;
        case 3:
            test_openmp_simd(results);
            break;
        case 4:
            test_multi_operand_asm(results);
            break;
        case 5:
            #ifdef __aarch64__
            test_aarch64_neon(results);
            #else
            test_multi_operand_asm(results);  /* Fallback */
            #endif
            break;
    }
    
    /* Compute checksum to ensure execution */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFFFF;
    }
    
    printf("Checksum: %d (seed: %d)\n", checksum, seed);
    return checksum != 0 ? 0 : 1;
}

/* Dummy use function to prevent optimization */
void use(void *ptr) {
    asm volatile ("" : : "r"(ptr) : "memory");
}
