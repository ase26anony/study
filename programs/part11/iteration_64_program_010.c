/* Test program to cover 10/11-operand expansion cases in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== Pattern 1: Vector shuffle with many operands ========== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle requiring many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 11, 10, 9, 8, 31, 30, 29, 28, 27, 26, 25, 24);
    
    /* AVX-512 specific built-in that may use many operands */
    v8di idx = {0, 2, 4, 6, 8, 10, 12, 14};
    v8di src = {100, 200, 300, 400, 500, 600, 700, 800};
    v8di mask = {-1, -1, -1, -1, -1, -1, -1, -1};
    
    /* __builtin_ia32_gatherdiv8di may expand to many operands */
    long long* base = (long long*)&a;
    v8di gathered = __builtin_ia32_gatherdiv8di(src, base, idx, mask, 1);
    
    /* Store results to volatile memory */
    memcpy((void*)result, &c, sizeof(c));
    memcpy((void*)(result + 16), &d, sizeof(d));
    memcpy((void*)(result + 32), &gathered, sizeof(gathered));
    
    use((void*)result);
}
#endif

/* ========== Pattern 2: Atomic operations with many parameters ========== */
__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* __atomic_exchange with memory order */
    int old = __atomic_exchange_n(ptr, desired + success, __ATOMIC_ACQ_REL);
    
    /* Complex atomic add with memory order */
    __atomic_fetch_add(ptr, old, __ATOMIC_SEQ_CST);
    
    /* Store to volatile to ensure execution */
    result[1] = success;
    result[2] = old;
    
    use((void*)result);
}

/* ========== Pattern 3: OpenMP SIMD with complex clauses ========== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    #define N 128
    int a[N], b[N], c[N];
    
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
    
    /* Another SIMD loop with different clauses */
    #pragma omp simd collapse(2) private(i,j) lastprivate(k) \
                    aligned(a:32) linear(j:2)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int k = i * 8 + j;
            a[k] = b[k] - c[k];
        }
    }
    
    use((void*)result);
}
#endif

/* ========== Pattern 4: Inline assembly with many operands ========== */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm for 10 operands */\n\t"
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
        "/* Multi-operand asm for 11 operands */\n\t"
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
    
    result[0] = out1;
    result[1] = out2;
    
    use((void*)result);
}

/* ========== Pattern 5: Target-specific built-ins ========== */
#ifdef __aarch64__
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_aarch64_builtins(volatile int* result) {
    /* AArch64 multi-register load/store may use many operands */
    int32x4x4_t data;
    int32_t src[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    
    /* Load 4 registers - may expand to multi-operand instruction */
    data = vld1q_s32_x4(src);
    
    /* Store with post-increment */
    vst1q_s32_x4((int32_t*)result, data);
    
    /* Complex SIMD operation with lane selection */
    int32x4_t a = vld1q_s32(src);
    int32x4_t b = vld1q_s32(src + 4);
    int32x4_t c = vld1q_s32(src + 8);
    
    /* Multiple operand SIMD operation */
    int32x4_t res = vmlaq_s32(a, b, c);
    res = vqdmulhq_s32(res, a);
    res = vrhaddq_s32(res, b);
    
    vst1q_s32((int32_t*)(result + 16), res);
    
    use((void*)result);
}
#endif

/* ========== Main function with volatile control flow ========== */
int main(int argc, char* argv[]) {
    /* Create volatile result array */
    volatile int results[256] = {0};
    
    /* Simple hash from argv[0] for control flow */
    unsigned seed = 0;
    if (argc > 0 && argv[0]) {
        for (char* p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    control = seed;
    
    /* Call different test functions based on seed */
    int test_case = seed % 5;
    
    switch (test_case) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle(results);
            #endif
            break;
        case 1:
            test_atomic_ops(results);
            break;
        case 2:
            #ifdef _OPENMP
            test_openmp_simd(results);
            #endif
            break;
        case 3:
            test_multi_operand_asm(results);
            break;
        case 4:
            #ifdef __aarch64__
            test_aarch64_builtins(results);
            #endif
            break;
    }
    
    /* Compute checksum to ensure code executed */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= results[i];
    }
    
    printf("Checksum: %d (control: %d, test_case: %d)\n", 
           checksum, control, test_case);
    
    return 0;
}
