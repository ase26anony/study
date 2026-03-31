/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
__attribute__((noinline, noclone))
static int compute_branch(int a, int b, int c) {
    if (a > b) {
        return a * c - b;
    } else if (a < b) {
        return b * c + a;
    } else {
        return (a + b) * c;
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v4sf v) {
    double sum = v[0] + v[1] + v[2] + v[3];
    return sum * 0.25;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile double test_remat(volatile int input1, volatile long input2,
                                  volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a1 = input1 + 1;
    long b1 = input2 - 2;
    float c1 = input3 * 3.14f;
    double d1 = input4 / 2.71828;
    
    int a2 = a1 * 2;
    long b2 = b1 + a1;
    float c2 = c1 - input3;
    double d2 = d1 * input4;
    
    int a3 = a2 ^ b1;
    long b3 = b2 | a2;
    float c3 = c2 + c1;
    double d3 = d2 - d1;
    
    int a4 = a3 << 2;
    long b4 = b3 >> 1;
    float c4 = c3 * 2.0f;
    double d4 = d3 / 3.0;
    
    int a5 = ~a4;
    long b5 = -b4;
    float c5 = -c4;
    double d5 = -d4;
    
    int a6 = a5 & 0xFF;
    long b6 = b5 & 0xFFFF;
    float c6 = c5 * 0.5f;
    double d6 = d5 * 0.25;
    
    int a7 = a6 | 0xAA;
    long b7 = b6 | 0x5555;
    float c7 = c6 + 1.0f;
    double d7 = d6 + 1.0;
    
    int a8 = a7 ^ a6;
    long b8 = b7 ^ b6;
    float c8 = c7 - c6;
    double d8 = d7 - d6;
    
    int a9 = a8 * 3;
    long b9 = b8 * 5;
    float c9 = c8 * 1.5f;
    double d9 = d8 * 1.25;
    
    int a10 = a9 / 2;
    long b10 = b9 / 3;
    float c10 = c9 / 2.0f;
    double d10 = d9 / 2.0;
    
    /* Vector variables for additional pressure */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4sf v3 = {c1, c2, c3, c4};
    v4sf v4 = {c5, c6, c7, c8};
    v2df v5 = {d1, d2};
    v2df v6 = {d3, d4};
    
    /* Long serial chain of interdependent operations */
    int t1 = a1 + a2;
    long t2 = b1 * b2;
    float t3 = c1 - c2;
    double t4 = d1 + d2;
    
    int t5 = t1 * a3;
    long t6 = t2 + b3;
    float t7 = t3 * c3;
    double t8 = t4 - d3;
    
    int t9 = t5 ^ a4;
    long t10 = t6 | b4;
    float t11 = t7 + c4;
    double t12 = t8 * d4;
    
    int t13 = t9 & a5;
    long t14 = t10 ^ b5;
    float t15 = t11 - c5;
    double t16 = t12 / d5;
    
    /* Vector operations */
    v4si v7 = v1 + v2;
    v4si v8 = v1 * v2;
    v4sf v9 = v3 + v4;
    v4sf v10 = v3 * v4;
    v2df v11 = v5 + v6;
    v2df v12 = v5 * v6;
    
    /* Control flow to create multiple basic blocks */
    if (t1 > t5) {
        t13 = compute_branch(t1, t5, t9);
        v7 = v7 * 2;
    } else {
        t14 = compute_branch(t5, t1, t10);
        v8 = v8 + v1;
    }
    
    switch (t13 & 0x3) {
        case 0:
            t15 = t11 * 2.0f;
            v9 = v9 - v3;
            break;
        case 1:
            t16 = t12 * 1.5;
            v10 = v10 / 2.0f;
            break;
        case 2:
            t15 = t11 / 2.0f;
            v11 = v11 * 1.25;
            break;
        default:
            t16 = t12 / 1.5;
            v12 = v12 - v5;
            break;
    }
    
    /* Inline assembly to clobber registers (x86_64 version) */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        "nop"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Recomputation of values in slightly different forms */
    /* This increases chances for early rematerialization */
    int t17 = t1 + t5;  /* Similar to earlier t1 + a2 + a3 */
    long t18 = t2 * t6; /* Similar to earlier b1 * b2 + b3 */
    
    /* More complex expressions that might be rematerialized */
    float t19 = (t3 * 2.0f) - (c1 + c2) / 3.0f;
    double t20 = (t4 / 1.5) + (d1 - d2) * 2.0;
    
    /* Vector recomputation */
    v4si v13 = v7 + v8;
    v4sf v14 = v9 * v10;
    v2df v15 = v11 - v12;
    
    /* More operations using recomputed values */
    int t21 = t17 * t13;
    long t22 = t18 + t14;
    float t23 = t19 - t15;
    double t24 = t20 * t16;
    
    /* Final reduction that uses most variables */
    double result = (double)t21 + (double)t22 + (double)t23 + t24;
    result += vector_reduce(v14);
    result += (double)v13[0] + (double)v13[1] + (double)v13[2] + (double)v13[3];
    result += v15[0] + v15[1];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile double total = 0.0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = seed2 * 6364136223846793005ULL + 1442695040888963407ULL;
        seed3 = seed3 * 1.01f + 0.5f;
        seed4 = seed4 * 1.001 + 0.1;
        
        total += test_remat(seed1, seed2, seed3, seed4);
    }
    
    printf("Result: %f\n", (double)total);
    return 0;
}
