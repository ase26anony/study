/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
static int __attribute__((noinline, noclone))
complex_helper(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a >> 2);
    }
}

static double __attribute__((noinline, noclone))
fp_helper(double x, double y, int scale) {
    switch (scale & 3) {
        case 0: return x * y + 1.0;
        case 1: return x / y - 2.0;
        case 2: return x + y * 3.0;
        default: return y - x / 4.0;
    }
}

/* Main test function with high register pressure */
static volatile long __attribute__((noinline))
test_remat(volatile int input1, volatile long input2, 
           volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a1 = input1 + 1;
    int a2 = input1 * 2;
    int a3 = input1 - 5;
    int a4 = input1 & 0xFF;
    int a5 = input1 | 0x55;
    int a6 = input1 ^ 0xAA;
    int a7 = a1 + a2;
    int a8 = a3 * a4;
    int a9 = a5 - a6;
    int a10 = a7 & a8;
    
    long b1 = input2 + 1000;
    long b2 = input2 * 3;
    long b3 = input2 - 500;
    long b4 = b1 ^ b2;
    long b5 = b3 | b4;
    long b6 = b1 + b2 + b3;
    long b7 = b4 * b5;
    long b8 = b6 - b7;
    long b9 = b8 >> 2;
    long b10 = b9 << 1;
    
    float c1 = input3 * 1.5f;
    float c2 = input3 / 2.0f;
    float c3 = c1 + c2;
    float c4 = c1 - c2;
    float c5 = c3 * c4;
    float c6 = c5 + input3;
    
    double d1 = input4 * 2.5;
    double d2 = input4 / 1.5;
    double d3 = d1 + d2;
    double d4 = d1 - d2;
    double d5 = d3 * d4;
    double d6 = d5 - input4;
    
    /* Vector variables for wider register pressure */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4si v3 = {a9, a10, a1, a2};
    v4sf vf1 = {c1, c2, c3, c4};
    v4sf vf2 = {c5, c6, c1, c2};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    
    /* Long serial chain of interdependent operations */
    int t1 = a1 + a2;
    int t2 = t1 * a3;
    int t3 = t2 - a4;
    int t4 = t3 | a5;
    int t5 = t4 & a6;
    int t6 = t5 + a7;
    int t7 = t6 * a8;
    int t8 = t7 - a9;
    int t9 = t8 | a10;
    int t10 = t9 & t1;
    
    long u1 = b1 + b2;
    long u2 = u1 * b3;
    long u3 = u2 - b4;
    long u4 = u3 | b5;
    long u5 = u4 & b6;
    long u6 = u5 + b7;
    long u7 = u6 * b8;
    long u8 = u7 - b9;
    long u9 = u8 | b10;
    long u10 = u9 & u1;
    
    float f1 = c1 + c2;
    float f2 = f1 * c3;
    float f3 = f2 - c4;
    float f4 = f3 * c5;
    float f5 = f4 + c6;
    
    double g1 = d1 + d2;
    double g2 = g1 * d3;
    double g3 = g2 - d4;
    double g4 = g3 * d5;
    double g5 = g4 + d6;
    
    /* Vector operations */
    v4si v4 = v1 + v2;
    v4si v5 = v4 * v3;
    v4si v6 = v5 - v1;
    
    v4sf vf3 = vf1 + vf2;
    v4sf vf4 = vf3 * vf1;
    
    v2df vd3 = vd1 + vd2;
    v2df vd4 = vd3 * vd1;
    
    /* Inline assembly to clobber physical registers */
    asm volatile (
#if defined(__x86_64__)
        "nop\n\t"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
#elif defined(__aarch64__)
        "nop\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
          "x24", "x25", "x26", "x27", "x28", "v0", "v1", "v2",
          "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11",
          "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19",
          "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27",
          "v28", "v29", "v30", "v31", "memory"
#elif defined(__arm__)
        "nop\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "d0", "d1", "d2",
          "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10",
          "d11", "d12", "d13", "d14", "d15", "memory"
#else
        "nop\n\t"
        :
        :
        : "memory"
#endif
    );
    
    /* Control flow to split basic blocks */
    if (t10 > 1000) {
        t10 = complex_helper(t10, t5, t2);
        v6 = v5 + v4;
    } else {
        t10 = complex_helper(t5, t10, t3);
        v6 = v5 - v4;
    }
    
    /* Recomputation of earlier values in different forms */
    /* This increases chances for early rematerialization */
    int t1_recomp = a2 + a1;  /* Same as t1 but different order */
    int t2_recomp = t1_recomp * a3;
    int t3_recomp = t2_recomp - a4;
    
    long u1_recomp = b2 + b1;
    long u2_recomp = u1_recomp * b3;
    
    float f1_recomp = c2 + c1;
    double g1_recomp = d2 + d1;
    
    /* More operations with recomputed values */
    int t11 = t10 + t1_recomp;
    int t12 = t11 * t2_recomp;
    int t13 = t12 - t3_recomp;
    
    long u11 = u10 + u1_recomp;
    long u12 = u11 * u2_recomp;
    
    float f6 = f5 + f1_recomp;
    double g6 = g5 + g1_recomp;
    
    /* Vector recomputation */
    v4si v4_recomp = v2 + v1;
    v4sf vf3_recomp = vf2 + vf1;
    
    /* Use helper functions for cross-block analysis */
    double fp_result = fp_helper(g6, d1, t13);
    
    /* Final computation using all major temporaries */
    volatile long result = (long)t13 + (long)u12 + (long)f6 + (long)fp_result;
    
    /* Use vector elements to ensure they're live */
    result += v6[0] + v6[1] + v6[2] + v6[3];
    result += (long)vf4[0] + (long)vf4[1];
    result += (long)vd4[0] + (long)vd4[1];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int input1 = 42;
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile long result = test_remat(
            input1 + (i & 0xF),
            input2 + i,
            input3 + (i * 0.01f),
            input4 + (i * 0.001)
        );
        total += result;
        
        /* Additional control flow */
        if (i % 100 == 0) {
            total ^= result;
        }
    }
    
    printf("Final result: %ld\n", (long)total);
    return 0;
}
