/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== Pattern 1: Vector Shuffle with Many Elements ==================== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with explicit control mask - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,    /* First 8 from a */
        16,17,18,19,20,21,22,23  /* First 8 from b */
    );
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,
        31,30,29,28,27,26,25,24
    );
    
    /* Complex operation combining results */
    v16si e = c + d;
    
    /* Store to volatile memory */
    for (int i = 0; i < 16; i++) {
        result[i] = e[i];
    }
    
    use(&e);
}
#endif

/* ==================== Pattern 2: AVX-512 Gather Intrinsics ==================== */
#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_gather_intrinsic(volatile int* result) {
    int base[64] __attribute__((aligned(64)));
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    __m512i index = _mm512_set_epi32(0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60);
    __mmask16 mask = 0xFFFF;
    
    /* __builtin_ia32_gathersiv16si can expand to many operands */
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Store result */
    _mm512_store_epi32((void*)result, gathered);
    
    use(&gathered);
}
#endif

/* ==================== Pattern 3: Atomic Compare Exchange with Many Parameters ==================== */
__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0, /* weak */
                                            __ATOMIC_SEQ_CST, 
                                            __ATOMIC_ACQUIRE);
    
    /* Another atomic operation */
    int old = __atomic_fetch_add(ptr, 100, __ATOMIC_RELAXED);
    
    result[1] = success;
    result[2] = old;
    
    use(ptr);
}

/* ==================== Pattern 4: OpenMP SIMD with Multiple Clauses ==================== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    int N = 64;
    int a[64] __attribute__((aligned(64)));
    int b[64] __attribute__((aligned(64)));
    int c[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 3;
    }
    
    /* OpenMP SIMD with many clauses - may expand to multi-operand operations */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(32) \
                    reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
        result[0] += c[i];
    }
    
    /* Store some results */
    for (int i = 0; i < 8; i++) {
        result[i+8] = c[i*8];
    }
    
    use(c);
}
#endif

/* ==================== Pattern 5: Inline Assembly with Many Operands ==================== */
__attribute__((noipa, noinline))
void test_many_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand test %0 = (%1+%2)*%3 + (%4+%5)*%6 + (%7+%8)*%9 */\n\t"
        "add %1, %2, %10\n\t"
        "mul %10, %3, %10\n\t"
        "add %4, %5, %11\n\t"
        "mul %11, %6, %11\n\t"
        "add %7, %8, %12\n\t"
        "mul %12, %9, %12\n\t"
        "add %10, %11, %10\n\t"
        "add %10, %12, %0"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i),
          "r"(out2)  /* dummy */
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand test */\n\t"
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
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
          "r"(k)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use(&out1);
}

/* ==================== Pattern 6: Complex Vector Operations ==================== */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_neon_operations(volatile int* result) {
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    
    /* Chain of operations that might expand to multi-operand patterns */
    int32x4_t e = vmlaq_s32(a, b, c);
    int32x4_t f = vmlsq_s32(d, e, a);
    int32x4_t g = vqdmulhq_s32(f, b);
    int32x4_t h = vrhaddq_s32(g, c);
    
    /* Store results */
    vst1q_s32((int32_t*)result, h);
    
    use(&h);
}
#endif

/* ==================== Main Function ==================== */
int main(int argc, char* argv[]) {
    /* Create volatile result arrays */
    volatile int results[128];
    for (int i = 0; i < 128; i++) results[i] = 0;
    
    /* Generate seed from program name */
    unsigned seed = 0;
    if (argc > 0) {
        for (char* p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Execute different patterns based on seed */
    int pattern = seed % 6;
    
    switch (pattern) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle(results);
            #endif
            break;
        case 1:
            #ifdef __AVX512F__
            test_gather_intrinsic(results);
            #endif
            break;
        case 2:
            test_atomic_ops(results);
            break;
        case 3:
            #ifdef _OPENMP
            test_openmp_simd(results);
            #endif
            break;
        case 4:
            test_many_operand_asm(results);
            break;
        case 5:
            #ifdef __ARM_NEON
            test_neon_operations(results);
            #endif
            break;
    }
    
    /* Compute checksum to ensure execution */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= results[i];
    }
    
    printf("Pattern %d executed, checksum: %d\n", pattern, checksum);
    
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void* ptr) {
    /* Empty but referenced to keep data alive */
    (void)ptr;
}
