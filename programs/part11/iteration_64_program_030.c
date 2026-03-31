/* test_optabs_10_11.c - Cover cases 10 and 11 in optabs.cc GEN_FCN calls */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Prevent interprocedural optimizations */
#define NOIPA __attribute__((noipa, noinline))

/* ========== Approach 1: Large Vector Shuffle ========== */
typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

NOIPA void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices = potentially many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23);
    
    /* Use volatile store to force computation */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i];
    }
    use((void*)result);
}

/* ========== Approach 2: x86 AVX-512 Gather Intrinsics ========== */
#ifdef __AVX512F__
#include <immintrin.h>

NOIPA void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather instructions often have many operands:
       base, scale, index, mask, etc. */
    __m512i index = _mm512_set_epi32(0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60);
    __mmask16 mask = 0xAAAA;
    int base[64];
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        index,                   // indices
        (const void*)base,       // base pointer
        4                        // scale
    );
    
    _mm512_storeu_si512((void*)result, gathered);
    use((void*)result);
}
#endif

/* ========== Approach 3: Atomic Operations ========== */
NOIPA void test_atomic_ops(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange has many parameters that expand to multiple operands */
    __atomic_compare_exchange(ptr, &expected, &desired, 
                              0,  /* weak */
                              __ATOMIC_SEQ_CST, 
                              __ATOMIC_RELAXED);
    
    /* Another atomic with many operands */
    __atomic_fetch_add(ptr, 1, __ATOMIC_SEQ_CST);
    __atomic_fetch_sub(ptr + 1, 1, __ATOMIC_SEQ_CST);
    __atomic_fetch_and(ptr + 2, 0xFF, __ATOMIC_SEQ_CST);
    __atomic_fetch_or(ptr + 3, 0x0F, __ATOMIC_SEQ_CST);
    __atomic_fetch_xor(ptr + 4, 0x55, __ATOMIC_SEQ_CST);
    
    use((void*)ptr);
}

/* ========== Approach 4: OpenMP SIMD with Many Clauses ========== */
NOIPA void test_openmp_simd(volatile int* result) {
    #define N 128
    int a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16) \
                     reduction(+:c[0:N]) if(simd:1)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    for (int i = 0; i < N; i++) {
        result[i % 16] += c[i];
    }
    use((void*)result);
}

/* ========== Approach 5: Multi-Operand Inline Assembly ========== */
NOIPA void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm template %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */\n"
        "add %0, %1, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand asm template */\n"
        "mov %0, %1\n"
        "add %0, %0, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9\n"
        "add %0, %0, %10"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    result[0] = out1 + out2;
    use((void*)result);
}

/* ========== Approach 6: AArch64 Specific Multi-Register Load ========== */
#ifdef __aarch64__
#include <arm_neon.h>

NOIPA void test_aarch64_multi_reg(volatile int* result) {
    /* AArch64 LD1 multiple structure loads can have many operands */
    int32x4x4_t data;
    int32_t src[16];
    for (int i = 0; i < 16; i++) src[i] = i;
    
    /* Load 4 registers - expands to multiple operands */
    data = vld1q_s32_x4(src);
    
    /* Store results */
    for (int i = 0; i < 4; i++) {
        vst1q_s32((int32_t*)result + i*4, data.val[i]);
    }
    use((void*)result);
}
#endif

/* ========== Main Execution Flow ========== */
int main(int argc, char *argv[]) {
    /* Use argv[0] to create a pseudo-random seed */
    unsigned seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    
    /* Allocate volatile memory to force actual operations */
    volatile int* results = (volatile int*)malloc(64 * sizeof(int));
    if (!results) return 1;
    
    /* Initialize results */
    for (int i = 0; i < 64; i++) results[i] = 0;
    
    /* Execute different test cases based on seed */
    switch (seed % 6) {
        case 0:
            test_vector_shuffle(results);
            break;
        case 1:
            #ifdef __AVX512F__
            test_avx512_gather(results);
            #else
            test_vector_shuffle(results);
            #endif
            break;
        case 2:
            test_atomic_ops(results);
            break;
        case 3:
            test_openmp_simd(results);
            break;
        case 4:
            test_multi_operand_asm(results);
            break;
        case 5:
            #ifdef __aarch64__
            test_aarch64_multi_reg(results);
            #else
            test_multi_operand_asm(results);
            #endif
            break;
    }
    
    /* Compute checksum to ensure all operations executed */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free((void*)results);
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void* ptr) {
    asm volatile ("" : : "r"(ptr) : "memory");
}
