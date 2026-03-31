/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's optabs expansion for 10 and 11-operand
 * instruction patterns by using various high-operand-count constructs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==============================
 * Pattern 1: Large Vector Shuffle
 * ============================== */
__attribute__((noipa, noinline))
static void test_vector_shuffle(volatile int* result) {
    /* Use GCC vector extensions with many shuffle indices */
    typedef int v16si __attribute__((vector_size(64)));
    
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices - during expansion this may require many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23);
    
    /* Store result to volatile memory */
    memcpy((void*)result, &c, sizeof(c));
    use((void*)result);
}

/* ==============================
 * Pattern 2: x86 Gather Intrinsic (if compiled for x86)
 * ============================== */
#ifdef __x86_64__
__attribute__((noipa, noinline))
static void test_gather_intrinsic(volatile int* result) {
    /* AVX-512 gather instruction with many parameters */
    typedef double v8df __attribute__((vector_size(64)));
    typedef long long v8di __attribute__((vector_size(64)));
    typedef long long v8dm __attribute__((vector_size(8)));
    
    double base[64] __attribute__((aligned(64)));
    v8di index = {0, 8, 16, 24, 32, 40, 48, 56};
    v8df scale_vec = {8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0};
    v8dm mask = {-1, -1, -1, -1, -1, -1, -1, -1};
    
    /* Initialize base array */
    for (int i = 0; i < 64; i++) {
        base[i] = i * 1.5;
    }
    
    /* Use gather intrinsic - expands to many operands */
    v8df gathered;
    
    /* Note: Actual intrinsic name may vary by GCC version */
    /* This is a placeholder for the actual gather intrinsic */
    asm volatile (
        "vmovupd %1, %%zmm0\n\t"
        "vmovupd %2, %%zmm1\n\t"
        "vmovupd %3, %%zmm2\n\t"
        "kmovq   %4, %%k1\n\t"
        "vgatherqpd %0{%%k1}, (%%zmm0, %%zmm1, 8)\n\t"
        : "=v"(gathered)
        : "v"((v8df)base), "v"((v8df)index), "v"(scale_vec), "r"(mask)
        : "zmm0", "zmm1", "zmm2", "k1", "memory"
    );
    
    memcpy((void*)result, &gathered, sizeof(gathered));
    use((void*)result);
}
#endif

/* ==============================
 * Pattern 3: Atomic Compare Exchange with Many Parameters
 * ============================== */
__attribute__((noipa, noinline))
static void test_atomic_ops(volatile int* result) {
    volatile int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0; /* Use strong compare-exchange */
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                           weak, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    *result = success + atomic_var;
    use((void*)result);
}

/* ==============================
 * Pattern 4: OpenMP SIMD with Many Clauses
 * ============================== */
__attribute__((noipa, noinline))
static void test_openmp_simd(volatile int* result) {
    #define N 1024
    static int a[N] __attribute__((aligned(64)));
    static int b[N] __attribute__((aligned(64)));
    static int c[N] __attribute__((aligned(64)));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        b[i] = i;
        c[i] = i * 2;
    }
    
    int i;
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32)
    for (i = 0; i < N; i++) {
        a[i] = b[i] + c[i] * 3;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i];
    }
    
    *result = sum;
    use((void*)result);
}

/* ==============================
 * Pattern 5: Inline Assembly with 10+ Operands
 * ============================== */
__attribute__((noipa, noinline))
static void test_multi_operand_asm(volatile int* result) {
    int out;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Inline asm with 10 operands (9 inputs, 1 output) */
    asm volatile (
        "/* Multi-operand operation */\n\t"
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0"
        : "=r"(out)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    *result = out + j; /* Now 11 values involved including j */
    use((void*)result);
}

/* ==============================
 * Pattern 6: Complex Vector Operation Chain
 * ============================== */
__attribute__((noipa, noinline))
static void test_vector_chain(volatile int* result) {
    /* Use vector operations that might chain to many operands */
    typedef float v16sf __attribute__((vector_size(64)));
    
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = v1 * 2.0f;
    v16sf v3 = v1 + v2;
    v16sf v4 = v3 - v1;
    v16sf v5 = v4 * v2;
    v16sf v6 = __builtin_shufflevector(v5, v1, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Fused multiply-add style operation */
    v16sf v7 = v6 + v1 * v2;
    
    memcpy((void*)result, &v7, sizeof(v7));
    use((void*)result);
}

/* ==============================
 * Main Function with Volatile Control Flow
 * ============================== */
int main(int argc, char *argv[]) {
    /* Create volatile result array */
    volatile int results[6] = {0};
    
    /* Generate seed from argv[0] */
    unsigned int seed = 0;
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Use seed to control which patterns execute */
    control = seed;
    
    /* Execute different patterns based on seed */
    switch (control % 6) {
        case 0:
            test_vector_shuffle(&results[0]);
            break;
        case 1:
#ifdef __x86_64__
            test_gather_intrinsic(&results[1]);
#else
            test_vector_chain(&results[1]);
#endif
            break;
        case 2:
            test_atomic_ops(&results[2]);
            break;
        case 3:
            test_openmp_simd(&results[3]);
            break;
        case 4:
            test_multi_operand_asm(&results[4]);
            break;
        case 5:
            test_vector_chain(&results[5]);
            break;
    }
    
    /* Compute final checksum to ensure all code affects output */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum += results[i];
    }
    
    printf("Result checksum: %d (seed: %u)\n", checksum, seed);
    return 0;
}

/* Dummy implementation of external function */
void use(void* ptr) {
    /* Prevent optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}
