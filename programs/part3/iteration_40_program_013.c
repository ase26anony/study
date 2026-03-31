/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper functions to split basic blocks */
__attribute__((noinline, noclone))
static int helper1(int a, int b, int c) {
    if (a > b) return a * c;
    return b * c + 1;
}

__attribute__((noinline, noclone))
static float helper2(float a, float b, float c) {
    switch ((int)a % 4) {
        case 0: return a + b * c;
        case 1: return a - b * c;
        case 2: return a * b + c;
        default: return a * b - c;
    }
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b, v4si c) {
    v4si t1 = a + b;
    v4si t2 = b - c;
    v4si t3 = a * c;
    return t1 + t2 - t3;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a1 = input1 + 1;
    int a2 = input1 * 2;
    int a3 = input1 - 5;
    int a4 = input1 / 2;
    int a5 = input1 % 7;
    int a6 = input1 | 0xFF;
    int a7 = input1 & 0x0F;
    int a8 = input1 ^ 0x55;
    int a9 = ~input1;
    int a10 = input1 << 2;
    
    long b1 = input2 + 1000;
    long b2 = input2 * 3;
    long b3 = input2 - 500;
    long b4 = input2 / 3;
    long b5 = input2 % 11;
    long b6 = input2 | 0xFFFF;
    long b7 = input2 & 0xFF00;
    long b8 = input2 ^ 0xAAAA;
    long b9 = ~input2;
    long b10 = input2 >> 1;
    
    float c1 = input3 + 1.5f;
    float c2 = input3 * 2.5f;
    float c3 = input3 - 0.5f;
    float c4 = input3 / 3.0f;
    float c5 = input3 * input3;
    float c6 = 1.0f / input3;
    float c7 = input3 + input3;
    float c8 = input3 - input3 * 0.1f;
    float c9 = input3 * 0.9f;
    float c10 = input3 / 0.8f;
    
    double d1 = input4 + 2.5;
    double d2 = input4 * 1.5;
    double d3 = input4 - 0.25;
    double d4 = input4 / 2.0;
    double d5 = input4 * input4;
    double d6 = 1.0 / input4;
    double d7 = input4 + input4 * 0.1;
    double d8 = input4 - input4 * 0.2;
    double d9 = input4 * 0.75;
    double d10 = input4 / 0.9;
    
    /* Vector variables for additional pressure */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4si v3 = {a9, a10, a1, a2};
    v4sf vf1 = {c1, c2, c3, c4};
    v4sf vf2 = {c5, c6, c7, c8};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    
    /* Long serial chain of interdependent operations */
    int t1 = a1 + a2;
    int t2 = t1 * a3;
    int t3 = t2 - a4;
    int t4 = t3 / (a5 ? a5 : 1);
    int t5 = t4 | a6;
    int t6 = t5 & a7;
    int t7 = t6 ^ a8;
    int t8 = helper1(t7, a9, a10);
    
    long t9 = b1 + b2;
    long t10 = t9 * b3;
    long t11 = t10 - b4;
    long t12 = t11 / (b5 ? b5 : 1);
    long t13 = t12 | b6;
    long t14 = t13 & b7;
    long t15 = t14 ^ b8;
    long t16 = t15 + b9 - b10;
    
    float t17 = c1 + c2;
    float t18 = t17 * c3;
    float t19 = t18 - c4;
    float t20 = t19 / (c5 ? c5 : 1.0f);
    float t21 = helper2(t20, c6, c7);
    float t22 = t21 + c8 - c9;
    float t23 = t22 * c10;
    
    double t24 = d1 + d2;
    double t25 = t24 * d3;
    double t26 = t25 - d4;
    double t27 = t26 / (d5 ? d5 : 1.0);
    double t28 = t27 + d6 * d7;
    double t29 = t28 - d8 + d9;
    double t30 = t29 * d10;
    
    /* Vector operations */
    v4si vt1 = v1 + v2;
    v4si vt2 = vt1 * v3;
    v4si vt3 = helper3(vt2, v1, v2);
    
    v4sf vtf1 = vf1 + vf2;
    v4sf vtf2 = vtf1 * vf1;
    
    v2df vtd1 = vd1 + vd2;
    v2df vtd2 = vtd1 * vd1;
    
    /* Inline assembly to clobber physical registers */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Complex expression that gets recomputed later */
    int complex_expr = (t1 * t2 + t3 - t4) | (t5 & t6) ^ (t7 + t8);
    
    /* Use complex_expr in multiple statements */
    int use1 = complex_expr + t1;
    int use2 = complex_expr * t2;
    int use3 = complex_expr - t3;
    int use4 = complex_expr | t4;
    
    /* Control flow to split basic blocks */
    if (complex_expr > 1000) {
        use1 = helper1(use1, use2, use3);
        vt1 = vt1 + vt2;
    } else {
        use4 = helper1(use4, use1, use2);
        vt1 = vt1 - vt2;
    }
    
    /* Recomputation of similar expression */
    int recomputed_expr = (t1 * t2 + t3 - t4) | (t5 & t6) ^ (t7 + t8 + 1);
    
    /* More operations with recomputed value */
    int use5 = recomputed_expr + use1;
    int use6 = recomputed_expr * use2;
    int use7 = recomputed_expr - use3;
    int use8 = recomputed_expr | use4;
    
    /* More vector operations */
    v4si vt4 = vt3 + vt1;
    v4sf vtf3 = vtf2 + vtf1;
    v2df vtd3 = vtd2 + vtd1;
    
    /* Final computation using all temporaries */
    volatile int result = 
        (t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8) +
        (t9 % 1000) + (t10 % 1000) + (t11 % 1000) +
        ((int)t17 + (int)t18 + (int)t19 + (int)t20) +
        ((int)t24 + (int)t25 + (int)t26) +
        vt1[0] + vt2[1] + vt3[2] + vt4[3] +
        (int)vtf1[0] + (int)vtf2[1] + (int)vtf3[2] +
        (int)vtd1[0] + (int)vtd2[1] + (int)vtd3[0] +
        use1 + use2 + use3 + use4 + use5 + use6 + use7 + use8 +
        complex_expr + recomputed_expr;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile int total = 0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789L;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = (seed2 * 6364136223846793005L + 1442695040888963407L);
        seed3 = seed3 * 1.1f + 0.5f;
        seed4 = seed4 * 1.05 + 0.25;
        
        int result = test_remat(seed1 % 1000, 
                               seed2 % 10000,
                               seed3,
                               seed4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile ("# Loop barrier" ::: "memory");
    }
    
    printf("Final result: %d\n", total);
    return total != 0 ? 0 : 1;
}
