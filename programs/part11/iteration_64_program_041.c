/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's optabs expansion for 10 and 11-operand
 * instruction patterns by using various language features that map to
 * complex multi-operand instructions during RTL expansion.
 */

#include <stdio.h>
#include <stdint.h>
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

__attribute__((noipa, noinline, target("avx512f")))
void test_vector_shuffle_10_operands(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* This shuffle uses 16 indices + 2 vectors = 18 operands in total
     * During expansion, this may be broken down into multiple operations
     * Some of which could require 10-11 operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Use AVX-512 specific gather intrinsic which often has many operands */
    long long base[64];
    v8di index = {0,8,16,24,32,40,48,56};
    v8di mask = {-1,-1,-1,-1,-1,-1,-1,-1};
    
    for (int i = 0; i < 64; i++) base[i] = i;
    
    /* __builtin_ia32_gather3div8di has 8 operands in the intrinsic,
     * but expands to more during RTL generation */
    v8di gathered = __builtin_ia32_gather3div8di(
        (__v8di){0}, 
        (long long const *)base, 
        index, 
        mask, 
        1);
    
    /* Combine results to ensure both are used */
    int sum = 0;
    for (int i = 0; i < 16; i++) sum += c[i];
    for (int i = 0; i < 8; i++) sum += gathered[i];
    
    *result = sum;
    use(&c);
    use(&gathered);
}
#endif

/* ==================== Pattern 2: Atomic Operations with Many Parameters ==================== */

__attribute__((noipa, noinline))
void test_atomic_11_operands(volatile int* result) {
    intptr_t ptr_val = 0x1000;
    int* volatile ptr = (int*)ptr_val;
    int expected = 42;
    int desired = 43;
    int weak = 0; /* strong version */
    
    /* __atomic_compare_exchange has 6 explicit arguments, but expands
     * to many more operands during RTL generation, especially for
     * complex memory models and weak/strong variants */
    int success = __atomic_compare_exchange(
        ptr, 
        &expected, 
        &desired, 
        weak,  /* weak */
        __ATOMIC_SEQ_CST,  /* success_memorder */
        __ATOMIC_ACQUIRE   /* failure_memorder */
    );
    
    /* Additional atomic operation with many parameters */
    int val = 100;
    int old = __atomic_fetch_add(&val, 10, __ATOMIC_SEQ_CST);
    
    *result = success + old + expected + desired;
    use(&success);
    use(&old);
}

/* ==================== Pattern 3: OpenMP SIMD with Many Clauses ==================== */

#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd_10_operands(volatile int* result) {
    #define N 1024
    static int a[N] __attribute__((aligned(64)));
    static int b[N] __attribute__((aligned(64)));
    static int c[N] __attribute__((aligned(64)));
    static int d[N] __attribute__((aligned(64)));
    
    /* Initialize with volatile to prevent optimization */
    for (volatile int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = 0;
    }
    
    /* OpenMP SIMD with multiple clauses - during expansion,
     * each clause adds operands to the internal representation */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                simdlen(16) safelen(32) \
                reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i] * c[i];
        result[0] += d[i];
    }
    
    /* Additional complex SIMD operation */
    #pragma omp simd collapse(2) simdlen(8)
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            a[i*32 + j] += b[i*32 + j] - c[i*32 + j];
        }
    }
    
    use(a);
    use(b);
    use(c);
    use(d);
}
#endif

/* ==================== Pattern 4: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
void test_inline_asm_11_operands(volatile int* result) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Inline asm with 10 explicit register operands plus the template */
    asm volatile (
        "/* Multi-operand assembly pattern */\n\t"
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
    
    /* Another asm with memory operands */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    asm volatile (
        "mov %0, [%1 + %2*4]\n\t"
        "add %0, %0, [%1 + %3*4]\n\t"
        "add %0, %0, [%1 + %4*4]\n\t"
        "add %0, %0, [%1 + %5*4]\n\t"
        "add %0, %0, [%1 + %6*4]"
        : "=r" (out2)
        : "r" (arr), "r" (1), "r" (2), "r" (3), "r" (4), "r" (5)
        : "memory"
    );
    
    *result = out1 + out2;
    use(&out1);
    use(&out2);
    use(arr);
}

/* ==================== Pattern 5: Complex Built-in with Many Arguments ==================== */

#ifdef __aarch64__
#include <arm_neon.h>

__attribute__((noipa, noinline, target("arch=armv8.2-a+simd")))
void test_aarch64_multi_reg_ops(volatile int* result) {
    /* AArch64 has multi-register load/store instructions that
     * can expand to many operands */
    int32x4x4_t quad_vec;
    int32_t data[16];
    
    for (int i = 0; i < 16; i++) data[i] = i;
    
    /* Load 4 registers at once - expands to multiple operands */
    quad_vec = vld1q_s32_x4(data);
    
    /* Perform operations on all vectors */
    int32x4_t sum = vaddq_s32(quad_vec.val[0], quad_vec.val[1]);
    sum = vaddq_s32(sum, quad_vec.val[2]);
    sum = vaddq_s32(sum, quad_vec.val[3]);
    
    /* Store back with multi-register store */
    vst1q_s32_x4(data, quad_vec);
    
    int total = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            total += quad_vec.val[i][j];
        }
    }
    
    *result = total + sum[0] + sum[1] + sum[2] + sum[3];
    use(&quad_vec);
    use(&sum);
    use(data);
}
#endif

/* ==================== Main Function with Volatile Control Flow ==================== */

int main(int argc, char *argv[]) {
    volatile int results[5] = {0};
    volatile int final_result = 0;
    
    /* Create a non-constant seed from argv[0] */
    unsigned seed = 0;
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    control = seed % 5;
    
    /* Execute different patterns based on seed to ensure
     * compiler considers all expansion paths */
    switch (control) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle_10_operands(&results[0]);
            #endif
            break;
        case 1:
            test_atomic_11_operands(&results[1]);
            break;
        case 2:
            #ifdef _OPENMP
            test_openmp_simd_10_operands(&results[2]);
            #endif
            break;
        case 3:
            test_inline_asm_11_operands(&results[3]);
            break;
        case 4:
            #ifdef __aarch64__
            test_aarch64_multi_reg_ops(&results[4]);
            #endif
            break;
    }
    
    /* Combine all results to ensure all code contributes */
    for (int i = 0; i < 5; i++) {
        final_result += results[i];
    }
    
    /* Use volatile to prevent optimization */
    volatile int output = final_result;
    
    printf("Result: %d (control=%d)\n", output, control);
    
    return output != 0 ? 0 : 1;
}

/* Dummy implementation of use() to satisfy linker */
void use(void* ptr) {
    /* Empty but prevents optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}
