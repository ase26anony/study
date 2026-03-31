/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper functions to split basic blocks */
__attribute__((noinline, noclone))
static int helper1(int a, int b, int c) {
    if (a > b) return a * c - b;
    return b * c + a;
}

__attribute__((noinline, noclone))
static float helper2(float a, float b, float c) {
    if (a < b) return a * b + c;
    return b * c - a;
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    v4si mask = {1, 2, 3, 4};
    return a * b + mask;
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
    int a6 = a1 + a2;
    int a7 = a3 * a4;
    int a8 = a5 ^ a6;
    int a9 = a7 | a8;
    int a10 = a9 & a6;
    
    long b1 = input2 + 1000;
    long b2 = input2 * 3;
    long b3 = input2 - 500;
    long b4 = b1 * b2;
    long b5 = b3 / 2;
    long b6 = b4 + b5;
    long b7 = b6 ^ b1;
    long b8 = b7 * b2;
    long b9 = b8 - b3;
    long b10 = b9 | b4;
    
    float c1 = input3 * 1.5f;
    float c2 = input3 + 2.3f;
    float c3 = input3 - 1.1f;
    float c4 = c1 * c2;
    float c5 = c3 / c1;
    float c6 = c4 + c5;
    float c7 = c6 * c2;
    float c8 = c7 - c3;
    float c9 = c8 / c4;
    float c10 = c9 + c5;
    
    double d1 = input4 * 2.5;
    double d2 = input4 + 3.7;
    double d3 = input4 - 1.3;
    double d4 = d1 * d2;
    double d5 = d3 / d1;
    double d6 = d4 + d5;
    double d7 = d6 * d2;
    double d8 = d7 - d3;
    double d9 = d8 / d4;
    double d10 = d9 + d5;
    
    /* Vector variables */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4si v3 = {a9, a10, a1, a2};
    v4sf vf1 = {c1, c2, c3, c4};
    v4sf vf2 = {c5, c6, c7, c8};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    
    /* Complex interdependent computation chain */
    int t1 = a1 + a2 - a3 * a4;
    int t2 = t1 ^ a5 | a6;
    int t3 = t2 * a7 - a8;
    int t4 = t3 / (a9 + 1) + a10;
    int t5 = helper1(t1, t2, t3);
    
    long t6 = b1 * b2 + b3;
    long t7 = t6 ^ b4 | b5;
    long t8 = t7 * b6 - b7;
    long t9 = t8 / (b8 + 1) + b9;
    long t10 = t9 | b10 & t6;
    
    float t11 = c1 * c2 + c3;
    float t12 = t11 / c4 - c5;
    float t13 = t12 * c6 + c7;
    float t14 = t13 - c8 / c9;
    float t15 = helper2(t11, t12, t13);
    
    double t16 = d1 * d2 + d3;
    double t17 = t16 / d4 - d5;
    double t18 = t17 * d6 + d7;
    double t19 = t18 - d8 / d9;
    double t20 = t19 + d10 * t16;
    
    /* Vector operations */
    v4si tv1 = v1 + v2 * v3;
    v4si tv2 = helper3(tv1, v2);
    v4sf tvf1 = vf1 * vf2 + vf1;
    v4sf tvf2 = tvf1 - vf2 * 2.0f;
    v2df tvd1 = vd1 * vd2 + vd1;
    v2df tvd2 = tvd1 - vd2 / 2.0;
    
    /* Inline assembly to clobber registers */
    /* x86_64 version */
    asm volatile (
        "# Clobber many registers\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rbx\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        "pxor %%xmm3, %%xmm3\n\t"
        "pxor %%xmm4, %%xmm4\n\t"
        "pxor %%xmm5, %%xmm5\n\t"
        "pxor %%xmm6, %%xmm6\n\t"
        "pxor %%xmm7, %%xmm7\n\t"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory"
    );
    
    /* ARM version (commented out, use for ARM targets) */
    /*
    asm volatile (
        "# Clobber many ARM registers\n\t"
        "mov r0, #0\n\t"
        "mov r1, #0\n\t"
        "mov r2, #0\n\t"
        "mov r3, #0\n\t"
        "mov r4, #0\n\t"
        "mov r5, #0\n\t"
        "mov r6, #0\n\t"
        "mov r7, #0\n\t"
        "mov r8, #0\n\t"
        "mov r9, #0\n\t"
        "mov r10, #0\n\t"
        "vmov.f32 s0, #0.0\n\t"
        "vmov.f32 s1, #0.0\n\t"
        "vmov.f32 s2, #0.0\n\t"
        "vmov.f32 s3, #0.0\n\t"
        "vmov.f32 s4, #0.0\n\t"
        "vmov.f32 s5, #0.0\n\t"
        "vmov.f32 s6, #0.0\n\t"
        "vmov.f32 s7, #0.0\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "memory"
    );
    */
    
    /* More computation after clobber - forces reload/rematerialization */
    int t21 = t1 + t2 - t3 * t4;
    int t22 = t21 ^ t5 | t4;
    long t23 = t6 * t7 + t8;
    long t24 = t23 ^ t9 | t10;
    float t25 = t11 * t12 + t13;
    float t26 = t25 / t14 - t15;
    double t27 = t16 * t17 + t18;
    double t28 = t27 / t19 - t20;
    
    /* Vector operations after clobber */
    v4si tv3 = tv1 + tv2 * v3;
    v4sf tvf3 = tvf1 * tvf2 + vf1;
    v2df tvd3 = tvd1 * tvd2 + vd1;
    
    /* Control flow to create multiple basic blocks */
    volatile int result = 0;
    if (t1 > t2) {
        result += t21 + t22;
        if (t3 < t4) {
            result += t5 * 2;
        } else {
            result += t5 / 2;
        }
    } else {
        result += t23 + t24;
        switch (t1 % 4) {
            case 0: result += t25; break;
            case 1: result += t26; break;
            case 2: result += t27; break;
            case 3: result += t28; break;
        }
    }
    
    /* Use vector results */
    result += tv3[0] + tv3[1];
    result += (int)tvf3[0] + (int)tvf3[1];
    result += (int)tvd3[0] + (int)tvd3[1];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = input_seed + i;
        volatile long in2 = input_seed * 1000L + i * 7L;
        volatile float in3 = (float)input_seed / 3.0f + i * 0.1f;
        volatile double in4 = (double)input_seed / 7.0 + i * 0.01;
        
        /* Call the test function in a loop */
        volatile int result = test_remat(in1, in2, in3, in4);
        total += result;
        
        /* Complex expression recomputed in slightly different form */
        /* This increases chances for early rematerialization */
        volatile int temp = (in1 * 3 + in2 % 7) ^ (int)in3;
        temp = temp * 2 - (int)in4;
        total += temp;
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total % 1000);
}
