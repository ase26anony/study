/* test_optabs_multioperand.c - Cover 10/11 operand expansion cases in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== Pattern 1: Vector shuffles with many elements ========== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
static void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle requiring many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 31, 30, 29, 28, 11, 10, 9, 8, 27, 26, 25, 24);
    
    /* Use AVX-512 specific shuffle for more operands */
    v8df da = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
    v8df db = {8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0};
    v8df dc = __builtin_shufflevector(da, db, 0,8,1,9,2,10,3,11);
    
    /* Force memory operations */
    memcpy((void*)result, &c, sizeof(c));
    memcpy((void*)(result + 16), &d, sizeof(d));
    memcpy((void*)(result + 32), &dc, sizeof(dc));
    
    use((void*)result);
}
#endif

/* ========== Pattern 2: Gather intrinsics (x86 AVX-512) ========== */
#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline))
static void test_gather_intrinsic(volatile int* result) {
    double base[64] __attribute__((aligned(64)));
    int indices[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    __mmask8 mask = 0xFF;
    
    /* __builtin_ia32_gathersiv8df expands to many operands */
    __m512d gathered = _mm512_i32gather_pd(vindex, (const void*)base, 8);
    
    /* Another gather with different scale */
    __m512d gathered2 = _mm512_i32gather_pd(vindex, (const void*)base, 4);
    
    /* Store results */
    _mm512_storeu_pd((double*)result, gathered);
    _mm512_storeu_pd((double*)(result + 8), gathered2);
    
    use((void*)result);
}
#endif

/* ========== Pattern 3: Atomic operations with many parameters ========== */
__attribute__((noipa, noinline))
static void test_atomic_operations(volatile int* result) {
    volatile int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0; /* strong compare-exchange */
    
    /* __atomic_compare_exchange with all parameters */
    int success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                           weak, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);
    
    /* Another atomic with different memory orders */
    int val = 50;
    __atomic_exchange(&atomic_var, &val, &expected, __ATOMIC_RELEASE);
    
    /* Atomic fetch-add with memory order */
    int old = __atomic_fetch_add(&atomic_var, 10, __ATOMIC_ACQ_REL);
    
    result[0] = success;
    result[1] = expected;
    result[2] = old;
    result[3] = atomic_var;
    
    use((void*)result);
}

/* ========== Pattern 4: OpenMP SIMD with many clauses ========== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
static void test_openmp_simd(volatile int* result) {
    #define N 1024
    static double a[N] __attribute__((aligned(64)));
    static double b[N] __attribute__((aligned(64)));
    static double c[N] __attribute__((aligned(64)));
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.5;
        b[i] = i * 1.5;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) \
                simdlen(8) safelen(16) reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i] + i;
        result[0] += (int)c[i];
    }
    
    /* Another loop with collapse clause */
    #pragma omp simd collapse(2) aligned(a,b:32) simdlen(4)
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            int idx = i * 32 + j;
            a[idx] = b[idx] * 2.0;
        }
    }
    
    use((void*)result);
    #undef N
}
#endif

/* ========== Pattern 5: Many-operand inline assembly ========== */
__attribute__((noipa, noinline))
static void test_many_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* 10-operand test */\n\t"
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "imul %9, %10\n\t"
        "mov %11, %0"
        : "=r" (out1)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand test */\n\t"
        "lea (%1,%2), %%rax\n\t"
        "add %3, %%rax\n\t"
        "add %4, %%rax\n\t"
        "add %5, %%rax\n\t"
        "add %6, %%rax\n\t"
        "add %7, %%rax\n\t"
        "add %8, %%rax\n\t"
        "add %9, %%rax\n\t"
        "add %10, %%rax\n\t"
        "add %11, %%rax\n\t"
        "mov %%rax, %0"
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j), "r" (k)
        : "rax", "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use((void*)result);
}

/* ========== Pattern 6: Complex vector operations ========== */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noipa, noinline))
static void test_neon_operations(volatile int* result) {
    /* Use NEON intrinsics that may expand to many operands */
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    
    /* Multiple vector operations in sequence */
    int32x4_t r1 = vaddq_s32(a, b);
    int32x4_t r2 = vmlaq_s32(c, d, a);  /* c + d * a */
    int32x4_t r3 = vqdmulhq_s32(b, c);
    
    /* Vector table lookup (can be multi-operand) */
    int8x16_t table = vcombine_s8(vcreate_s8(0x0706050403020100ULL),
                                 vcreate_s8(0x0F0E0D0C0B0A0908ULL));
    int8x16_t indices = vdupq_n_s8(0);
    int8x16_t lookup = vtbl1q_s8(table, indices);
    
    vst1q_s32((int32_t*)result, r1);
    vst1q_s32((int32_t*)(result + 4), r2);
    vst1q_s32((int32_t*)(result + 8), r3);
    
    use((void*)result);
}
#endif

/* ========== Main test driver ========== */
int main(int argc, char *argv[]) {
    /* Create volatile result buffer */
    volatile int results[256] = {0};
    
    /* Simple hash from program name for control flow */
    unsigned int seed = 0;
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    control = seed;
    
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
            test_atomic_operations(results);
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
    for (int i = 0; i < 256; i++) {
        checksum ^= results[i];
    }
    
    printf("Test completed with checksum: %d (pattern: %d)\n", checksum, pattern);
    
    return 0;
}

/* Dummy function to prevent optimization */
void use(void *ptr) {
    /* Empty but referenced to keep data live */
    (void)ptr;
}
