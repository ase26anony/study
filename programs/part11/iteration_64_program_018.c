/* test_optabs_coverage.c - Cover 10/11 operand expansion cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== Pattern 1: Vector Shuffle with Many Elements ==================== */

typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* This shuffle uses 16 indices + 2 vectors = 18 operands in RTL expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Force memory operations */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    
    use(&c);
    use(&d);
}

/* ==================== Pattern 2: x86 AVX-512 Gather Intrinsics ==================== */

#ifdef __x86_64__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i;
    
    /* __m512i _mm512_i32gather_epi32(__m512i vindex, void const * base, int scale) */
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Store result */
    _mm512_store_epi32((void*)result, gathered);
    
    use(&gathered);
}
#endif

/* ==================== Pattern 3: Atomic Operations ==================== */

__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange with many parameters */
    __atomic_compare_exchange(ptr, &expected, &desired, 
                              0, /* weak */
                              __ATOMIC_SEQ_CST, 
                              __ATOMIC_ACQUIRE);
    
    /* Another atomic with multiple memory orders */
    int val = __atomic_load_n(ptr, __ATOMIC_RELAXED);
    __atomic_store_n(ptr + 1, val + 1, __ATOMIC_RELEASE);
    
    /* Atomic exchange with memory order */
    int old = __atomic_exchange_n(ptr + 2, 100, __ATOMIC_ACQ_REL);
    
    use(&old);
}

/* ==================== Pattern 4: OpenMP SIMD with Many Clauses ==================== */

__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result, int n) {
    int a[256] __attribute__((aligned(64)));
    int b[256] __attribute__((aligned(64)));
    int c[256] __attribute__((aligned(64)));
    
    for (int i = 0; i < 256; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with multiple clauses - can expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(32) \
                reduction(+:result[0])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
        result[0] += c[i];
    }
    
    use(a);
    use(b);
    use(c);
}

/* ==================== Pattern 5: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand test %0 = (%1 + %2) * (%3 + %4) + (%5 + %6) - (%7 + %8) / %9 */\n\t"
        "addl %1, %2\n\t"
        "addl %3, %4\n\t"
        "imull %%eax, %%ebx\n\t"
        "addl %5, %6\n\t"
        "addl %7, %8\n\t"
        "movl %%ebx, %0\n\t"
        "addl %%ecx, %0\n\t"
        "subl %%edx, %0"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), 
          "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand test */\n\t"
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "addl %4, %%eax\n\t"
        "addl %5, %%eax\n\t"
        "addl %6, %%eax\n\t"
        "addl %7, %%eax\n\t"
        "addl %8, %%eax\n\t"
        "addl %9, %%eax\n\t"
        "addl %10, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "eax", "memory"
    );
    
    result[0] = out1 + out2;
    
    use(&out1);
    use(&out2);
}

/* ==================== Pattern 6: AArch64 Specific Multi-register Load/Store ==================== */

#ifdef __aarch64__
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_aarch64_neon(volatile int* result) {
    /* AArch64 has multi-register load/store instructions */
    int32x4x4_t quad_vec;
    int32_t data[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    
    /* Load 4 registers - expands to multiple operands */
    quad_vec = vld1q_s32_x4(data);
    
    /* Perform operations */
    quad_vec.val[0] = vaddq_s32(quad_vec.val[0], quad_vec.val[1]);
    quad_vec.val[2] = vmulq_s32(quad_vec.val[2], quad_vec.val[3]);
    
    /* Store back */
    vst1q_s32_x4((int32_t*)result, quad_vec);
    
    use(&quad_vec);
}
#endif

/* ==================== Main Function with Volatile Control Flow ==================== */

int main(int argc, char *argv[]) {
    /* Create volatile seed from argv[0] */
    unsigned seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    
    /* Allocate aligned memory for results */
    volatile int* results = aligned_alloc(64, 256 * sizeof(int));
    if (!results) return 1;
    
    /* Initialize results */
    for (int i = 0; i < 256; i++) {
        results[i] = 0;
    }
    
    /* Execute different patterns based on seed */
    int pattern = seed % 6;
    
    switch (pattern) {
        case 0:
            test_vector_shuffle(results);
            break;
        case 1:
            #ifdef __x86_64__
            test_avx512_gather(results);
            #else
            test_vector_shuffle(results);
            #endif
            break;
        case 2:
            test_atomic_ops(results);
            break;
        case 3:
            test_openmp_simd(results, 128);
            break;
        case 4:
            test_multi_operand_asm(results);
            break;
        case 5:
            #ifdef __aarch64__
            test_aarch64_neon(results);
            #else
            test_multi_operand_asm(results);
            #endif
            break;
    }
    
    /* Compute checksum to ensure code executed */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += results[i];
    }
    
    printf("Pattern %d executed. Checksum: %d\n", pattern, checksum);
    
    free((void*)results);
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void* ptr) {
    /* Empty but referenced to keep variables alive */
    (void)ptr;
}
