/* test_optabs_multioperand.c - Cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ====== Pattern 1: Vector shuffles with many operands ====== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

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
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    use(&c);
    use(&d);
}

/* AVX-512 gather intrinsic - often expands to many operands */
__attribute__((noipa, noinline))
void test_gather_intrinsic(volatile int* result) {
    double base[64] __attribute__((aligned(64)));
    int indices[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    v8df src = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    
    /* Initialize base array */
    for (int i = 0; i < 64; i++) {
        base[i] = i * 0.5;
    }
    
    /* Use gather intrinsic - this often requires many operands */
    v8df gathered;
    __m512i vindex = _mm512_loadu_si512(indices);
    __mmask8 mask = 0xFF;
    
    /* Different gather patterns based on control */
    if (control & 1) {
        gathered = _mm512_i32gather_pd(vindex, base, 8);
    } else {
        gathered = _mm512_mask_i32gather_pd(src, mask, vindex, base, 8);
    }
    
    /* Convert to int for storage */
    for (int i = 0; i < 8; i++) {
        result[i] = (int)gathered[i];
    }
    use(&gathered);
}
#endif

/* ====== Pattern 2: Atomic operations with many parameters ====== */
__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    intptr_t atomic_var = 0;
    intptr_t expected = 0;
    intptr_t desired = 42;
    intptr_t desired2 = 84;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                            0, /* weak */
                                            __ATOMIC_SEQ_CST,
                                            __ATOMIC_ACQUIRE);
    
    /* Another atomic exchange with different memory orders */
    if (success) {
        expected = 42;
        __atomic_compare_exchange(&atomic_var, &expected, &desired2,
                                  1, /* strong */
                                  __ATOMIC_RELEASE,
                                  __ATOMIC_RELAXED);
    }
    
    /* Atomic fetch-and-add with memory order */
    intptr_t old = __atomic_fetch_add(&atomic_var, 10, __ATOMIC_ACQ_REL);
    
    result[0] = (int)atomic_var;
    result[1] = success;
    result[2] = (int)old;
    
    use(&atomic_var);
}

/* ====== Pattern 3: OpenMP SIMD with complex clauses ====== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    const int N = 128;
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    int d[N] __attribute__((aligned(64)));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = 0;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                simdlen(8) safelen(16) reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i] * c[i];
        result[0] += d[i];
    }
    
    /* Another SIMD loop with different clauses */
    #pragma omp simd aligned(a,d:64) simdlen(16) collapse(2) \
                private(i,j) lastprivate(k)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            int k = i * 16 + j;
            a[k] = d[k] - b[k];
        }
    }
    
    use(a);
    use(b);
    use(c);
    use(d);
}
#endif

/* ====== Pattern 4: Inline assembly with many operands ====== */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand assembly statement */
    asm volatile (
        "/* Multi-operand operation */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (out1)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* 11-operand assembly statement */
    asm volatile (
        "/* Another multi-operand operation */\n\t"
        "imul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "sub %0, %0, %4\n\t"
        "and %0, %0, %5\n\t"
        "or %0, %0, %6\n\t"
        "xor %0, %0, %7\n\t"
        "shl %0, %0, %8\n\t"
        "shr %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use(&out1);
    use(&out2);
}

/* ====== Pattern 5: Complex built-in with many arguments ====== */
#ifdef __aarch64__
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_aarch64_multi_reg(volatile int* result) {
    int32x4_t a = {1, 2, 3, 4};
    int32x4_t b = {5, 6, 7, 8};
    int32x4_t c = {9, 10, 11, 12};
    int32x4_t d = {13, 14, 15, 16};
    
    /* Complex NEON operations that may expand to many operands */
    int32x4x4_t quad;
    quad.val[0] = vaddq_s32(a, b);
    quad.val[1] = vsubq_s32(c, d);
    quad.val[2] = vmulq_s32(a, c);
    quad.val[3] = vmlaq_s32(b, d, a);
    
    /* Table lookup with many operands */
    int8x16_t table1 = vcombine_s8(vcreate_s8(0x0706050403020100ULL),
                                   vcreate_s8(0x0F0E0D0C0B0A0908ULL));
    int8x16_t table2 = vcombine_s8(vcreate_s8(0x1716151413121110ULL),
                                   vcreate_s8(0x1F1E1D1C1B1A1918ULL));
    
    int8x16_t indices = vdupq_n_s8(0);
    int8x16_t tbl_result = vqtbl2q_s8(vcombine_s8x2(table1, table2), indices);
    
    /* Store results */
    vst1q_s32((int32_t*)&result[0], quad.val[0]);
    vst1q_s32((int32_t*)&result[4], quad.val[1]);
    vst1q_s32((int32_t*)&result[8], quad.val[2]);
    vst1q_s32((int32_t*)&result[12], quad.val[3]);
    
    use(&quad);
    use(&tbl_result);
}
#endif

/* ====== Main test driver ====== */
int main(int argc, char *argv[]) {
    /* Use argv[0] to create a pseudo-random seed */
    unsigned int seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    control = seed;
    
    /* Result arrays */
    volatile int results[5][32] = {0};
    int checksum = 0;
    
    /* Execute different patterns based on seed */
    int pattern = seed % 5;
    
    switch (pattern) {
        case 0:
#ifdef __AVX512F__
            test_vector_shuffle(results[0]);
#endif
            break;
        case 1:
            test_atomic_operations(results[1]);
            break;
        case 2:
#ifdef _OPENMP
            test_openmp_simd(results[2]);
#endif
            break;
        case 3:
            test_multi_operand_asm(results[3]);
            break;
        case 4:
#ifdef __aarch64__
            test_aarch64_multi_reg(results[4]);
#endif
            break;
    }
    
    /* Force execution of all patterns if control has specific bits set */
    if (control & 0x1) {
#ifdef __AVX512F__
        test_gather_intrinsic(results[0]);
#endif
    }
    
    /* Compute checksum to ensure all code affects output */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 32; j++) {
            checksum ^= results[i][j];
            checksum = (checksum << 1) | (checksum >> 31);
        }
    }
    
    printf("Checksum: %d (seed: %u, pattern: %d)\n", checksum, seed, pattern);
    
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void* ptr) {
    /* Empty but referenced to keep variables alive */
    (void)ptr;
}
