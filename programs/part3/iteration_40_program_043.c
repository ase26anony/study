/* test-early-remat.c */
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
    else return b * c + a;
}

__attribute__((noinline, noclone))
static float helper2(float a, float b, float c) {
    switch ((int)a % 4) {
        case 0: return a + b * c;
        case 1: return a - b / c;
        case 2: return a * b + c;
        default: return a / b - c;
    }
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    v4si mask = {1, 0, 1, 0};
    return (a & mask) | (b & ~mask);
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    /* Integer variables */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Floating point variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Vector variables */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Long chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * 3;
    t3 = t2 - a;
    t4 = t3 / 2;
    t5 = t4 | 0xFF;
    t6 = t5 & 0x0F;
    t7 = t6 ^ t1;
    t8 = t7 << 2;
    t9 = t8 >> 1;
    t10 = t9 + t2;
    
    f1 = c * 1.5f;
    f2 = f1 + c;
    f3 = f2 - 0.5f;
    f4 = f3 * 2.0f;
    f5 = f4 / 1.3f;
    
    d1 = d + 1.234;
    d2 = d1 * 2.0;
    d3 = d2 - 0.987;
    d4 = d3 / 1.111;
    d5 = d4 + d;
    
    /* Vector operations */
    v1 = (v4si){t1, t2, t3, t4};
    v2 = (v4si){t5, t6, t7, t8};
    v3 = v1 + v2;
    v4 = v1 * v2;
    v5 = v3 & v4;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = vf1 * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
    vf3 = vf1 + vf2;
    
    vd1 = (v2df){d1, d2};
    vd2 = vd1 * (v2df){0.9, 1.1};
    
    /* Inline assembly to clobber registers (x86_64 version) */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
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
        "pxor %%xmm8, %%xmm8\n\t"
        "pxor %%xmm9, %%xmm9\n\t"
        "pxor %%xmm10, %%xmm10\n\t"
        "pxor %%xmm11, %%xmm11\n\t"
        "pxor %%xmm12, %%xmm12\n\t"
        "pxor %%xmm13, %%xmm13\n\t"
        "pxor %%xmm14, %%xmm14\n\t"
        "pxor %%xmm15, %%xmm15"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* More operations after assembly clobber */
    t11 = helper1(t10, t9, t8);
    t12 = t11 + t7;
    t13 = t12 * 2;
    t14 = t13 - t6;
    t15 = t14 | 0xAA;
    
    f6 = helper2(f5, f4, f3);
    f7 = f6 * 0.75f;
    f8 = f7 + f2;
    f9 = f8 - f1;
    f10 = f9 / 1.1f;
    
    d6 = d5 * 1.2345;
    d7 = d6 + 0.5432;
    d8 = d7 - d4;
    d9 = d8 / 2.3456;
    d10 = d9 + d3;
    
    /* Control flow to create multiple basic blocks */
    if (t15 > 1000) {
        t16 = t15 * 3;
        t17 = helper1(t16, t14, t13);
        v5 = helper3(v5, v4);
    } else {
        t16 = t15 / 2;
        t17 = helper1(t16, t12, t11);
        v5 = v5 | v4;
    }
    
    /* Another complex expression that might get recomputed */
    int complex_expr = (t17 * 2 + t16 / 3 - t15 % 7) ^ (t14 & 0xF0);
    
    /* Use complex_expr multiple times */
    t18 = complex_expr + t13;
    t19 = complex_expr * 2 - t12;
    
    /* Recompute similar expression later */
    int recomputed_expr = (t17 * 2 + t16 / 3 - t15 % 7) ^ (t14 & 0xF0) + 1;
    
    t20 = recomputed_expr + t19;
    
    /* Final mixing of all values */
    long result = (long)t20 + (long)t17 + (long)t10;
    result += (long)(f10 * 100.0f) + (long)(d10 * 100.0);
    
    /* Mix in vector results */
    int vsum = v5[0] + v5[1] + v5[2] + v5[3];
    result += vsum;
    
    float vfsum = vf3[0] + vf3[1] + vf3[2] + vf3[3];
    result += (long)(vfsum * 10.0f);
    
    double vdsum = vd2[0] + vd2[1];
    result += (long)(vdsum * 10.0);
    
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
        volatile long in2 = input_seed * 3L + i * 2L;
        volatile float in3 = (float)input_seed / 3.0f + (float)i * 0.1f;
        volatile double in4 = (double)input_seed / 7.0 + (double)i * 0.01;
        
        total += test_remat(in1, in2, in3, in4);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %ld\n", (long)total);
    return 0;
}
